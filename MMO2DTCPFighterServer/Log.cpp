#include "stdafx.h"
#include "Log.h"

int gLogLevel;

WCHAR gLogBuffer[1024];


void DoPrintLog(int logLevel, WCHAR* logBuffer)
{

	switch (logLevel)
	{
	case eLogList::LOG_LEVEL_DEBUG:

		wprintf_s(L"[ logLevel : dfLOG_LEVEL_DEBUG ] %s\n", logBuffer);

		break;

	case eLogList::LOG_LEVEL_WARNING:
		
		wprintf_s(L"[ logLevel : dfLOG_LEVEL_WARNING ] %s\n", logBuffer);

		break;

	case eLogList::LOG_LEVEL_NOTICE:

		wprintf_s(L"[ logLevel : LOG_LEVEL_NOTICE ] %s\n", logBuffer);

		break;

	case eLogList::LOG_LEVEL_ERROR:

		wprintf_s(L"[ logLevel : LOG_LEVEL_ERROR ] %s\n", logBuffer);

		break;
	default:

		wprintf_s(L"logLevel : %d , LINE : %d, FILE : %s\n", logLevel, __LINE__, __FILEW__);

		int* ptr = nullptr;

		*ptr = -1;
		
		break;
	}

	return;
}



void SettingLogLevel()
{
	wprintf_s(L"1. LOG_LEVEL_DEBUG\n2. LOG_LEVEL_WARNING\n3. LOG_LEVEL_NOTICE\n4. LOG_LEVEL_ERROR\n");
	wprintf_s(L"로그 레벨을 입력하세요 : ");

	WCHAR buffer[10];
	wscanf_s(L"%s", buffer, sizeof(WCHAR) * 10);

	gLogLevel = _wtoi(buffer);

}