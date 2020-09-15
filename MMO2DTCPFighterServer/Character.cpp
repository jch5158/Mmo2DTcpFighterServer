#include "stdafx.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Character.h"

std::unordered_map<DWORD, Character*> gCharacterMap;

Character* FindCharacter(DWORD sessionID)
{
	auto characterIterE = gCharacterMap.end();

	auto characterIter = gCharacterMap.find(sessionID);

	if (characterIter == characterIterE)
	{
		return nullptr;
	}

	return characterIter->second;
}