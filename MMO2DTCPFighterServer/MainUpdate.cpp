#include "stdafx.h"
#include "Log.h"
#include "CFrameSkip.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Sector.h"
#include "Character.h"
#include "MainUpdate.h"


void Update()
{

	if (!gFrame.FrameSkip())
	{
		return;
	}



	stCharacter* pCharecter = nullptr;

	auto charecterIterE = gCharacterMap.end();
	for (auto charecterIter = gCharacterMap.begin(); charecterIter != charecterIterE;)
	{
		pCharecter = charecterIter->second;
		++charecterIter;

		if (pCharecter->hp <= 0)
		{	
			DeleteClient(pCharecter->pSession->socket);
		}
		else
		{
			switch (pCharecter->action)
			{
			case eKeyList::eACTION_MOVE_LL:

				if (CheckCharacterMove(pCharecter->posX-dfSPEED_PLAYER_X, pCharecter->posY))
				{
					pCharecter->posX -= dfSPEED_PLAYER_X;
				}

				break;

			case eKeyList::eACTION_MOVE_LU:

				if (CheckCharacterMove(pCharecter->posX - dfSPEED_PLAYER_X, pCharecter->posY - dfSPEED_PLAYER_Y))
				{
					pCharecter->posX -= dfSPEED_PLAYER_X;
					pCharecter->posY -= dfSPEED_PLAYER_Y;
				}

				break;

			case eKeyList::eACTION_MOVE_UU:

				if (CheckCharacterMove(pCharecter->posX, pCharecter->posY - dfSPEED_PLAYER_Y))
				{
					pCharecter->posY -= dfSPEED_PLAYER_Y;
				}

				break;

			case eKeyList::eACTION_MOVE_RU:

				if (CheckCharacterMove(pCharecter->posX + dfSPEED_PLAYER_X, pCharecter->posY- dfSPEED_PLAYER_Y))
				{
					pCharecter->posX += dfSPEED_PLAYER_X;
					pCharecter->posY -= dfSPEED_PLAYER_Y;
				}

				break;

			case eKeyList::eACTION_MOVE_RR:

				if (CheckCharacterMove(pCharecter->posX + dfSPEED_PLAYER_X, pCharecter->posY))
				{
					pCharecter->posX += dfSPEED_PLAYER_X;
				}

				break;

			case eKeyList::eACTION_MOVE_RD:

				if (CheckCharacterMove(pCharecter->posX + dfSPEED_PLAYER_X, pCharecter->posY + dfSPEED_PLAYER_Y))
				{
					pCharecter->posX += dfSPEED_PLAYER_X;
					pCharecter->posY += dfSPEED_PLAYER_Y;
				}

				break;
					
			case eKeyList::eACTION_MOVE_DD:

				if (CheckCharacterMove(pCharecter->posX, pCharecter->posY + dfSPEED_PLAYER_Y))
				{
					pCharecter->posY += dfSPEED_PLAYER_Y;
				}

				break;

			case eKeyList::eACTION_MOVE_LD:

				if (CheckCharacterMove(pCharecter->posX - dfSPEED_PLAYER_X, pCharecter->posY + dfSPEED_PLAYER_Y))
				{
					pCharecter->posX -= dfSPEED_PLAYER_X;
					pCharecter->posY += dfSPEED_PLAYER_Y;
				}

				break;
			}

			if (eKeyList::eACTION_MOVE_LL <= pCharecter->action && eKeyList::eACTION_MOVE_LD >= pCharecter->action )
			{
				//섹터 변경이 있었는지 체크한다.
				if (UpdateSectorPosition(pCharecter))
				{
					// 섹터 변경이 있으면은 SendUpdateCharacterSector 로직을 수행한다.
					SendUpdateCharacterSector(pCharecter);
				}			
			}

		}
	}
}