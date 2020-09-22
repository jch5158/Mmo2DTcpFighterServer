#pragma once

class CFrameSkip
{
public:
	CFrameSkip();

	~CFrameSkip();

	bool FrameSkip();
	
	void FrameCheck();

private:

	//20
	DWORD mMaxFPS;

	// 업데이트 지연될 때 마다 더해지는 멤버변수
	DWORD mSupplementTime;

};


extern CFrameSkip gFrame;
