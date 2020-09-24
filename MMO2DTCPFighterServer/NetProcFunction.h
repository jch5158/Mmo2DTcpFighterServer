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

void DisconnectSession(SOCKET socket);


// DisconnectSession(), DeleteCharacter() 함수 랩핑
void DeleteClient(SOCKET socket);


void CleanUpSession(void);


void SetupNetwork(void);

void CleanUpNetwork(void);

void NetworkProcessing(void);

void SelectSocket(SOCKET* pSocketTable, FD_SET* pWriteSet, FD_SET* pReadSet);
 
void Accept(void);

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


void PackingCreateCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp);

void PackingDeleteCharacter(CMessage* pMessage, DWORD sessionID);

void PackingCreateOtherCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp);

void PackingMoveStart(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY);

void PackingMoveStop(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY);

void PackingSynPosition(CMessage* pMessage, DWORD sessionID, short posX, short posY);


void SendProcUnicasting(stSession* pSession, CMessage* pMessage);

void SendProcOneSector(int sectorPosX, int sectorPosY, stSession* pExceptSession, CMessage* pMessage);

void SendProcAroundSector(stSession* pSession, CMessage* pMessage, bool sendMeFlag = false);