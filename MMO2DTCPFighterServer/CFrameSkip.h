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

	// ������Ʈ ������ �� ���� �������� �������
	DWORD mSupplementTime;

};


extern CFrameSkip gFrame;
