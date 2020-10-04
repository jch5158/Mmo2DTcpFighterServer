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

	// 1�� �� ������ üũ
	int mOneSecFrame;

	// 40
	int mMaxFPS;

	// ������Ʈ ������ �� ���� �������� �������
	int mSupplementTime;
};


extern CFrameSkip gFrame;
