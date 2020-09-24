#pragma once

enum eLogList 
{
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_NOTICE,
	LOG_LEVEL_ERROR
};

#define _LOG(filePrintFlag, logLevel,fmt,...)											\
do																		\
{																		\
	if (gLogLevel <= logLevel)											\
	{																	\
		swprintf_s(gLogBuffer,fmt, ##__VA_ARGS__);						\
		DoPrintLog(filePrintFlag,logLevel, gLogBuffer);								\
	}																	\
}while(0)																\


void DoPrintLog(bool filePrintFalg, int logLevel, WCHAR * logBuffer);

void SettingLogLevel();

extern int gLogLevel;

extern WCHAR gLogBuffer[1024];