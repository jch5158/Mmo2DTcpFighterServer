#pragma once

class CFrameSkip
{
public:
	CFrameSkip();

	~CFrameSkip();

	bool FrameSkip();
		
private:

	void getFrameAvrage(DWORD frame);

	DWORD mAvgFrameTime;
	DWORD mMinFrameTime;
	DWORD mMaxFrameTime;
	DWORD mFrameCount;

	// 1초 당 프레임 체크
	DWORD mOneSecFrame;

	//20
	DWORD mMaxFPS;

	// 업데이트 지연될 때 마다 더해지는 멤버변수
	DWORD mSupplementTime;
};


extern CFrameSkip gFrame;
