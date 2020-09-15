#pragma once


struct Character
{
	stSession* pSession;
	DWORD sessionID;

	DWORD action;
	BYTE direction;
	BYTE moveDirection;

	short posX;
	short posY;

	char hp;
};

extern std::unordered_map<DWORD, Character*> gCharacterMap;


Character* FindCharacter(DWORD sessionID);