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
	int posX;

	int posY;

	if (pCharacter->posX < 0)
	{
		posX = 0;
	}
	else if (pCharacter->posY > 6400)
	{
		posX = 6400;
	}
	else
	{
		posX = pCharacter->posX;
	}

	if (pCharacter->posY < 0)
	{
		posY = 0;
	}
	else if (pCharacter->posY > 6400)
	{
		posY = 6400;
	}
	else
	{
		posY = pCharacter->posY;
	}

	pSectorPos->posX = posX / (dfRANGE_MOVE_RIGHT / dfSECTOR_MAX_X);

	pSectorPos->posY = posY / (dfRANGE_MOVE_BOTTOM / dfSECTOR_MAX_Y);			
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

void GetAttackSectorAround(stCharacter* pCharacter, stAttackSectorAround* pAttackSectorAround)
{
	pAttackSectorAround->count = 0;

	int sectorPosX = pCharacter->curSector.posX;
	int sectorPosY = pCharacter->curSector.posY;

	
	if (pCharacter->direction == eKeyList::eACTION_MOVE_RR)
	{
		sectorPosX -= 1;
	}
	else
	{
		sectorPosX -= 2;
	}
	
	sectorPosY -= 2;

	for (int countY = 0; countY < 4; ++countY)
	{
		if (sectorPosY + countY < 0 || sectorPosY + countY >= dfSECTOR_MAX_Y)
		{
			continue;
		}


		for (int countX = 0; countX < 4; ++countX)
		{
			if (sectorPosX + countX < 0 || sectorPosX + countX >= dfSECTOR_MAX_X)
			{
				continue;
			}

			pAttackSectorAround->around[pAttackSectorAround->count].posY = sectorPosY + countY;
			pAttackSectorAround->around[pAttackSectorAround->count].posX = sectorPosX + countX;
			++pAttackSectorAround->count;
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

	GetSectorAround(pCharacter->oldSector.posX, pCharacter->oldSector.posY, &oldSectorAround);
	GetSectorAround(pCharacter->curSector.posX, pCharacter->curSector.posY, &curSectorAround);


	for (int oldCount = 0; oldCount < oldSectorAround.count; ++oldCount)
	{
		findFlag = false;
	
		for (int curCount = 0; curCount < curSectorAround.count; ++curCount)
		{

			// oldSectorAround.around[oldCount] 와 curSectorAround.around[curCount]가 좌표가 같다면 
			// 삭제 섹터가 아니라 유지해야 될 섹터이다. findFlag = true로 하여 pRemoveAroundSector에 
			// oldSectorAround.around[oldCount] 가 추가되지 않도록 한다.
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
			// curSectorAround.around[curCount] 와 oldSectorAround.around[oldCount]가 좌표가 같다면
			// 새로 추가된 섹터가 아니기 때문에 findFlag = true로 하여 pAddAroundSector 에 curSectorAround.around[curCount]
			// 값을 추가되지 않도록 한다.
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
	
	// 삭제 메시지 만들기
	PackingDeleteCharacter(&message, pCharacter->sessionID);

	// removeAroundSector에 있는 client들에게 pCharacter 삭제 메시지 보내기
	for (int removeCount = 0; removeCount < removeAroundSector.count; ++removeCount)
	{
		SendProcOneSector(removeAroundSector.around[removeCount].posX, removeAroundSector.around[removeCount].posY, nullptr, &message);
	}

	
	// pCharacter에게 removeAroundSector에 있는 캐릭터 삭제 메시지 보내기 
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

	// addAroundSector에 있는 client들에게 pChracter 생성 메시지 보내기
	for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
	{
		SendProcOneSector(addAroundSector.around[addCount].posX, addAroundSector.around[addCount].posY, nullptr, &message);
	}

	
	// Stop에서도 싱크로 좌표를 조정하는 데, 좌표를 조정하면은 섹터위치가 바뀔 수 있기 때문에 
	// 섹터 함수들이 호출된다. action이 Stop일 경우 moveStart 함수를 호출하지 않는다.
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

		// addAroundSector에 있는 client들에게 pChracter Move 메시지 보내기
		for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
		{
			SendProcOneSector(addAroundSector.around[addCount].posX, addAroundSector.around[addCount].posY, nullptr, &message);
		}
		
		break;
	}

	
	// pCharacter에게 addAroundSector에 있는 client 생성 메시지 보내기 
	for (int addCount = 0; addCount < addAroundSector.count; ++addCount)
	{
		auto sectorInfo = gSector[addAroundSector.around[addCount].posY][addAroundSector.around[addCount].posX];

		auto sectorIterE = sectorInfo.end();
		for (auto sectorIter = sectorInfo.begin(); sectorIter != sectorIterE; ++sectorIter)
		{
			stCharacter* pExistCharacter = *sectorIter;

			// 나를 제외하고 메시지를 보낸다.
			if (pExistCharacter != pCharacter)
			{
				PackingCreateOtherCharacter(&message, pExistCharacter->sessionID, pExistCharacter->moveDirection, pExistCharacter->posX, pExistCharacter->posY, pExistCharacter->hp);

				// 나에게 메시지를 보낸다.
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