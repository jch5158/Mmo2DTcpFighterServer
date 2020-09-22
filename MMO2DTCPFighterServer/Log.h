#pragma once

enum eLogList 
{
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_NOTICE,
	LOG_LEVEL_ERROR
};

#define _LOG(logLevel,fmt,...)											\
do																		\
{																		\
	if (gLogLevel <= logLevel)											\
	{																	\
		wsprintf(gLogBuffer,fmt, ##__VA_ARGS__);						\
		DoPrintLog(logLevel, gLogBuffer);								\
	}																	\
}while(0)																\


void DoPrintLog(int logLevel, WCHAR * logBuffer);

void SettingLogLevel();

extern int gLogLevel;

extern WCHAR gLogBuffer[1024];