#pragma once

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
	DWORD mOneSecFrame;

	//20
	DWORD mMaxFPS;

	// ������Ʈ ������ �� ���� �������� �������
	DWORD mSupplementTime;
};


extern CFrameSkip gFrame;
