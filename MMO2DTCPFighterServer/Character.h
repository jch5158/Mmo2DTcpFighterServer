#pragma once


struct stCharacter
{
	stSession* pSession;
	DWORD sessionID;

	DWORD action;
	BYTE direction;
	BYTE moveDirection;

	short posX;
	short posY;

	stSectorPos oldSector;
	stSectorPos curSector;

	char hp;
};

extern std::map<DWORD, stCharacter*> gCharacterMap;

stCharacter* CreateCharacter(stSession* pSession, DWORD action, BYTE direction, BYTE moveDirection, short posX, short posY);

stCharacter* FindCharacter(DWORD sessionID);

void DeleteCharacter(stSession* pSession);

bool CheckCharacterMove(short posX, short posY);

void CleanUpCharacter(void);

void Attack1ColisionCheck(stCharacter *pCharacter,stCharacter **pVictimCharacter);

void Attack2ColisionCheck(stCharacter* pCharacter, stCharacter** pVictimCharacter);

void Attack3ColisionCheck(stCharacter* pCharacter, stCharacter** pVictimCharacter);