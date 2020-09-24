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

	// ���� ��ġ �������� ���� ����
	SetSectorPosition(pCharacter, &curSectorPos);
	pCharacter->curSector = curSectorPos;

	// pCharacter ���� ��ġ�� �ش��ϴ� ���͸���Ʈ�� pCharaceter�� �߰�
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
		_LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"DeleteCharacter NotFound SessionID : %d", pSession->sessionID);
		
		return;
	}

	CMessage message;

	// pSession ĳ���� ���� �޽��� �����
	PackingDeleteCharacter(&message, pSession->sessionID);

	// pSession ����� Client�鿡�� ���� �޽��� ������
	SendProcAroundSector(pSession, &message);

	// �ش� ���� ����Ʈ���� ĳ���� ���� �����ϱ�
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

void CleanUpCharacter(void)
{

	auto iterE = gCharacterMap.end();

	for (auto iter = gCharacterMap.begin(); iter != iterE;)
	{
		auto deleteIter = iter;

		++iter;

		RemoveSectorPosition((*deleteIter).second);

		gCharacterMap.erase((*deleteIter).second->sessionID);
	}
}