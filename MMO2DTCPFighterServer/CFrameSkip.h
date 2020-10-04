#pragma once

#define TARGET_FRAME 25

class CFrameSkip
{
public:
	CFrameSkip();

	~CFrameSkip();

	bool FrameSkip();
		
private:

	void getFrameAvrage(DWORD frame);

	DWORD mAvgDeltaTime;
	DWORD mMaxDeltaTime;
	DWORD mMinDeltaTime;
	DWORD mFrameCount;

	// 1초 당 프레임 체크
	int mOneSecFrame;

	// 40
	int mMaxFPS;

	// 업데이트 지연될 때 마다 더해지는 멤버변수
	int mSupplementTime;
};


extern CFrameSkip gFrame;
