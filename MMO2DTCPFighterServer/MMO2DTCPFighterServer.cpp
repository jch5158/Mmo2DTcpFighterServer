#include "stdafx.h"
#include "Log.h"
#include "ServerControl.h"
#include "CRingBuffer.h"
#include "Protocol.h"
#include "CExceptionObject.h"
#include "CMessage.h"
#include "NetProcFunction.h"

bool gShutdwonFlag = false;

int main()
{

	timeBeginPeriod(1);

	setlocale(LC_ALL, "");

	SettingLogLevel();
	
	// 리슨 소켓 셋팅
	SetupNetwork();	

	while (!gShutdwonFlag)
	{
		NetworkProcessing();

		// TODO : 컨텐츠 로직 파트

		ServerControling();
	}
	
	timeEndPeriod(1);
}
