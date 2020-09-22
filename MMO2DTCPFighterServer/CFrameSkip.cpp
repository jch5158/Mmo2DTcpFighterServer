#include "stdafx.h"
#include "Log.h"
#include "CFrameSkip.h"


CFrameSkip gFrame;

CFrameSkip::CFrameSkip()
{	
	mMaxFPS = 40;

	mSupplementTime = 0;
}

CFrameSkip::~CFrameSkip()
{

}


bool CFrameSkip::FrameSkip()
{
	static DWORD oldTime = timeGetTime();

	DWORD nowTime = timeGetTime();

	mSupplementTime += nowTime - oldTime;

	oldTime = nowTime;

	if (mSupplementTime >= mMaxFPS)
	{
		mSupplementTime -= mMaxFPS;
		
		return true;
	}

	return false;
}


void CFrameSkip::FrameCheck()
{
	static DWORD frameCheck = 25;

	static DWORD timeCheck = timeGetTime();

	static DWORD frameNow = 0;

	frameNow += 1;

	if (timeGetTime() - timeCheck >= 1000)
	{
		frameCheck = frameNow;

		timeCheck = timeGetTime();

		frameNow = 0;

		if (frameCheck != 25)
		{
			_LOG(eLogList::LOG_LEVEL_DEBUG, L"frame : %d\n", frameCheck);
		}
	}



	return;
}