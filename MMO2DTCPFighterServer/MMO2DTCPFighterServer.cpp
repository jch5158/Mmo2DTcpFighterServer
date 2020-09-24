#include "stdafx.h"
#include "Log.h"
#include "ServerControl.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"
#include "Sector.h"
#include "Character.h"
#include "MainUpdate.h"
#include "MMO2DTCPFighterServer.h"

bool gShutdwonFlag = false;

int main()
{

	timeBeginPeriod(1);

	srand((unsigned)time(NULL));

	setlocale(LC_ALL, "");

	SettingLogLevel();
	
	// 리슨 소켓 셋팅
	SetupNetwork();	

	while (!gShutdwonFlag)
	{
		NetworkProcessing();

		Update();
		
		ServerControling();
	}
	
	CleanUpAll();

	timeEndPeriod(1);
}


void CleanUpAll()
{
	CleanUpCharacter();
	
	CleanUpSession();

	CleanUpNetwork();
}