#include "stdafx.h"
#include "Log.h"
#include "CFrameSkip.h"


CFrameSkip gFrame;

CFrameSkip::CFrameSkip()
{	
	mAvgFrameTime = 0;
	mMinFrameTime = 0;
	mMaxFrameTime = 0;
	mFrameCount = 0;

	mOneSecFrame = 0;

	mMaxFPS = 40;

	mSupplementTime = 0;
}

CFrameSkip::~CFrameSkip()
{

}


bool CFrameSkip::FrameSkip()
{
	static DWORD oldTime = timeGetTime();

	static DWORD timeCheck = timeGetTime();

	DWORD nowTime = timeGetTime();

	int frameTime = nowTime - oldTime;

	mSupplementTime += frameTime;

	getFrameAvrage(frameTime);

	oldTime = nowTime;

	if (mSupplementTime >= mMaxFPS)
	{
		mSupplementTime -= mMaxFPS;

		mOneSecFrame += 1;

		if (nowTime - timeCheck >= 1000)
		{
			// mFrameCheck 가 25가 아닐경우는 프레임이 틀어져서 Sync가 발생할 확률이 높다.
			if (mOneSecFrame == 25)
			{
				_LOG(FALSE, eLogList::LOG_LEVEL_DEBUG, L"frame : %d, avgFrameTime : %f, maxFrameTime : %d, minFrameTime :%d \n", mOneSecFrame, (double)((double)mAvgFrameTime / (double)mFrameCount), mMaxFrameTime, mMinFrameTime);
			}
			else 
			{
			    _LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"frame : %d, avgFrameTime : %f, maxFrameTime : %d, minFrameTime :%d \n", mOneSecFrame, (double)((double)mAvgFrameTime / (double)mFrameCount), mMaxFrameTime, mMinFrameTime);
			}

			mAvgFrameTime = 0;
			mMaxFrameTime = 0;
			mMinFrameTime = 0;
			mFrameCount = 0;

			mOneSecFrame = 0;

			timeCheck = nowTime;
		}

		return true;
	}

	return false;
}


void CFrameSkip::getFrameAvrage(DWORD frame)
{
	if (mMaxFrameTime < frame)
	{
		mMaxFrameTime = frame;
	}
	else if (mMinFrameTime > frame)
	{
		mMinFrameTime = frame;
	}

	mFrameCount += 1;

	mAvgFrameTime += frame;
}