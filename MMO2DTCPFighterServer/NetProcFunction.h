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

extern std::map<SOCKET, stSession*> gClearSessionMap;


// Socket 으로 세션 찾기.
stSession* FindSession(SOCKET socket);

// 새로운 세션을 생성,등록 한다.
stSession* CreateSession(SOCKET socket);

void DisconnectSession(SOCKET socket);

// DisconnectSession(), DeleteCharacter() 함수 랩핑
void DeleteClient(stSession *pSession);



void CheckClearSessionMap(void);



void SetupNetwork(void);

void CleanUpSession(void);

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

bool EchoMessageProcessing(stSession* pSession, CMessage* pMessage);


void PackingCreateCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp);

void PackingDeleteCharacter(CMessage* pMessage, DWORD sessionID);

void PackingCreateOtherCharacter(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY, BYTE hp);

void PackingMoveStart(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY);

void PackingMoveStop(CMessage* pMessage, DWORD sessionID, BYTE moveDirection, short posX, short posY);

void PackingSynPosition(CMessage* pMessage, DWORD sessionID, short posX, short posY);


void PackingAttack1(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY);

void PackingAttack2(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY);

void PackingAttack3(CMessage* pMessage, DWORD sessionID, BYTE direction, short posX, short posY);

void PackingDamage(CMessage* pMessage, DWORD sessionID, DWORD victimSessionID, BYTE damegeHp);


void PackingEcho(CMessage* pMessage, unsigned int recvTime);


// stSession의 sendQ에 인큐를 한다. 
void SendProcUnicasting(stSession* pSession, CMessage* pMessage);


// 섹터에 있는 Client들에게 sendQ 인큐를 한다.
void SendProcOneSector(int sectorPosX, int sectorPosY, stSession* pExceptSession, CMessage* pMessage);


// pSession 영향권 섹터들한테 snedQ 인큐를 한다.
// sendMeFlag가 true일 경우 pSession에게도 sendQ에 인큐하는데, 싱크 메시지에서 true를 사용한다.
void SendProcAroundSector(stSession* pSession, CMessage* pMessage, bool sendMeFlag = false);


// 공격시에만 영향권을 ( 4 X 4 )로 하여 영향권 클라이언트들에게 sendQ에 인큐한다. 
void SendProcAttackAroundSector(stSession* pSession, CMessage* pMessage, bool sendMeFlag = false);