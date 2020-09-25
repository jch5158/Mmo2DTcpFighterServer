#pragma once


#pragma once


#ifndef __PACKET_DEFINE__

#define __PACKET_DEFINE__

#define dfNETWORK_PACKET_CODE 0x89

#pragma pack(1)
struct stHeader
{
	//패킷코드 0x89 고정.
	BYTE byCode;

	//패킷 사이즈.
	BYTE bySize;

	//패킷타입.
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
	//(방향 디파인 값 8방향 사용)
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
// 캐릭터 데미지 패킷							Server -> Client
//
// 공격에 맞은 캐릭터의 정보를 보냄.
//
//	4	-	AttackID	( 공격자 ID )
//	4	-	DamageID	( 피해자 ID )
//	1	-	DamageHP	( 피해자 HP )
//
//---------------------------------------------------------------

struct stPacketScDamage
{
	DWORD dwAttackerID;
	DWORD victimID;
	BYTE byDamageHP;
};



// 사용안함...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------



#define	dfPACKET_SC_SYNC						251
// 서버 만들 때 사용한다.
//---------------------------------------------------------------
// 동기화를 위한 패킷					Server -> Client
//
// 서버로부터 동기화 패킷을 받으면 해당 캐릭터를 찾아서
// 캐릭터 좌표를 보정해준다.
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



#define	dfPACKET_CS_ECHO						252
//---------------------------------------------------------------
// Echo 용 패킷					Client -> Server
//
//	4	-	Time
//
//---------------------------------------------------------------

#define	dfPACKET_SC_ECHO						253
//---------------------------------------------------------------
// Echo 응답 패킷				Server -> Client
//
//	4	-	Time
//
//---------------------------------------------------------------


#pragma pack()

#endif