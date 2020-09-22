#include "stdafx.h"
#include "Log.h"
#include "CRingBuffer.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Sector.h"
#include "Character.h"

std::list<stCharacter*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void SetSectorPosition(stCharacter* pCharacter, stSectorPos* pSectorPos)
{
	pSectorPos->posX = pCharacter->posX / (dfRANGE_MOVE_RIGHT / dfSECTOR_MAX_X);

	pSectorPos->posY = pCharacter->posY / (dfRANGE_MOVE_BOTTOM / dfSECTOR_MAX_Y);			
}


void RemoveSectorPosition(stCharacter* pCharacter)
{
	gSector[pCharacter->curSector.posY][pCharacter->curSector.posX].remove(pCharacter);

}

void AddCurCharacterSector(stCharacter* pCharacter)
{
	gSector[pCharacter->curSector.posY][pCharacter->curSector.posX].push_back(pCharacter);
}

void RemoveOldCharacterSector(stCharacter* pCharacter)
{
	gSector[pCharacter->oldSector.posY][pCharacter->oldSector.posX].remove(pCharacter);
}

bool UpdateSectorPosition(stCharacter* pCharacter)
{	
	stSectorPos sectorPos;

	SetSectorPosition(pCharacter, &sectorPos);

	if(pCharacter->curSector.posX == sectorPos.posX && pCharacter->curSector.posY == sectorPos.posY)
	{
		return false;
	}

	pCharacter->oldSector = pCharacter->curSector;

	RemoveOldCharacterSector(pCharacter);

	pCharacter->curSector = sectorPos;

	AddCurCharacterSector(pCharacter);

	return true;
}


void GetSectorAround(int sectorPosX, int sectorPosY, stSectorAround* pSectorAround)
{
	pSectorAround->count = 0;

	--sectorPosX;
	--sectorPosY;

	for (int countY = 0; countY < 3; ++countY)
	{
		if (sectorPosY + countY < 0 || sectorPosY + countY >= dfSECTOR_MAX_Y)
		{
			continue;
		}

		for (int countX = 0; countX < 3; ++countX)
		{
			if (sectorPosX + countX < 0 || sectorPosX + countX >= dfSECTOR_MAX_X)
			{
				continue;
			}

			pSectorAround->around[pSectorAround->count].posY = sectorPosY + countY;
			pSectorAround->around[pSectorAround->count].posX = sectorPosX + countX;
			++pSectorAround->count;

		}
	}
}


void UpdateSectorAround(stCharacter* pCharacter, stSectorAround* pRemoveAroundSector, stSectorAround* pAddAroundSector)
{
	bool findFlag;

	pRemoveAroundSector->count = 0;
	pAddAroundSector->count = 0;

	stSectorAround oldSectorAround;
	stSectorAround curSectorAround;

	oldSectorAround.count = 0;
	curSectorAround.count = 0;

	GetSectorAround(pCharacter->oldSector.posX, pCharacter->oldSector.posY, &oldSectorAround);
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &curSectorAround);


	for (int oldCount = 0; oldCount < oldSectorAround.count; ++oldCount)
	{
		findFlag = false;
	
		for (int curCount = 0; curCount < curSectorAround.count; ++curCount)
		{
			if (oldSectorAround.around[oldCount].posX == curSectorAround.around[curCount].posX &&
				oldSectorAround.around[oldCount].posY == curSectorAround.around[curCount].posY)
			{
				findFlag = true;
				break;
			}
		}

		if (!findFlag)
		{
			pRemoveAroundSector->around[pRemoveAroundSector->count].posX = oldSectorAround.around[oldCount].posX;
			pRemoveAroundSector->around[pRemoveAroundSector->count].posY = oldSectorAround.around[oldCount].posY;

			++pRemoveAroundSector->count;
		}
	}


	for (int curCount = 0; curCount < curSectorAround.count; ++curCount)
	{
		findFlag = false;

		for (int oldCount = 0; oldCount < oldSectorAround.count; ++oldCount)
		{
			if (curSectorAround.around[curCount].posX == oldSectorAround.around[oldCount].posX &&
				curSectorAround.around[curCount].posY == oldSectorAround.around[oldCount].posY)
			{
				findFlag = true;
				break;
			}
		}

		if (!findFlag)
		{
			pAddAroundSector->around[pAddAroundSector->count].posX = curSectorAround.around[curCount].posX;
			pAddAroundSector->around[pAddAroundSector->count].posY = curSectorAround.around[curCount].posY;
		
			++pAddAroundSector->count;
		}
	}
}


void SendUpdateCharacterSector(stCharacter* pCharacter)
{	
	stSectorAround removeAroundSector;
	stSectorAround addAroundSector;

	UpdateSectorAround(pCharacter, &removeAroundSector, &addAroundSector);

	CMessage message;
	
	// ���� �޽��� �����
	PackingDeleteCharacter(&message, pCharacter->sessionID);

	// removeAroundSector�� �ִ� client�鿡�� pCharacter ���� �޽��� ������
	for (int removeCount = 0; removeCount < removeAroundSector.count; ++removeCount)
	{
		SendProcOneSector(removeAroundSector.around[removeCount].posX, removeAroundSector.around[removeCount].posY, nullptr, &message);
	}

	
	// pCharacter���� removeAroundSector�� �ִ� ĳ���� ���� �޽��� ������ 
	for (int removeCount = 0; removeCount < removeAroundSector.count; ++removeCount)
	{
		auto sectorInfo = gSector[removeAroundSector.around[removeCount].posY][removeAroundSector.around[removeCount].posX];
	
		auto sectorIterE = sectorInfo.end();

		for (auto sectorIter = sectorInfo.begin(); sectorIter != sectorIterE; ++sectorIter)
		{
			PackingDeleteCharacter(&message, (*sectorIter)->sessionID);

			SendProcUnicasting(pCharacter->pSession, &message);
		}	
	}


	PackingCreateOtherCharacter(&message, pCharacter->sessionID, pCharacter->moveDirection, pCharacter->posX, pCharacter->posY, pCharacter->hp);

	// addAroundSector�� �ִ� client�鿡�� pChracter ���� �޽��� ������
	for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
	{
		SendProcOneSector(addAroundSector.around[addCount].posX, addAroundSector.around[addCount].posY, nullptr, &message);
	}

	
	// Stop������ ��ũ�� ��ǥ�� �����ϴ� ��, ��ǥ�� �����ϸ��� ������ġ�� �ٲ� �� �ֱ� ������ 
	// ���� �Լ����� ȣ��ȴ�. action�� Stop�� ��� moveStart �Լ��� ȣ������ �ʴ´�.
	switch (pCharacter->action)
	{
	case eKeyList::eACTION_MOVE_LL:

	case eKeyList::eACTION_MOVE_LU:

	case eKeyList::eACTION_MOVE_UU:

	case eKeyList::eACTION_MOVE_RU:

	case eKeyList::eACTION_MOVE_RR:

	case eKeyList::eACTION_MOVE_RD:

	case eKeyList::eACTION_MOVE_DD:

	case eKeyList::eACTION_MOVE_LD:

		PackingMoveStart(&message, pCharacter->sessionID, pCharacter->moveDirection, pCharacter->posX, pCharacter->posY);

		// addAroundSector�� �ִ� client�鿡�� pChracter Move �޽��� ������
		for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
		{
			SendProcOneSector(addAroundSector.around[addCount].posX, addAroundSector.around[addCount].posY, nullptr, &message);
		}
		
		break;
	}

	
	// pCharacter���� addAroundSector�� �ִ� client ���� �޽��� ������ 
	for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
	{
		auto sectorInfo = gSector[addAroundSector.around[addCount].posY][addAroundSector.around[addCount].posX];

		auto sectorIterE = sectorInfo.end();
		for (auto sectorIter = sectorInfo.begin(); sectorIter != sectorIterE; ++sectorIter)
		{
			stCharacter* pExistCharacter = *sectorIter;

			// ���� �����ϰ� �޽����� ������.
			if (pExistCharacter != pCharacter)
			{
				PackingCreateOtherCharacter(&message, pExistCharacter->sessionID, pExistCharacter->moveDirection, pExistCharacter->posX, pExistCharacter->posY, pExistCharacter->hp);

				// ������ �޽����� ������.
				SendProcUnicasting(pCharacter->pSession, &message);	

				switch (pExistCharacter->action)
				{
				case eKeyList::eACTION_MOVE_LL:
				
				case eKeyList::eACTION_MOVE_LU:
				
				case eKeyList::eACTION_MOVE_UU:
				
				case eKeyList::eACTION_MOVE_RU:
				
				case eKeyList::eACTION_MOVE_RR:
				
				case eKeyList::eACTION_MOVE_RD:
				
				case eKeyList::eACTION_MOVE_DD:
				
				case eKeyList::eACTION_MOVE_LD:
					
					PackingMoveStart(&message, pExistCharacter->sessionID, pExistCharacter->moveDirection, pExistCharacter->posX, pExistCharacter->posY);

					SendProcUnicasting(pCharacter->pSession, &message);
					
					break;
				}
			}
		}
	}

}