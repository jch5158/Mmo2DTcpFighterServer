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

void RemoveSectorPosition(stCharacter* pCharacter);

// ���� ���� ��ġ�� �������� pCharacter�� gSector �� �߰�
void AddCurCharacterSector(stCharacter* pCharacter);

// ���� ���� ��ġ�� �������� pCharacter�� gSector ���� ����
void RemoveOldCharacterSector(stCharacter* pCharacter);

// stCharacter ���� ��ǥ �������� ���� ��ĥ ������Ʈ 
// RemoveOldCharacterSector(), AddCurCharacterSector() ȣ��
bool UpdateSectorPosition(stCharacter* pCharacter);

// ���ڷ� ���޵� ���� ��ǥ�� �������� pSectorAround�� �ֺ� ���� ������ ����
void GetSectorAround(int sectorPosX, int sectorPosY, stSectorAround* pSectorAround);

// pCharacter�� oldSector, curSector �������� ������ Sector�� ���� Update �� Sector�� pRemoveAroundSector, pAddAroundSector�� �����Ѵ�.
void UpdateSectorAround(stCharacter* pCharacter, stSectorAround* pRemoveAroundSector, stSectorAround* pAddAroundSector);

// ĳ������ ���Ͱ� ������Ʈ �Ǿ��ٴ� ��������� �� ����� ���Ϳ� ������ ĳ���͵鿡�� send�Ѵ�.
void SendUpdateCharacterSector(stCharacter* pCharacter);