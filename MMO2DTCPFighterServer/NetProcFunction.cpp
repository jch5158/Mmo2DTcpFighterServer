#include "stdafx.h"
#include "Log.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Sector.h"
#include "Character.h"

SOCKET gListenSocket;

std::unordered_map<SOCKET, stSession*> gSessionMap;

// ���ο� ������ ����,��� �Ѵ�.
stSession* CreateSession(SOCKET socket)
{
	static DWORD sessionID = 1;

	stSession* pSession = FindSession(socket);
	if (pSession != nullptr)
	{
		return nullptr;
	}

	pSession = (stSession*)malloc(sizeof(stSession));

	// ������ ������ ȣ���� ���� placement new
	new(pSession) stSession();

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

	// Accept���� stSession, stChracter�� �� ����ϴµ�, Disconnect���� ã�� �� ���ٸ��� �� ������ �����̴�.
	if (pSession == nullptr)
	{
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"DisconnectSession Error LINE : %d, FILE : %s", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	gSessionMap.erase(socket);
	closesocket(socket);
	free(pSession);

	return;
}


void DeleteClient(SOCKET socket)
{		
	stSession* pSession = FindSession(socket);

	// Accept���� stSession, stChracter�� �� ����ϴµ�, Disconnect���� ã�� �� ���ٸ��� �� ������ �����̴�.
	if (pSession == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"DeleteClient Error");
		int* ptr = nullptr;
		*ptr = -1;
	}


	DeleteCharacter(pSession);

	DisconnectSession(pSession->socket);
}



void SetupNetwork(void)
{
	int retval;

	WSADATA wsa;

	retval = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (retval == SOCKET_ERROR)
	{
		wprintf_s(L"WSAStartup error %d\n", WSAGetLastError());

		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"WSAStartup Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	gListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (gListenSocket == INVALID_SOCKET)
	{
		wprintf_s(L"socket error %d\n", WSAGetLastError());

		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Socket Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	// �ͺ�ŷ ��ȯ
	u_long on = 1;
	retval = ioctlsocket(gListenSocket, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"ioctlsocket Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	// KeepAlive �ɼ�
	BOOL enable = true;
	retval = setsockopt(gListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
	if (retval == SOCKET_ERROR)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"setsockopt Error, Error Code : %d\n", WSAGetLastError());

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
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"setsockopt Error, Error Code : %d\n", WSAGetLastError());

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
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"bind Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	retval = listen(gListenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{	
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"listen Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;
	}

	return;
}

void NetworkProcessing(void)
{
	// 64���� ���� select
	int socketCount = 0;

	stSession* pSession;

	SOCKET socketTable[FD_SETSIZE] = { INVALID_SOCKET };

	FD_SET writeSet;
	FD_SET readSet;

	FD_ZERO(&writeSet);
	FD_ZERO(&readSet);

	// gListenSocket���� Accept�� �ޱ� ���� �̸� readSet�� �����صд�.
	FD_SET(gListenSocket, &readSet);
	socketTable[socketCount] = gListenSocket;
	++socketCount;


	auto sessionIterE = gSessionMap.end();

	for (auto sessionIter = gSessionMap.begin(); sessionIter != sessionIterE;)
	{
		pSession = sessionIter->second;
	
		// �ϴ� ��Ʈ��ũ �������� ���� sessionIter ���� ������ �� �ֱ� �����̴�. �̸� ���� iterator�� �ű��.
		++sessionIter;

		socketTable[socketCount] = pSession->socket;

		// sendQ�� ������ ���� ��� ���� �� ����
		if (!pSession->sendQ.IsEmpty())
		{
			FD_SET(pSession->socket, &writeSet);
		}

		// �б� �� ����
		FD_SET(pSession->socket, &readSet);
	
		
		++socketCount;	
		
		// select�� ������ 64���� ��� ó���ϱ� �빮�� socketCount�� 64���� ��� SelectSocket�� ����
		if (socketCount >= FD_SETSIZE)
		{
			SelectSocket(socketTable, &writeSet, &readSet);

			// ���� ���ϵ��� ó���ϱ� ���� writeSet, readSet, socketTable�� �ʱ�ȭ�Ѵ�.
			FD_ZERO(&writeSet);
			FD_ZERO(&readSet);
			memset(socketTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			// �� Select���� Accept�� ������ ó���ϱ� ���� gListenSocket�� �����Ѵ�.
			FD_SET(gListenSocket, &readSet);
			socketTable[0] = gListenSocket;
			socketCount = 1;
		}
	}

	// socketTable�� 64�� �̸��� ������ ���� �������� �� �ֱ� ������ SelectSocket�� �ٽ� ȣ���Ѵ�.
	if (socketCount > 0)
	{
		SelectSocket(socketTable, &writeSet, &readSet);
	}

}

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet)
{
	int retval;

	bool disconnectFlag;

	// select�� block�� ���ֱ� ���� Ÿ�Ӿƿ��� 0���� ����
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;	

	retval = select(0, pReadSet, pWriteSet, nullptr, &timeout);
	if (retval > 0)
	{

		// retval�� ������ ������ ���� SendEvent, Accept, RecvEvent ȣ�� �� ���� retval �� 1�� ���ҽ�Ų��.
		for (int count = 0; retval > 0 && count < FD_SETSIZE; ++count)
		{
			disconnectFlag = false;

			if (FD_ISSET(pSocketTable[count], pWriteSet))
			{	
				--retval;

				// SendEvent�� return ���� true�� ��� �̹� ������ �������ٰ� �Ǵ��Ѵ�.
				// �׷��� �Ʒ��� RecvEvent�� ������ �°Ͱ� ������� �������� �ʴ´�.
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
		// Select���� Error�� �߻����� ���
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Select Socket Error, Error Code : %d\n", WSAGetLastError());
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
		_LOG(TRUE, eLogList::LOG_LEVEL_WARNING, L"Accept Error, Error Code : %d\n", WSAGetLastError());
		return;
	}

	stSession* pSession = CreateSession(clientSocket);
	if (pSession == nullptr)
	{
		// �ߺ� ������ ��� ���� ���� ������ �� ���� Ȯ���� ����.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept �ߺ� ����, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	gSessionMap.insert(std::pair<SOCKET, stSession*>(clientSocket, pSession));

	//auto testIter = gSessionMap.begin();

	/*short posX = (rand() % 6000) + 200;
	short posY = (rand() % 6000) + 200;*/

	short posX = 500;
	short posY = 500;

	stCharacter* pCharacter = CreateCharacter(pSession, eKeyList::eACTION_STAND, eKeyList::eACTION_MOVE_RR, eKeyList::eACTION_STAND, posX, posY);
	if (pCharacter == nullptr)
	{
		// �ߺ� ������ ��� ���� ���� ������ �� ���� Ȯ���� ����.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept �ߺ� ĳ���� ������û sessionID : %d, LINE : %d, FILE : %s\n", pSession->sessionID,__LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}


	gCharacterMap.insert(std::pair<DWORD, stCharacter*>(pSession->sessionID, pCharacter));

	CMessage message;

	// pCharacter �����޽��� �����
	PackingCreateCharacter(&message, pCharacter->sessionID, pCharacter->direction, posX, posY, pCharacter->hp);

	// pSession�� pCharacter �����޽��� ����ĳ����
	SendProcUnicasting(pSession, &message);
	
	// pCharacter ������ �Ѹ���
	PackingCreateOtherCharacter(&message, pCharacter->sessionID, pCharacter->direction, posX, posY, pCharacter->hp);

	// pSession ����� client�鿡�� pCharacter �����϶�� �Ѹ���
	SendProcAroundSector(pSession, &message);
	

	stSectorAround sectorAround;
	
	// pCharacter ����� ���� stSectorAround�� �����ϱ�
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &sectorAround);


	for (int count = 0; count < sectorAround.count; ++count)
	{
		auto sectorList = gSector[sectorAround.around[count].posY][sectorAround.around[count].posX];

		auto iterE = sectorList.end();

		for (auto iter = sectorList.begin(); iter != iterE; ++iter)
		{
			if (pCharacter != (*iter))
			{
				// ����� client�� �����϶�� pCharcter�ְ� ����ĳ����
				PackingCreateOtherCharacter(&message, (*iter)->sessionID, (*iter)->direction, (*iter)->posX, (*iter)->posY, (*iter)->hp);

				SendProcUnicasting(pSession, &message);
			}
		}
	}	

	_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept �Ϸ� sessionID : %d\n", pSession->sessionID);


	return;
}



void RecvEvent(SOCKET socket)
{
	int retval;

	// session �� ã�� ���ߴٸ� �� ������ �Ǽ��ѰŴ�.
	stSession* pSession = FindSession(socket);
	if (pSession == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Recv Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
		
		int* ptr = nullptr;
		*ptr = -1;
	}

	// recv Ÿ���� �����Ѵ�. recv�� ��� ���� �ʴ´ٸ� client�� ���´�.
	pSession->lastRecvTime = timeGetTime();

	int directBufferSize = pSession->recvQ.DirectEnqueueSize();

	retval = recv(pSession->socket, pSession->recvQ.GetRearBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR || retval == 0)
	{
		// Ŭ���̾�Ʈ�� �����ų� recv �����̴�.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"recv Eerror, Error Code : %d\n", WSAGetLastError());	
			
		// session�� client�� �����Ѵ�.
		DeleteClient(socket);

		return;
	}

	// recv ���� 0 �̻��� ��� �޽����� ���� �� �ֱ� ������ �Ʒ� ������ �����Ѵ�.
	if (retval > 0)
	{
		// GetRearBufferPtr�� �����͸� �����߱� ������ rear�� �Ű��ش�.
		pSession->recvQ.MoveRear(retval);

		while (1)
		{
			// �޽����� �ϼ��Ǿ����� Ȯ���ϰ� �ϼ��Ǿ��ٸ� �޽����� ó���Ѵ�.
			retval = CheckComplateMessage(pSession);

			// �� �̻� ó���� �޽����� ���� ��� break�� �ݺ����� ���� ������.
			if (retval == 1)
			{
				break;
			}
			else if (retval == -1)
			{
				// �� �߸��� ���� �׸��� client�� ������ ���� �ֱ� ������ ���Ⱑ �ָ��ϴ�.
				_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"DoCheckComplateMessage Eerror, Session ID : %d\n", pSession->sessionID);

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
		// ���� �޽����� �ִµ� FindSession���� ã�� �� ���ٸ� ������ �߸��ȴ�.
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"Send Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);

		int* ptr = nullptr;
		*ptr = -1;

		return false;
	}

	int directBufferSize = pSession->sendQ.DirectDequeueSize();

	retval = send(socket, pSession->sendQ.GetFrontBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR)
	{
		// retval �� SOCKET_ERROR�� ��� �۽Ź��۰� �� á�ų� Ŭ���̾�Ʈ�� ����ٰ� �Ǵ��Ͽ� �D�´�.
		_LOG(TRUE,eLogList::LOG_LEVEL_WARNING, L"recv Eerror, Error Code : %d\n", WSAGetLastError());

		DeleteClient(socket);

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

	// ���� �� ���ۿ� �ִ� �����Ͱ� ��� ��Ŷ ��� �����ŭ �ִ��� Ȯ���Ѵ�.
	if (sizeof(stHeader) > recvSize)
	{
		return 1;
	}

	// ��� �����ŭ ���� ��� ��� �����ŭ Peek�Ͽ� ��� ������ Ȯ���Ѵ�.
	stHeader messageHeader;
	retval = pSession->recvQ.Peek((char*)&messageHeader, sizeof(stHeader));
	if(retval != sizeof(stHeader))
	{
		// ���� �������� ��� ����� Ȯ���ϰ� Peek�Ͽ��µ� retval ũ�Ⱑ ���������� �ٸ� ��� �� �����ۿ� ������.
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"stHeader Peek Eerror, retval : %d, GetUseSize : %d \n", retval, pSession->recvQ.GetUseSize());
	
		int* ptr = nullptr;
		*ptr = -1;
	}

	// ��Ŷ �ڵ尡 ��ġ���� �������� ���۵� �����̴�.
	if (messageHeader.byCode != dfNETWORK_PACKET_CODE)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Packet Code : %d, Session ID : %d \n", messageHeader.byCode, pSession->sessionID);
		
		// Accept() �������� ���ؼ� ������ Ŭ���̾�Ʈ ĳ���Ͱ� �����ϱ� ������ DeleteClient() �Լ��� ���ǰ� ĳ���� ������ �����Ѵ�.
		DeleteClient(pSession->socket);

		return -1;
	}

	// ������ ����� �޽��� ũ�⺸�� ���� ��� ���� recv�� ó���ϱ� ���� return �Ѵ�.
	if (sizeof(stHeader) + messageHeader.bySize > recvSize)
	{
		return 1;
	}

	pSession->recvQ.MoveFront(sizeof(stHeader));

	CMessage message;

	if (!pSession->recvQ.Dequeue((char*)message.GetBufferPtr(), messageHeader.bySize))
	{
		// �� �������� ������ ���� ����̴�.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"recvQ Size :%d \n", pSession->recvQ.GetUseSize());

		int* ptr = nullptr;
		*ptr = -1;
	}

	// �޽����� buffer �����ͷ� ���� �־��� ������ ���� ũ�⸸ŭ message�� rear�� �Ű��ش�.
	message.MoveWritePos(messageHeader.bySize);

	try
	{
		if (!RecvMessageProcessing(pSession, messageHeader.byType, &message))
		{
			// �� ������ ������ �־��� ��쵵 �ְ�, ���۵� ���� �ִ� ���	
			_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"#RecvMessageProcessing# Session ID : %d \n", pSession->sessionID);

			return -1;
		}
	}
	catch (CExceptionObject exception)
	{
		// Ŭ���̾�Ʈ�� �Ǽ��� Ŭ���̾�Ʈ�� ���������� ���۵� �޽����� ������ ���
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"MessageProcessing Eerror, Session ID : %d\n", pSession->sessionID);

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

		return -1;
	}

	return 0;
}


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

		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"unknown type, type : %d, Session ID : %d\n", messageType, pSession->sessionID);

		DeleteClient(pSession->socket);

		break;
	}

	// default ���� ���� ����ڰ� ���۵� ��Ŷ�� ���´ٴ� �ǹ��̴�.
	return false;
}

bool MoveStartMessageProcessing(stSession* pSession, CMessage* pMessage)
{
	BYTE moveDirection;
	short posX;
	short posY;
	
	*pMessage >> moveDirection;
	*pMessage >> posX;
	*pMessage >> posY;

	_LOG(FALSE,eLogList::LOG_LEVEL_DEBUG, L"# MOVESTART # SessionID:%d / Direction:%d / X:%d / Y:%d \n",pSession->sessionID, moveDirection, posX, posY);

	stCharacter* pCharacter = FindCharacter(pSession->sessionID);
	if (pCharacter == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"# MOVESTART # Character Not Found : %d\n", pSession->sessionID);

		DisconnectSession(pSession->socket);
		
		int* ptr = nullptr;

		*ptr = -1;
	}


	if (abs(pCharacter->posX - posX) > dfERROR_RANGE || abs(pCharacter->posY - posY) > dfERROR_RANGE)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"#Sync Packet# SessionID : %d ServerX : %d, ServerY : %d, ClientX : %d, ClientY : %d",
			pCharacter->sessionID, pCharacter->posX, pCharacter->posY, posX, posY
		);

		PackingSynPosition(pMessage, pCharacter->sessionID, pCharacter->posX, pCharacter->posY);

		SendProcAroundSector(pSession, pMessage, true);
		
		posX = pCharacter->posX;
		posY = pCharacter->posY;
	}

	pCharacter->action = moveDirection;
	pCharacter->moveDirection = moveDirection;

	switch (moveDirection)
	{
	case eKeyList::eACTION_MOVE_LL:

	case eKeyList::eACTION_MOVE_LU:

	case eKeyList::eACTION_MOVE_LD:

		pCharacter->direction = eKeyList::eACTION_MOVE_LL;

		break;
	case eKeyList::eACTION_MOVE_RR:

	case eKeyList::eACTION_MOVE_RU:

	case eKeyList::eACTION_MOVE_RD:

		pCharacter->direction = eKeyList::eACTION_MOVE_RR;

		break;
	}

	// client�� ��ǥ�� �ϰ� ��������
	pCharacter->posX = posX;
	pCharacter->posY = posY;

	if (UpdateSectorPosition(pCharacter))
	{
		// ���Ͱ� �ٲ���ٸ��� SendUpdateCharacterSector�� ��ε�ĳ����
		SendUpdateCharacterSector(pCharacter);
		
		return true;
	}

	PackingMoveStart(pMessage, pCharacter->sessionID, pCharacter->moveDirection, pCharacter->posX, pCharacter->posY);

	// ���Ͱ� �ٲ��� �ʾҴٸ��� ���� ��Ŷ ���� �� AroundSector�� ��ε�ĳ����
	SendProcAroundSector(pSession, pMessage);

	return true;
}

bool MoveStopMessageProcessing(stSession* pSession, CMessage* pMessage)
{
	unsigned char moveDirection;
	short posX;
	short posY;

	*pMessage >> moveDirection;
	*pMessage >> posX;
	*pMessage >> posY;

	_LOG(FALSE, eLogList::LOG_LEVEL_DEBUG, L"# MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d \n", pSession->sessionID, moveDirection, posX, posY);

	stCharacter* pCharacter = FindCharacter(pSession->sessionID);
	if (pCharacter == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"# MOVESTOP # Character Not Found : %d\n", pSession->sessionID);

		DisconnectSession(pSession->socket);
		
		int* ptr = nullptr;
		*ptr = -1;

		return false;
	}

	if (abs(pCharacter->posX - posX) > dfERROR_RANGE || abs(pCharacter->posY - posY) > dfERROR_RANGE)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"#Sync Packet# SessionID : %d ServerX : %d, ServerY : %d, ClientX : %d, ClientY : %d",
			pCharacter->sessionID, pCharacter->posX, pCharacter->posY, posX, posY
		);

		PackingSynPosition(pMessage, pCharacter->sessionID, pCharacter->posX, pCharacter->posY);

		SendProcAroundSector(pSession, pMessage, true);
		
		posX = pCharacter->posX;
		posY = pCharacter->posY;
	}

	pCharacter->posX = posX;
	pCharacter->posY = posY;
	pCharacter->direction = moveDirection;
	pCharacter->action = eKeyList::eACTION_STAND;

	if (UpdateSectorPosition(pCharacter))
	{
		// ���Ͱ� �ٲ���ٸ��� SendUpdateCharacterSector�� ��ε�ĳ����
		SendUpdateCharacterSector(pCharacter);

	}

	PackingMoveStop(pMessage, pCharacter->sessionID, pCharacter->direction, pCharacter->posX, pCharacter->posY);

	SendProcAroundSector(pSession, pMessage);

	return true;
}

bool Attack1MessageProcessing(stSession* pSession, CMessage* pMessage)
{

	return true;
}

bool Attack2MessageProcessing(stSession* pSession, CMessage* pMessage)
{

	return true;
}

bool Attack3MessageProcessing(stSession* pSession, CMessage* pMessage)
{

	return true;
}


void PackingCreateCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp)
{
	stHeader messageHeader;

	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	messageHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short) + sizeof(BYTE);

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID << (unsigned char)direction << posX << posY << (unsigned char)hp;
}

void PackingDeleteCharacter(CMessage* pMessage, DWORD sessionID)
{	
	stHeader messageHeader;

	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.bySize = sizeof(DWORD);
	messageHeader.byType = dfPACKET_SC_DELETE_CHARACTER;

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID;
}

// �׼� �κ� Ȯ���� �ؾ��ҵ�
void PackingCreateOtherCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp)
{
	stHeader messageHeader;

	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short) + sizeof(BYTE);
	messageHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID << (unsigned char)direction << posX << posY << (unsigned char)hp;
}

void PackingMoveStart(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY)
{
	stHeader messageHeader;

	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	messageHeader.byType = dfPACKET_SC_MOVE_START;

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID << (unsigned char)moveDirection << posX << posY;
}


void PackingMoveStop(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY)
{
	stHeader messageHeader;

	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	messageHeader.byType = dfPACKET_SC_MOVE_STOP;

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID << (unsigned char)moveDirection << posX << posY;
}


void PackingSynPosition(CMessage* pMessage, DWORD sessionID, short posX, short posY)
{
	stHeader messageHeader;
	messageHeader.byCode = dfNETWORK_PACKET_CODE;
	messageHeader.bySize = sizeof(DWORD) + sizeof(short) + sizeof(short);
	messageHeader.byType = dfPACKET_SC_SYNC;

	pMessage->Clear();

	pMessage->PutData((char*)&messageHeader, sizeof(stHeader));
	pMessage->MoveWritePos(sizeof(stHeader));

	*pMessage << (unsigned int)sessionID << posX << posY;
}


void SendProcUnicasting(stSession* pSession, CMessage* pMessage)
{
	pSession->sendQ.Enqueue(pMessage->GetBufferPtr(), pMessage->GetDataSize());
}


void SendProcOneSector(int sectorPosX, int sectorPosY, stSession* pExceptSession, CMessage* pMessage)
{
	auto iterE = gSector[sectorPosY][sectorPosX].end();

	for (auto iter = gSector[sectorPosY][sectorPosX].begin(); iter != iterE; ++iter)
	{
		if (pExceptSession != (*iter)->pSession)
		{
			SendProcUnicasting((*iter)->pSession, pMessage);
		}
	}
}

void SendProcAroundSector(stSession* pSession, CMessage* pMessage, bool sendMeFlag)
{
	stCharacter* pCharacter;

	pCharacter = FindCharacter(pSession->sessionID);
	if (pCharacter == nullptr)
	{
		// ĳ���͸� ��ã������ ���� �� ������ ������.
		// ���۵� Ŭ���̾�Ʈ��� �ص� Accept�ʿ��� ó���ع���
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR,L"#SendProcAroundSector# Not Found Character SessionID : %d", pSession->sessionID);

		int* ptr = nullptr;

		*ptr = -1;
	}

	stSectorAround sectorAround;
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &sectorAround);

	stSession* pExistSession;

	// sendMeFlag �� true�� ��� �����׵� �޽����� ������.
	if (sendMeFlag)
	{
		pExistSession = nullptr;
	}
	else
	{
		pExistSession = pSession;
	}

	for (int count = 0; count < sectorAround.count; ++count)
	{	
		SendProcOneSector(sectorAround.around[count].posX, sectorAround.around[count].posY, pExistSession, pMessage);
	}
}