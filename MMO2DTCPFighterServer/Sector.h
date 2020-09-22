#pragma once

struct stCharacter;

struct stSectorPos
{
	int posX;
	int posY;
};

struct stSectorAround
{
	int count;
	stSectorPos around[9];
};

extern std::list<stCharacter*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void SetSectorPosition(stCharacter* pCharacter, stSectorPos* pSectorPos);

// 현재 섹터 위치를 기준으로 pCharacter를 gSector 에 추가
void AddCurCharacterSector(stCharacter* pCharacter);

// 이전 섹터 위치를 기준으로 pCharacter를 gSector 에서 삭제
void RemoveOldCharacterSector(stCharacter* pCharacter);

// stCharacter 현재 좌표 기준으로 섹터 위칠 업데이트 
// RemoveOldCharacterSector(), AddCurCharacterSector() 호출
bool UpdateSectorPosition(stCharacter* pCharacter);

// 인자로 전달된 섹터 좌표를 기준으로 pSectorAround에 주변 섹터 정보를 저장
void GetSectorAround(int sectorPosX, int sectorPosY, stSectorAround* pSectorAround);

// pCharacter의 oldSector, curSector 기준으로 지워질 Sector와 새로 Update 될 Sector를 pRemoveAroundSector, pAddAroundSector에 저장한다.
void UpdateSectorAround(stCharacter* pCharacter, stSectorAround* pRemoveAroundSector, stSectorAround* pAddAroundSector);

// 캐릭터의 섹터가 업데이트 되었다는 변경사항을 각 영향권 섹터에 위차한 캐릭터들에게 send한다.
void SendUpdateCharacterSector(stCharacter* pCharacter);