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

	// 내 좌표 위치에 맞는 섹터 셋팅
	stSectorPos curSectorPos;

	// 현재 위치 기준으로 섹터 셋팅
	SetSectorPosition(pCharacter, &curSectorPos);
	pCharacter->curSector = curSectorPos;

	// pCharacter 섹터 위치에 해당하는 섹터리스트에 pCharaceter를 추가
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



void DeleteCharacter(stSession* pSession)
{
	stCharacter* pCharacter = FindCharacter(pSession->sessionID);
	if (pCharacter == nullptr)
	{
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"DeleteCharacter Error LINE : %d, FILE : %s", __LINE__, __FILEW__);
		int* ptr = nullptr;
		*ptr = -1;
	}

	CMessage message;

	// pSession 캐릭터 삭제 메시지 만들기
	PackingDeleteCharacter(&message, pSession->sessionID);

	// pSession 영향권 Client들에게 삭제 메시지 보내기
	SendProcAroundSector(pSession, &message);

	// 해당 섹터 리스트에서 캐릭터 정보 정리하기
	RemoveSectorPosition(pCharacter);

	gCharacterMap.erase(pSession->sessionID);

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