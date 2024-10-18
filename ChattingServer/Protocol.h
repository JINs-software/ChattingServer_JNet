#pragma once

#include <minwindef.h>
#include "Configuration.h"

#if defined(MOW_CHAT_SERVER_MODE)
#define TOKEN_LENGTH					20
#define MAX_OF_CHAT_LENGTH				100

enum enPacketType {
	CHAT_PACKET_TYPE = 15000,
	REQ_LOGIN,
	REQ_ENTER_MATCH,
	REQ_LEAVE_MATCH,
	REPLY_CODE,
	SEND_CHAT_MSG,
	RECV_CHAT_MSG,

	en_PACKET_SS_MONITOR = 20000,
	en_PACKET_SS_MONITOR_LOGIN,
	en_PACKET_SS_MONITOR_DATA_UPDATE,
	en_PACKET_CS_MONITOR = 25000,
	en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN,
	en_PACKET_CS_MONITOR_TOOL_RES_LOGIN,
	en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE,

	en_SESSION_JOIN = 30000,
	en_SESSION_RELEASE,
};

enum enReplyCode {
	LOGIN_SUCCESS,
	LOGIN_FAIL,
};

#pragma pack(push, 1)
struct MSG_REQ_LOGIN {
	UINT16	Type;
	UINT16	AccountNo;
	WCHAR	Token[TOKEN_LENGTH];
	INT32	TokenLength;
};

struct MSG_REQ_ENTER_MATCH {
	UINT16	Type;
	UINT16	RoomID;
};

struct MSG_REQ_LEAVE_MATCH {
	UINT16	Type;
};

struct MSG_REPLY_CODE {
	UINT16	Type;
	UINT16	ReplyCode;
};

struct MSG_SEND_CHAT_MSG {
	UINT16	Type;
	WCHAR	ChatMsg[MAX_OF_CHAT_LENGTH];
	INT32	Length;
};

struct MSG_RECV_CHAT_MSG {
	UINT16	Type;
	WCHAR	ChatMsg[MAX_OF_CHAT_LENGTH];
	INT32	Length;
	UINT16	AccountNo;
};
#pragma pack(pop)

#else

enum en_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & Server Protocol
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Chatting Server
	//------------------------------------------------------
	en_PACKET_CS_CHAT_SERVER = 0,

	//------------------------------------------------------------
	// ä�ü��� �α��� ��û
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]				// null ����
	//		WCHAR	Nickname[20]		// null ����
	//		char	SessionKey[64];		// ������ū
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_LOGIN,

	//------------------------------------------------------------
	// ä�ü��� �α��� ����
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status				// 0:����	1:����
	//		INT64	AccountNo
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_LOGIN,

	//------------------------------------------------------------
	// ä�ü��� ���� �̵� ��û
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_SECTOR_MOVE,

	//------------------------------------------------------------
	// ä�ü��� ���� �̵� ���
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_SECTOR_MOVE,

	//------------------------------------------------------------
	// ä�ü��� ä�ú����� ��û
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null ������
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_MESSAGE,

	//------------------------------------------------------------
	// ä�ü��� ä�ú����� ����  (�ٸ� Ŭ�� ���� ä�õ� �̰ɷ� ����)
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]						// null ����
	//		WCHAR	Nickname[20]				// null ����
	//		
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null ������
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_MESSAGE,

	//------------------------------------------------------------
	// ��Ʈ��Ʈ
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// Ŭ���̾�Ʈ�� �̸� 30�ʸ��� ������.
	// ������ 40�� �̻󵿾� �޽��� ������ ���� Ŭ���̾�Ʈ�� ������ ������� ��.
	//------------------------------------------------------------	
	en_PACKET_CS_CHAT_REQ_HEARTBEAT,

	en_SESSION_JOIN,
	en_SESSION_RELEASE,
	//en_SESSION_RELEASE_BEFORE_LOGIN
};

enum en_Login_Status 
{
	FAIL,
	SUCCESS
};

#pragma pack(push, 1)
struct MSG_PACKET_CS_CHAT_REQ_LOGIN {
	WORD Type;
	INT64 AccountNo;
	WCHAR ID[20];			// null ����
	WCHAR Nickname[20];		// null ����
	char sessionKey[64];		// ���� ��ū
};
struct MSG_PACKET_CS_CHAT_RES_LOGIN {
	WORD	Type;
	BYTE	Status;				// 0:����	1:����
	INT64	AccountNo;
};
struct MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE {
	WORD	Type;
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
};
struct MSG_PACKET_CS_CHAT_RES_SECTOR_MOVE {
	WORD	Type;
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
};
struct MSG_PACKET_CS_CHAT_REQ_MESSAGE {
	WORD	Type;
	INT64	AccountNo;
	WORD	MessageLen;

	//WCHAR	Message[MessageLen / 2];		// null ������
	//WCHAR* Message;
};
struct MSG_PACKET_CS_CHAT_RES_MESSAGE {
	WORD	Type;
	INT64	AccountNo;
	WCHAR	ID[20];						// null ����
	WCHAR	Nickname[20];				// null ����

	WORD	MessageLen;

	//WCHAR	Message[MessageLen / 2];		// null ������
	//WCHAR* Message;
};
struct MSG_PACKET_CS_CHAT_REQ_HEARTBEAT {
	WORD		Type;
};

#pragma pack(pop)
#endif

#pragma pack(push, 1)
struct stMSG_MONITOR_DATA_UPDATE {
	WORD	Type;
	BYTE	DataType;
	int		DataValue;
	int		TimeStamp;

};
#pragma pack(pop)

enum en_PACKET_SS_MONITOR_DATA_UPDATE {
	dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN = 30,		// ä�ü��� ChatServer ���� ���� ON / OFF
	dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU = 31,		// ä�ü��� ChatServer CPU ����
	dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM = 32,		// ä�ü��� ChatServer �޸� ��� MByte
	dfMONITOR_DATA_TYPE_CHAT_SESSION = 33,			// ä�ü��� ���� �� (���ؼ� ��)
	dfMONITOR_DATA_TYPE_CHAT_PLAYER = 34,			// ä�ü��� �������� ����� �� (���� ������)
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS = 35,		// ä�ü��� UPDATE ������ �ʴ� �ʸ� Ƚ��
	dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL = 36,		// ä�ü��� ��ŶǮ ��뷮
	dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL = 37,	// ä�ü��� UPDATE MSG Ǯ ��뷮
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_WORKER_CPU = 38,	// ä�ü��� UPDATE MSG Ǯ ��뷮
};
enum en_SERVER_TYPE {
	dfSERVER_LOGIN_SERVER = 0,
	dfSERVER_ECHO_GAME_SERVER,
	dfSERVER_CHAT_SERVER,
	dfSERVER_SYSTEM
};