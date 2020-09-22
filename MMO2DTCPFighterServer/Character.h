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

extern std::unordered_map<DWORD, stCharacter*> gCharacterMap;

stCharacter* CreateCharacter(stSession* pSession, DWORD action, BYTE direction, BYTE moveDirection, short posX, short posY);

stCharacter* FindCharacter(DWORD sessionID);

void DeleteCharacter(stSession* pSession);

bool CheckCharacterMove(short posX, short posY);