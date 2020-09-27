#include "stdafx.h"
#include "Log.h"
#include "CFrameSkip.h"


CFrameSkip gFrame;

CFrameSkip::CFrameSkip()
{	
	mAvgDeltaTime = 0;
	mMinDeltaTime = UINT_MAX;
	mMaxDeltaTime = 0;
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

	static DWORD deltaTimeCheck = timeGetTime();
	
	DWORD nowTime = timeGetTime();
	
	mSupplementTime += nowTime - oldTime;

	oldTime = nowTime;

	if (mSupplementTime >= mMaxFPS)
	{
		mSupplementTime -= mMaxFPS;

		mOneSecFrame += 1;

		getFrameAvrage(nowTime - deltaTimeCheck);

		deltaTimeCheck = timeGetTime();

		if (nowTime - timeCheck >= 1000)
		{
			// mFrameCheck �� 25�� �ƴҰ��� �������� Ʋ������ Sync�� �߻��� Ȯ���� ����.
			if (mOneSecFrame == 25)
			{
			//	_LOG(FALSE, eLogList::LOG_LEVEL_DEBUG, L"frame : %d, avgDeltaTime : %f, maxDeltaTime : %d, minDeltaTime :%d \n", mOneSecFrame, (double)((double)mAvgDeltaTime / (double)mFrameCount), mMaxDeltaTime, mMinDeltaTime);
			}
			else 
			{
			    _LOG(TRUE, eLogList::LOG_LEVEL_ERROR, L"frame : %d, avgDeltaTime : %f, maxDeltaTime : %d, minDeltaTime :%d \n", mOneSecFrame, (double)((double)mAvgDeltaTime / (double)mFrameCount), mMaxDeltaTime, mMinDeltaTime);
			}

			mAvgDeltaTime = 0;
			mMaxDeltaTime = 0;
			mMinDeltaTime = UINT_MAX;
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
	if (mMaxDeltaTime < frame)
	{
		mMaxDeltaTime = frame;
	}
	else if (mMinDeltaTime > frame)
	{
		mMinDeltaTime = frame;
	}

	mFrameCount += 1;

	mAvgDeltaTime += frame;
}