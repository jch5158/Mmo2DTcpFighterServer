#pragma once

#define SERVER_PORT 20000
#define SERVER_IP L"0.0.0.0"

struct stSession
{
	SOCKET socket;
	DWORD sessionID;
	CRingBuffer recvQ;
	CRingBuffer sendQ;
	DWORD lastRecvTime;
};

extern SOCKET gListenSocket;

extern std::unordered_map<SOCKET, stSession*> gSessionMap;

// Socket 으로 세션 찾기.
stSession* FindSession(SOCKET socket);

// 새로운 세션을 생성,등록 한다.
stSession* CreateSession(SOCKET socket);

// 해당 세션 종료처리.
// 세션만 삭제하는 경우는 없다 차라리 DeleteClient로 해서 
// 해당 세션 정보를 가진 클라이언트의 리소스를 모두 정리하는 것이 좋을 것 같다.
void DisconnectSession(SOCKET socket);

bool SetupNetwork(void);

void NetworkProcessing(void);

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet);
 
void Accept();

void RecvEvent(SOCKET socket);

bool SendEvent(SOCKET socket);

// 0은 처리완료, 1은 처리 할 거 없음, -1 오류
int CheckComplateMessage(stSession* pSession);


bool RecvMessageProcessing(stSession* pSession, BYTE messageType, CMessage* pMessage);


bool MoveStartMessageProcessing(stSession* pSession, CMessage* pMessage);

bool MoveStopMessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack1MessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack2MessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack3MessageProcessing(stSession* pSession, CMessage* pMessage);
