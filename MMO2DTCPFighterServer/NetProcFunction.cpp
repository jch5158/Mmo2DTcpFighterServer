#include "stdafx.h"
#include "Log.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Character.h"

SOCKET gListenSocket;

std::unordered_map<SOCKET, stSession*> gSessionMap;


// ���ο� ������ ����,��� �Ѵ�.
stSession* CreateSession(SOCKET socket)
{
	static DWORD sessionID;

	stSession* pSession = FindSession(socket);
	if (pSession == nullptr)
	{
		return nullptr;
	}

	pSession = (stSession*)malloc(sizeof(stSession));

	pSession->socket = socket;
	pSession->sessionID = sessionID;
	pSession->lastRecvTime = 0;

	++sessionID;

	return pSession;
}


// Socket ���� ���� ã��.
stSession* FindSession(SOCKET socket)
{	
	auto sessionIterE = gSessionMap.end();

	auto sessionIter = gSessionMap.find(socket);

	if (sessionIter != sessionIterE)
	{
		return sessionIter->second;
	}
	else
	{
		return nullptr;
	}
}


// �ش� ���� ����ó��.
void DisconnectSession(SOCKET socket)
{
	stSession* pSession = FindSession(socket);
	if (pSession != nullptr)
	{
		gSessionMap.erase(socket);	
		closesocket(socket);
		free(pSession);
	}

	return;
}



bool SetupNetwork(void)
{
	int retval;

	WSADATA wsa;

	retval = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (retval == SOCKET_ERROR)
	{
		wprintf_s(L"WSAStartup error %d\n", WSAGetLastError());

		_LOG(eLogList::LOG_LEVEL_ERROR, L"WSAStartup Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	gListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (gListenSocket == INVALID_SOCKET)
	{
		wprintf_s(L"socket error %d\n", WSAGetLastError());

		_LOG(eLogList::LOG_LEVEL_ERROR, L"Socket Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	// �ͺ�ŷ ��ȯ
	u_long on = 1;
	retval = ioctlsocket(gListenSocket, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"ioctlsocket Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	// RST �ɼ�
	linger optval;
	optval.l_onoff = 1;
	optval.l_linger = 0;
	retval = setsockopt(gListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"setsockopt Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	SOCKADDR_IN serverAddr;
	memset((char*)&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	InetPtonW(AF_INET, SERVER_IP, (char*)&serverAddr.sin_addr);

	retval = bind(gListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"bind Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	retval = listen(gListenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		wprintf_s(L"listen error : %d\n", WSAGetLastError());
		
		_LOG(eLogList::LOG_LEVEL_ERROR, L"listen Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	return;
}

void NetworkProcessing(void)
{
	stSession* pSession;

	SOCKET socketTable[FD_SETSIZE] = { INVALID_SOCKET };

	int socketCount = 0;

	FD_SET writeSet;
	FD_SET readSet;

	FD_ZERO(&writeSet);
	FD_ZERO(&readSet);

	FD_SET(gListenSocket, &readSet);
	socketTable[socketCount] = gListenSocket;

	++socketCount;

	auto sessionIterE = gSessionMap.end();

	for (auto sessionIter = gSessionMap.begin(); sessionIter != sessionIterE;)
	{
		pSession = sessionIter->second;

		socketTable[socketCount] = pSession->socket;

		FD_SET(pSession->socket, &readSet);

		if (!pSession->sendQ.IsEmpty())
		{
			FD_SET(pSession->socket, &writeSet);
		}

		++socketCount;
		
		// �ϴ� ��Ʈ��ũ �������� ���� sessionIter ���� ������ �� �ֱ� �����̴�. 
		++sessionIter;


		if (socketCount >= FD_SETSIZE)
		{
			SelectSocket(socketTable, &writeSet, &readSet);

			FD_ZERO(&writeSet);
			FD_ZERO(&readSet);

			memset(socketTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			FD_SET(gListenSocket, &readSet);
			socketTable[0] = gListenSocket;
			socketCount = 1;
		}
	}

	if (socketCount > 0)
	{
		SelectSocket(socketTable, &writeSet, &readSet);
	}

}

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet)
{
	int retval;

	bool disconnectFlag = false;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;	

	retval = select(0, pReadSet, pWriteSet, nullptr, &timeout);
	if (retval > 0)
	{
		for (int count = 0; retval > 0 && count < FD_SETSIZE; ++count)
		{

			if (FD_ISSET(pSocketTable[count], pWriteSet))
			{	
				--retval;
				disconnectFlag = SendEvent(pSocketTable[count]);
			}

			if (FD_ISSET(pSocketTable[count], pReadSet))
			{
				--retval;
				if (!disconnectFlag)
				{
					if (pSocketTable[count] == gListenSocket)
					{
						Accept();
					}
					else
					{						
						RecvEvent(pSocketTable[count]);
					}	
				}
			}
		}

	}
	else if (retval == SOCKET_ERROR)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"Select Socket Error, Error Code : %d\n", WSAGetLastError());
	}
}


void Accept()
{
	SOCKET clientSocket;

	SOCKADDR_IN clientAddr;
	int addrLength = sizeof(clientAddr);

	clientSocket = accept(gListenSocket, (SOCKADDR*)&clientAddr, &addrLength);
	if (clientSocket == INVALID_SOCKET)
	{
		_LOG(eLogList::LOG_LEVEL_WARNING, L"Accept Error, Error Code : %d\n", WSAGetLastError());
		return;
	}

	stSession* pSession = CreateSession(clientSocket);
	if (pSession == nullptr)
	{
		// �ߺ� ������ ��� ���� ���� ������ �� ���� Ȯ���� ����.
		_LOG(eLogList::LOG_LEVEL_ERROR, L"Accept �ߺ� ����, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	gSessionMap.insert(std::pair<SOCKET, stSession*>(clientSocket, pSession));

	// TODO : CreateCharacter �Լ� �����

	return;
}



void RecvEvent(SOCKET socket)
{
	int retval;

	stSession* pSession = FindSession(socket);
	if (pSession == nullptr)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"Recv Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
		
		int* ptr = nullptr;
		*ptr = -1;
	}

	pSession->lastRecvTime = timeGetTime();

	int directBufferSize = pSession->recvQ.DirectEnqueueSize();

	retval = recv(pSession->socket, pSession->recvQ.GetRearBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR || retval == 0)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"recv Eerror, Error Code : %d\n", WSAGetLastError());	
		
		DisconnectSession(socket);
		//TODO : ĳ���� ����?	

		return;
	}

	if (retval > 0)
	{
		pSession->recvQ.MoveRear(retval);

		while (1)
		{
			retval = CheckComplateMessage(pSession);

			if (retval == 1)
			{
				break;
			}
			else if (retval == -1)
			{
				_LOG(eLogList::LOG_LEVEL_ERROR, L"DoCheckComplateMessage Eerror, Session ID : %d\n", pSession->sessionID);

				int* ptr = nullptr;
				*ptr = -1;
				return;
			}
		}
	}
}


bool SendEvent(SOCKET socket)
{
	int retval;

	stSession* pSession = FindSession(socket);
	if (pSession == nullptr)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"Send Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);

		int* ptr = nullptr;
		*ptr = -1;

		return false;
	}

	int directBufferSize = pSession->sendQ.DirectDequeueSize();

	retval = send(socket, pSession->sendQ.GetFrontBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR)
	{
		_LOG(eLogList::LOG_LEVEL_WARNING, L"recv Eerror, Error Code : %d\n", WSAGetLastError());

		DisconnectSession(socket);

		return false;
	}

	pSession->sendQ.MoveFront(retval);

	return true;
}


// 0�� ó���Ϸ�, 1�� ó�� �� �� ����, -1 ����
int CheckComplateMessage(stSession* pSession)
{
	int retval;

	int recvSize = pSession->recvQ.GetUseSize();

	if (sizeof(stHeader) > recvSize)
	{
		return 1;
	}

	stHeader messageHeader;
	retval = pSession->recvQ.Peek((char*)&messageHeader, sizeof(stHeader));
	if(retval != sizeof(stHeader))
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"Peek Eerror, retval : %d, GetUseSize : %d \n", retval, pSession->recvQ.GetUseSize());
		return -1;
	}

	// ��Ŷ �ڵ尡 ��ġ���� �������� ���۵� �����̴�.
	if (messageHeader.byCode != dfNETWORK_PACKET_CODE)
	{
		_LOG(eLogList::LOG_LEVEL_WARNING, L"Packet Code : %d, Session ID : %d \n", messageHeader.byCode, pSession->sessionID);

		DisconnectSession(pSession->socket);

		return 1;
	}

	if (sizeof(stHeader) + messageHeader.bySize > recvSize)
	{
		return 1;
	}

	pSession->recvQ.MoveFront(sizeof(stHeader));

	CMessage message;

	if (!pSession->recvQ.Dequeue((char*)&message, messageHeader.bySize))
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"recvQ Size :%d \n", pSession->recvQ.GetUseSize());

		return -1;
	}

	message.MoveWritePos(messageHeader.bySize);

	try
	{
		if (!RecvMessageProcessing(pSession, messageHeader.byType, &message))
		{
			// �� ������ ������ �־��� ���
			return -1;
		}
	}
	catch (CExceptionObject& exception)
	{
		// Ŭ���̾�Ʈ�� �Ǽ��� Ŭ���̾�Ʈ�� ���������� ���۵� �޽����� ������ ���
		_LOG(eLogList::LOG_LEVEL_ERROR, L"MessageProcessing Eerror, Session ID : %d\n", pSession->sessionID);

		FILE* fp;

		// ���� ���� ��Ǭ
		fopen_s(&fp, "ErrorDump.txt", "a+t");

		for (int iCnt = 0; iCnt < exception.m_BufferSize; ++iCnt)
		{
			// �޽��� �α�
			fprintf_s(fp, "%02x ", exception.m_MessageLog[iCnt]);
		}

		// ���� �Լ��� ���� ������ Ÿ��
		fwrite(exception.m_ErrorDataLog, 1, strlen(exception.m_ErrorDataLog), fp);

		DisconnectSession(pSession->socket);
		
		fclose(fp);

		return 1;
	}

	return 0;
}


// TODO : MessageProcessing
bool RecvMessageProcessing(stSession* pSession, BYTE messageType, CMessage* pMessage)
{
	switch(messageType)
	{
	case dfPACKET_CS_MOVE_START:

		return MoveStartMessageProcessing(pSession, pMessage);

		break;
	case dfPACKET_CS_MOVE_STOP:

		return MoveStopMessageProcessing(pSession, pMessage);

		break;
	case dfPACKET_CS_ATTACK1:

		return Attack1MessageProcessing(pSession, pMessage);

		break;
	case dfPACKET_CS_ATTACK2:
		
		return Attack2MessageProcessing(pSession, pMessage);

		break;
	case dfPACKET_CS_ATTACK3:

		return Attack3MessageProcessing(pSession, pMessage);

		break;
	default:

		_LOG(eLogList::LOG_LEVEL_ERROR, L"unknown type, type : %d, Session ID : %d\n", messageType, pSession->sessionID);

		DisconnectSession(pSession->socket);

		break;
	}

	// default ���� ���� ����ڰ� ���۵� ��Ŷ�� ���´ٴ� �ǹ��̴�.
	return true;
}

// TODO : �޽��� ���μ��� �Լ� �����ϱ�
bool MoveStartMessageProcessing(stSession* pSession, CMessage* pMessage)
{
	BYTE direction;
	short posX;
	short posY;
	
	*pMessage >> direction;
	*pMessage >> posX;
	*pMessage >> posY;


	_LOG(eLogList::LOG_LEVEL_DEBUG, L"# MOVESTART # SessionID:%d / Direction:%d / X:%d / Y:%d",pSession->sessionID, direction, posX, posY);



}

bool MoveStopMessageProcessing(stSession* pSession, CMessage* pMessage)
{

}

bool Attack1MessageProcessing(stSession* pSession, CMessage* pMessage)
{

}

bool Attack2MessageProcessing(stSession* pSession, CMessage* pMessage)
{

}

bool Attack3MessageProcessing(stSession* pSession, CMessage* pMessage)
{

}
