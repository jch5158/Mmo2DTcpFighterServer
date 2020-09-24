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

std::list<stSession*> gClearSessionList;


// 새로운 세션을 생성,등록 한다.
stSession* CreateSession(SOCKET socket)
{
	static DWORD sessionID = 1;

	stSession* pSession = FindSession(socket);
	if (pSession != nullptr)
	{
		return nullptr;
	}

	pSession = (stSession*)malloc(sizeof(stSession));

	// 링버퍼 생성자 호출을 위한 placement new
	new(pSession) stSession();

	pSession->socket = socket;
	pSession->sessionID = sessionID;
	pSession->lastRecvTime = 0;

	++sessionID;

	return pSession;
}


// Socket 으로 세션 찾기.
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


// 해당 세션 종료처리.
void DisconnectSession(SOCKET socket)
{
	stSession* pSession = FindSession(socket);

	// Accept에서 stSession, stChracter를 다 등록하는데, Disconnect에서 찾을 수 없다면은 내 로직의 문제이다.
	if (pSession == nullptr)
	{
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"DisconnectSession Error LINE : %d, FILE : %s", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	gSessionMap.erase(socket);
	closesocket(socket);
	delete pSession;

	return;
}


void DeleteClient(SOCKET socket)
{		
	stSession* pSession = FindSession(socket);

	// Accept에서 stSession, stChracter를 다 등록하는데, Disconnect에서 찾을 수 없다면은 내 로직의 문제이다.
	if (pSession == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"DeleteClient Error");
		int* ptr = nullptr;
		*ptr = -1;
	}

	DeleteCharacter(pSession);

	DisconnectSession(pSession->socket);
}

void CleanUpSession(void)
{
	gClearSessionList.clear();

	auto iterE = gSessionMap.end();

	for (auto iter = gSessionMap.begin(); iter != iterE;)
	{
		auto deleteIter = iter;
		++iter;

		gSessionMap.erase((*deleteIter).second->socket);
		closesocket((*deleteIter).second->socket);
			
		delete (*deleteIter).second;
	}
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

	// 넌블럭킹 전환
	u_long on = 1;
	retval = ioctlsocket(gListenSocket, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"ioctlsocket Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;

	}

	// KeepAlive 옵션
	BOOL enable = true;
	retval = setsockopt(gListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
	if (retval == SOCKET_ERROR)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"setsockopt Error, Error Code : %d\n", WSAGetLastError());

		int* ptr = nullptr;
		*ptr = -1;
	}

	// RST 옵션
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

void CleanUpNetwork(void)
{
	closesocket(gListenSocket);
	WSACleanup();
}


void ClearSessionList(void)
{

	auto iterE = gClearSessionList.end();

	for (auto iter = gClearSessionList.begin(); iter != iterE; )
	{
		DeleteClient((*iter)->socket);

		iter = gClearSessionList.erase(iter);
	}
}



void NetworkProcessing(void)
{
	// gClearSessionList에 있는 세션들을 다 정리한다.
	ClearSessionList();

	// 64개씩 소켓 select
	int socketCount = 0;

	stSession* pSession;

	SOCKET socketTable[FD_SETSIZE] = { INVALID_SOCKET };

	FD_SET writeSet;
	FD_SET readSet;

	FD_ZERO(&writeSet);
	FD_ZERO(&readSet);

	// gListenSocket으로 Accept를 받기 위해 미리 readSet에 셋팅해둔다.
	FD_SET(gListenSocket, &readSet);
	socketTable[socketCount] = gListenSocket;
	++socketCount;

	auto sessionIterE = gSessionMap.end();

	for (auto sessionIter = gSessionMap.begin(); sessionIter != sessionIterE;)
	{
		pSession = sessionIter->second;
	
		// 하단 네트워크 로직에서 현재 sessionIter 값을 삭제할 수 있기 때문이다. 미리 다음 iterator로 옮긴다.
		++sessionIter;

		socketTable[socketCount] = pSession->socket;

		// sendQ에 보낼게 있을 경우 쓰기 셋 셋팅
		if (!pSession->sendQ.IsEmpty())
		{
			FD_SET(pSession->socket, &writeSet);
		}

		// 읽기 셋 셋팅
		FD_SET(pSession->socket, &readSet);
	
		
		++socketCount;	
		
		// select는 소켓을 64개씩 끊어서 처리하기 대문에 socketCount가 64개일 경우 SelectSocket을 실행
		if (socketCount >= FD_SETSIZE)
		{
			SelectSocket(socketTable, &writeSet, &readSet);

			// 다음 소켓들을 처리하기 위해 writeSet, readSet, socketTable을 초기화한다.
			FD_ZERO(&writeSet);
			FD_ZERO(&readSet);
			memset(socketTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			// 매 Select마다 Accept가 들어오면 처리하기 위해 gListenSocket을 셋팅한다.
			FD_SET(gListenSocket, &readSet);
			socketTable[0] = gListenSocket;
			socketCount = 1;
		}
	}

	// socketTable에 64개 미만의 소켓이 아직 남아있을 수 있기 때문에 SelectSocket을 다시 호출한다.
	if (socketCount > 0)
	{
		SelectSocket(socketTable, &writeSet, &readSet);
	}

}

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet)
{
	int retval;

	bool disconnectFlag;

	// select의 block을 없애기 위해 타임아웃값 0으로 셋팅
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;	

	retval = select(0, pReadSet, pWriteSet, nullptr, &timeout);
	if (retval > 0)
	{

		// retval은 반응온 소켓의 수로 SendEvent, Accept, RecvEvent 호출 시 마다 retval 을 1씩 감소시킨다.
		for (int count = 0; retval > 0 && count < FD_SETSIZE; ++count)
		{
			disconnectFlag = false;

			if (FD_ISSET(pSocketTable[count], pWriteSet))
			{	
				--retval;

				// SendEvent의 return 값이 true일 경우 이번 소켓은 끊어졌다고 판단한다.
				// 그러면 아래의 RecvEvent는 반응이 온것과 상관없이 실행하지 않는다.
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
		// Select에서 Error가 발생됐을 경우
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Select Socket Error, Error Code : %d\n", WSAGetLastError());
	}
}


void Accept(void)
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
		// 중복 세션일 경우 내가 세션 정리를 안 했을 확률이 높다.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept 중복 세션, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
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
		// 중복 세션일 경우 내가 세션 정리를 안 했을 확률이 높다.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept 중복 캐릭터 생성요청 sessionID : %d, LINE : %d, FILE : %s\n", pSession->sessionID,__LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}


	gCharacterMap.insert(std::pair<DWORD, stCharacter*>(pSession->sessionID, pCharacter));

	CMessage message;

	// pCharacter 생성메시지 만들기
	PackingCreateCharacter(&message, pCharacter->sessionID, pCharacter->direction, posX, posY, pCharacter->hp);

	// pSession에 pCharacter 생성메시지 유니캐스팅
	SendProcUnicasting(pSession, &message);
	
	// pCharacter 만들라고 뿌리기
	PackingCreateOtherCharacter(&message, pCharacter->sessionID, pCharacter->direction, posX, posY, pCharacter->hp);

	// pSession 영향권 client들에게 pCharacter 생성하라고 뿌리기
	SendProcAroundSector(pSession, &message);
	

	stSectorAround sectorAround;
	
	// pCharacter 영향권 섹터 stSectorAround에 셋팅하기
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &sectorAround);


	for (int count = 0; count < sectorAround.count; ++count)
	{
		auto sectorList = gSector[sectorAround.around[count].posY][sectorAround.around[count].posX];

		auto iterE = sectorList.end();

		for (auto iter = sectorList.begin(); iter != iterE; ++iter)
		{
			if (pCharacter != (*iter))
			{
				// 영향권 client를 생성하라고 pCharcter애게 유니캐스팅
				PackingCreateOtherCharacter(&message, (*iter)->sessionID, (*iter)->direction, (*iter)->posX, (*iter)->posY, (*iter)->hp);

				SendProcUnicasting(pSession, &message);
			}
		}
	}	

	_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Accept 완료 sessionID : %d\n", pSession->sessionID);


	return;
}



void RecvEvent(SOCKET socket)
{
	int retval;

	// session 을 찾지 못했다면 내 로직이 실수한거다.
	stSession* pSession = FindSession(socket);
	if (pSession == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Recv Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);
		
		int* ptr = nullptr;
		*ptr = -1;
	}

	// recv 타임을 갱신한다. recv가 계속 오지 않는다면 client를 끊는다.
	pSession->lastRecvTime = timeGetTime();

	int directBufferSize = pSession->recvQ.DirectEnqueueSize();

	retval = recv(pSession->socket, pSession->recvQ.GetRearBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR || retval == 0)
	{
		// 클라이언트가 끊었거나 recv 에러이다.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"recv Eerror, Error Code : %d\n", WSAGetLastError());	
			
		// session과 client를 정리한다.
		
		//DeleteClient(socket);
		
		gClearSessionList.push_back(pSession);

		return;
	}

	// recv 값이 0 이상일 경우 메시지를 받은 게 있기 때문에 아래 로직을 실행한다.
	if (retval > 0)
	{
		// GetRearBufferPtr로 데이터를 복사했기 때문에 rear를 옮겨준다.
		pSession->recvQ.MoveRear(retval);

		while (1)
		{
			// 메시지가 완성되었는지 확인하고 완성되었다면 메시지를 처리한다.
			retval = CheckComplateMessage(pSession);

			// 더 이상 처리할 메시지가 없을 경우 break로 반복문을 빠져 나간다.
			if (retval == 1)
			{
				break;
			}
			else if (retval == -1)
			{
				// 내 잘못일 수도 그리고 client의 조작일 수도 있기 떄문에 끊기가 애매하다.
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
		// 보낼 메시지가 있는데 FindSession에서 찾을 수 없다면 로직이 잘못된다.
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"Send Find Session Eerror, LINE : %d, FILE : %s\n", __LINE__, __FILEW__);

		int* ptr = nullptr;
		*ptr = -1;

		return false;
	}

	int directBufferSize = pSession->sendQ.DirectDequeueSize();

	retval = send(socket, pSession->sendQ.GetFrontBufferPtr(), directBufferSize, 0);
	if (retval == SOCKET_ERROR)
	{
		// retval 이 SOCKET_ERROR일 경우 송신버퍼가 꽉 찼거나 클라이언트가 끊겼다고 판단하여 귾는다.
		_LOG(TRUE,eLogList::LOG_LEVEL_WARNING, L"recv Eerror, Error Code : %d\n", WSAGetLastError());

		//DeleteClient(socket);

		gClearSessionList.push_back(pSession);

		return false;
	}

	pSession->sendQ.MoveFront(retval);

	return true;
}


// 0은 처리완료, 1은 처리 할 거 없음, -1 오류
int CheckComplateMessage(stSession* pSession)
{
	int retval;

	int recvSize = pSession->recvQ.GetUseSize();

	// 수신 링 버퍼에 있는 데이터가 적어도 패킷 헤더 사이즈만큼 있는지 확인한다.
	if (sizeof(stHeader) > recvSize)
	{
		return 1;
	}

	// 헤더 사이즈만큼 있을 경우 헤더 사이즈만큼 Peek하여 헤더 내용을 확인한다.
	stHeader messageHeader;
	retval = pSession->recvQ.Peek((char*)&messageHeader, sizeof(stHeader));
	if(retval != sizeof(stHeader))
	{
		// 이전 로직에서 헤더 사이즈를 확인하고 Peek하였는데 retval 크기가 헤더사이즈와 다를 경우 내 링버퍼에 문제다.
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"stHeader Peek Eerror, retval : %d, GetUseSize : %d \n", retval, pSession->recvQ.GetUseSize());
	
		int* ptr = nullptr;
		*ptr = -1;
	}

	// 패킷 코드가 일치하지 않으면은 조작된 정보이다.
	if (messageHeader.byCode != dfNETWORK_PACKET_CODE)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"Packet Code : %d, Session ID : %d \n", messageHeader.byCode, pSession->sessionID);
		
		// Accept() 로직으로 인해서 무조건 클라이언트 캐릭터가 존재하기 때문에 DeleteClient() 함수로 세션과 캐릭터 정보를 정리한다.
		//DeleteClient(pSession->socket);

		gClearSessionList.push_back(pSession);

		return -1;
	}

	// 링버퍼 사이즈가 메시지 크기보다 작을 경우 다음 recv에 처리하기 위해 return 한다.
	if (sizeof(stHeader) + messageHeader.bySize > recvSize)
	{
		return 1;
	}

	pSession->recvQ.MoveFront(sizeof(stHeader));

	CMessage message;

	if (!pSession->recvQ.Dequeue((char*)message.GetBufferPtr(), messageHeader.bySize))
	{
		// 내 링버퍼의 문제가 있을 경우이다.
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"recvQ Size :%d \n", pSession->recvQ.GetUseSize());

		int* ptr = nullptr;
		*ptr = -1;
	}

	// 메시지에 buffer 포인터로 직접 넣었기 때문에 읽은 크기만큼 message의 rear를 옮겨준다.
	message.MoveWritePos(messageHeader.bySize);

	try
	{
		if (!RecvMessageProcessing(pSession, messageHeader.byType, &message))
		{
			// 내 로직에 문제가 있었을 경우도 있고, 조작될 수도 있는 경우	
			_LOG(TRUE,eLogList::LOG_LEVEL_ERROR, L"#RecvMessageProcessing# Session ID : %d \n", pSession->sessionID);

			return -1;
		}
	}
	catch (CExceptionObject exception)
	{
		// 클라이언트의 실수나 클라이언트가 인위적으로 조작된 메시지를 보냈을 경우
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"MessageProcessing Eerror, Session ID : %d\n", pSession->sessionID);

		FILE* fp;

		// 에러 파일 오푼
		fopen_s(&fp, "ErrorDump.txt", "a+t");

		for (int iCnt = 0; iCnt < exception.m_BufferSize; ++iCnt)
		{
			// 메시지 로그
			fprintf_s(fp, "%02x ", exception.m_MessageLog[iCnt]);
		}

		// 에러 함수의 인자 데이터 타입
		fwrite(exception.m_ErrorDataLog, 1, strlen(exception.m_ErrorDataLog), fp);

		//DeleteClient(pSession->socket);
		
		gClearSessionList.push_back(pSession);

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

//		DeleteClient(pSession->socket);

		gClearSessionList.push_back(pSession);

		break;
	}

	// default 같은 경우는 사용자가 조작된 패킷을 보냈다는 의미이다.
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

		//DeleteClient(pSession->socket);

		gClearSessionList.push_back(pSession);

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

	// client의 좌표를 믿고 가고있음
	pCharacter->posX = posX;
	pCharacter->posY = posY;

	if (UpdateSectorPosition(pCharacter))
	{
		// 섹터가 바뀌었다면은 SendUpdateCharacterSector로 브로드캐스팅
		SendUpdateCharacterSector(pCharacter);
		
		return true;
	}

	PackingMoveStart(pMessage, pCharacter->sessionID, pCharacter->moveDirection, pCharacter->posX, pCharacter->posY);

	// 섹터가 바뀌지 않았다면은 직접 패킷 만든 후 AroundSector로 브로드캐스팅
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

		//DeleteClient(pSession->socket);

		gClearSessionList.push_back(pSession);

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
		// 섹터가 바뀌었다면은 SendUpdateCharacterSector로 브로드캐스팅
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

// 액션 부분 확인좀 해야할듯
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
		// 캐릭터를 못찾을리가 없음 내 로직의 문제임.
		// 조작된 클라이언트라고 해도 Accept쪽에서 처리해버림
		_LOG(TRUE,eLogList::LOG_LEVEL_ERROR,L"#SendProcAroundSector# Not Found Character SessionID : %d", pSession->sessionID);

		int* ptr = nullptr;

		*ptr = -1;
	}

	stSectorAround sectorAround;
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &sectorAround);

	stSession* pExistSession;

	// sendMeFlag 가 true일 경우 나한테도 메시지를 보낸다.
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