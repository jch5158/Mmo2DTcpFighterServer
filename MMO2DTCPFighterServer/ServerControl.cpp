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
			wprintf_s(L"Q �Է� �� ���� ���� \n");	
			wprintf_s(L"L �Է� �� �Է¸�� ����\n");

			controlModeFlag = true;
		}

		if (controlKey == L'l' || controlKey == L'L')
		{
			wprintf(L"�Է� ��� ����\n");
			controlModeFlag = false;
		}

		if (controlKey == L'q' || controlKey == L'Q')
		{
			gShutdwonFlag = true;
		}

	}
}