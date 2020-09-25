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

struct stAttackSectorAround
{
	int count;
	stSectorPos around[16];
};

extern std::list<stCharacter*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

// pCharacter�� ��ǥ�� ���߾ pSectorPos ��������� �ʱ�ȭ�Ѵ�.
void SetSectorPosition(stCharacter* pCharacter, stSectorPos* pSectorPos);

// pCharacter�� curSector ��ǥ�� gSector list���� pCharacter�� Remove�Ѵ�.
void RemoveSectorPosition(stCharacter* pCharacter);

// ���� ���� ��ġ�� �������� pCharacter�� gSector �� �߰�
void AddCurCharacterSector(stCharacter* pCharacter);

// ���� ���� ��ġ�� �������� pCharacter�� gSector ���� ����
void RemoveOldCharacterSector(stCharacter* pCharacter);

// stCharacter ���� ��ǥ �������� ���� ��ĥ ������Ʈ 
// RemoveOldCharacterSector(), AddCurCharacterSector() ȣ��
// curSector ���� ����Ǿ��ٸ� true�� return�Ѵ�.
bool UpdateSectorPosition(stCharacter* pCharacter);



// ���ڷ� ���޵� ���� ��ǥ�� �������� pSectorAround�� �ֺ� ���� ������ pSectorAround�� �����Ѵ�.
// ��ǥ�� ���޵� ������ curSector�� ���ڷ� ���� �� ���� �ְ� oldSector�� ���ڷ� ������ ���� �ֱ� �����̴�. 
void GetSectorAround(int sectorPosX, int sectorPosY, stSectorAround* pSectorAround);


void GetAttackSectorAround(stCharacter* pCharacter, stAttackSectorAround* pAttackSectorAround);


// pCharacter�� oldSector, curSector �������� ������ Sector�� ���� Update �� Sector�� pRemoveAroundSector, pAddAroundSector�� �����Ѵ�.
void UpdateSectorAround(stCharacter* pCharacter, stSectorAround* pRemoveAroundSector, stSectorAround* pAddAroundSector);


// ĳ������ ���Ͱ� ������Ʈ �Ǿ��ٴ� ��������� �� ����� ���Ϳ� ������ ĳ���͵鿡�� send�Ѵ�.
void SendUpdateCharacterSector(stCharacter* pCharacter);