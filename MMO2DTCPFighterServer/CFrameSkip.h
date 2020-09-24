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

	// 1�� �� ������ üũ
	DWORD mOneSecFrame;

	//20
	DWORD mMaxFPS;

	// ������Ʈ ������ �� ���� �������� �������
	DWORD mSupplementTime;
};


extern CFrameSkip gFrame;
