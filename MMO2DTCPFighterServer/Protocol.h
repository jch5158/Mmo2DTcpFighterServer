#pragma once


#pragma once


#ifndef __PACKET_DEFINE__

#define __PACKET_DEFINE__

#define dfNETWORK_PACKET_CODE 0x89

#pragma pack(1)
struct stHeader
{
	//��Ŷ�ڵ� 0x89 ����.
	BYTE byCode;

	//��Ŷ ������.
	BYTE bySize;

	//��ŶŸ��.
	BYTE byType;
};


#define	dfPACKET_SC_CREATE_MY_CHARACTER			0

struct stPacketCreateMyCharacter
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
	BYTE byHP;

};


#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
struct stPacketCreateOtherCharacter
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
	BYTE byHP;
};



#define	dfPACKET_SC_DELETE_CHARACTER			2
struct stPacketDeleteCharacter
{
	DWORD dwID;
};



#define	dfPACKET_CS_MOVE_START					10
struct stPacketCsMoveStart
{
	//(���� ������ �� 8���� ���)
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};


#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7




#define	dfPACKET_SC_MOVE_START					11
struct stPacketScMoveStart
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};



#define	dfPACKET_CS_MOVE_STOP					12
struct stPacketCsMoveStop
{
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};



#define	dfPACKET_SC_MOVE_STOP					13

struct stPacketScMoveStop
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};



#define	dfPACKET_CS_ATTACK1						20

struct stPacketCsAttack1
{
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};




#define	dfPACKET_SC_ATTACK1						21

struct stPacketScAttack1
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};



#define	dfPACKET_CS_ATTACK2						22

struct stPacketCsAttack2
{
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};


#define	dfPACKET_SC_ATTACK2						23
struct stPacketScAttack2
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};


#define	dfPACKET_CS_ATTACK3						24

struct stPacketCsAttack3
{
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};


#define	dfPACKET_SC_ATTACK3						25

struct stPacketScAttack3
{
	DWORD dwID;
	BYTE byDirection;
	unsigned short usX;
	unsigned short usY;
};




#define	dfPACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// ĳ���� ������ ��Ŷ							Server -> Client
//
// ���ݿ� ���� ĳ������ ������ ����.
//
//	4	-	AttackID	( ������ ID )
//	4	-	DamageID	( ������ ID )
//	1	-	DamageHP	( ������ HP )
//
//---------------------------------------------------------------

struct stPacketScDamage
{
	DWORD dwAttackerID;
	DWORD victimID;
	BYTE byDamageHP;
};



// ������...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------



#define	dfPACKET_SC_SYNC						251
// ���� ���� �� ����Ѵ�.
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Server -> Client
//
// �����κ��� ����ȭ ��Ŷ�� ������ �ش� ĳ���͸� ã�Ƽ�
// ĳ���� ��ǥ�� �������ش�.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct stPacketScSync
{
	DWORD dwID;
	unsigned short usX;
	unsigned short usY;
};

#pragma pack()

#endif