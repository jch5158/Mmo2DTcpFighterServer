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

// Socket ���� ���� ã��.
stSession* FindSession(SOCKET socket);

// ���ο� ������ ����,��� �Ѵ�.
stSession* CreateSession(SOCKET socket);

// �ش� ���� ����ó��.
// ���Ǹ� �����ϴ� ���� ���� ���� DeleteClient�� �ؼ� 
// �ش� ���� ������ ���� Ŭ���̾�Ʈ�� ���ҽ��� ��� �����ϴ� ���� ���� �� ����.
void DisconnectSession(SOCKET socket);

bool SetupNetwork(void);

void NetworkProcessing(void);

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet);
 
void Accept();

void RecvEvent(SOCKET socket);

bool SendEvent(SOCKET socket);

// 0�� ó���Ϸ�, 1�� ó�� �� �� ����, -1 ����
int CheckComplateMessage(stSession* pSession);


bool RecvMessageProcessing(stSession* pSession, BYTE messageType, CMessage* pMessage);


bool MoveStartMessageProcessing(stSession* pSession, CMessage* pMessage);

bool MoveStopMessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack1MessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack2MessageProcessing(stSession* pSession, CMessage* pMessage);

bool Attack3MessageProcessing(stSession* pSession, CMessage* pMessage);
