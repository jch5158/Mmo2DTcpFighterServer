#include "stdafx.h"
#include "Log.h"

int gLogLevel;

WCHAR gLogBuffer[1024];

void DoPrintLog(bool filePrintFlag, int logLevel, WCHAR* logBuffer)
{	
	FILE* fp = nullptr;

	static WCHAR logInfo[500];

	tm stTm;

	__time64_t nowTime;

	time(&nowTime);

	_localtime64_s(&stTm, &nowTime);

	switch (logLevel)
	{
	case eLogList::LOG_LEVEL_DEBUG:

		_wfopen_s(&fp, L"LogData.txt", L"a+t");
	
		wsprintf(logInfo, L"[%02d/%02d/%02d %02d:%02d:%02d][dfLOG_LEVEL_DEBUG] ", stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
		
		if (filePrintFlag)
		{
			fwprintf_s(fp, logInfo);
			fwprintf_s(fp, logBuffer);
		}

		wprintf_s(L"%s", logInfo);
		wprintf_s(L"%s", logBuffer);
		
		fclose(fp);

		break;

	case eLogList::LOG_LEVEL_WARNING:

		_wfopen_s(&fp, L"LogData.txt", L"a+t");

		wsprintf(logInfo, L"[%02d/%02d/%02d %02d:%02d:%02d][LOG_LEVEL_WARNING] ", stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
	
		if (filePrintFlag)
		{
			fwprintf_s(fp, logInfo);
			fwprintf_s(fp, logBuffer);
		}

		wprintf_s(L"%s", logInfo);
		wprintf_s(L"%s", logBuffer);

		fclose(fp);

		break;

	case eLogList::LOG_LEVEL_NOTICE:

		_wfopen_s(&fp, L"LogData.txt", L"a+t");

		wsprintf(logInfo, L"[%02d/%02d/%02d %02d:%02d:%02d][LOG_LEVEL_NOTICE] ", stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

		if (filePrintFlag)
		{
			fwprintf_s(fp, logInfo);
			fwprintf_s(fp, logBuffer);
		}


		wprintf_s(L"%s", logInfo);
		wprintf_s(L"%s", logBuffer);

		fclose(fp);

		break;

	case eLogList::LOG_LEVEL_ERROR:

		_wfopen_s(&fp, L"LogData.txt", L"a+t");

		wsprintf(logInfo, L"[%02d/%02d/%02d %02d:%02d:%02d][LOG_LEVEL_ERROR] ", stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

		if (filePrintFlag)
		{
			fwprintf_s(fp, logInfo);
			fwprintf_s(fp, logBuffer);
		}

		wprintf_s(L"%s", logInfo);
		wprintf_s(L"%s", logBuffer);

		fclose(fp);

		break;
	default:

		_wfopen_s(&fp, L"LogData.txt", L"a+t");

		wsprintf(logInfo, L"[%02d/%02d/%02d %02d:%02d:%02d] ", stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

		if (filePrintFlag)
		{
			fwprintf_s(fp, logInfo);
			fwprintf_s(fp, logBuffer);
		}

		wprintf_s(L"%s", logInfo);
		wprintf_s(L"logLevel : %d , LINE : %d, FILE : %s\n", logLevel, __LINE__, __FILEW__);

		fclose(fp);

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