#include "stdafx.h"
#include "Log.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Sector.h"
#include "Character.h"

std::unordered_map<DWORD, stCharacter*> gCharacterMap;

stCharacter* CreateCharacter(stSession* pSession, DWORD action, BYTE direction, BYTE moveDirection, short posX, short posY)
{
	stCharacter* pCharacter = FindCharacter(pSession->sessionID);
	if (pCharacter != nullptr)
	{
		return nullptr;
	}

	pCharacter = (stCharacter*)malloc(sizeof(stCharacter));

	pCharacter->pSession = pSession;
	pCharacter->sessionID = pSession->sessionID;

	pCharacter->action = action;
	pCharacter->direction = direction;
	pCharacter->moveDirection = moveDirection;
	pCharacter->posX = posX;
	pCharacter->posY = posY;

	// �� ��ǥ ��ġ�� �´� ���� ����
	stSectorPos curSectorPos;
	SetSectorPosition(pCharacter, &curSectorPos);
	pCharacter->curSector = curSectorPos;

	// �ش� ĳ������ ���� ��ġ�� �ش��ϴ� ���͸���Ʈ�� �߰�
	AddCurCharacterSector(pCharacter);

	pCharacter->hp = 100;

	return pCharacter;
}

stCharacter* FindCharacter(DWORD sessionID)
{
	auto characterIterE = gCharacterMap.end();

	auto characterIter = gCharacterMap.find(sessionID);

	if (characterIter == characterIterE)
	{
		return nullptr;
	}

	return characterIter->second;
}



void DeleteCharacter(DWORD sessionID)
{
	stCharacter* pCharacter = FindCharacter(sessionID);
	if (pCharacter == nullptr)
	{
		_LOG(eLogList::LOG_LEVEL_ERROR, L"DeleteCharacter Error LINE : %d, FILE : %s", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	gCharacterMap.erase(sessionID);

	free(pCharacter);
}


bool CheckCharacterMove(short posX, short posY)
{
	if (dfRANGE_MOVE_RIGHT > posX && dfRANGE_MOVE_LEFT < posX && dfRANGE_MOVE_TOP < posY && dfRANGE_MOVE_BOTTOM  > posY)
	{
		return true;
	}

	return false;
}