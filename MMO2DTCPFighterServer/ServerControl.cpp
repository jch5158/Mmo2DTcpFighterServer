#include "stdafx.h"
#include "MMO2DTCPFighterServer.h"
#include "ServerControl.h"


void ServerControling(void)
{
	static bool controlModeFlag = false;


	if (_kbhit())
	{
		WCHAR controlKey = _getwch();

		if (controlKey == L'u' || controlKey == L'U')
		{
			wprintf_s(L"Q 입력 시 서버 종료 \n");	
			wprintf_s(L"L 입력 시 입력모드 종료\n");

			controlModeFlag = true;
		}

		if (controlKey == L'l' || controlKey == L'L')
		{
			wprintf(L"입력 모드 종료\n");
			controlModeFlag = false;
		}

		if ((controlKey == L'q' || controlKey == L'Q') && controlModeFlag)
		{
			gShutdwonFlag = true;
		}

	}
}