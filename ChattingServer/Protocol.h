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
	// 채팅서버 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]				// null 포함
	//		WCHAR	Nickname[20]		// null 포함
	//		char	SessionKey[64];		// 인증토큰
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_LOGIN,

	//------------------------------------------------------------
	// 채팅서버 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status				// 0:실패	1:성공
	//		INT64	AccountNo
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_LOGIN,

	//------------------------------------------------------------
	// 채팅서버 섹터 이동 요청
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
	// 채팅서버 섹터 이동 결과
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
	// 채팅서버 채팅보내기 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_MESSAGE,

	//------------------------------------------------------------
	// 채팅서버 채팅보내기 응답  (다른 클라가 보낸 채팅도 이걸로 받음)
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]						// null 포함
	//		WCHAR	Nickname[20]				// null 포함
	//		
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_MESSAGE,

	//------------------------------------------------------------
	// 하트비트
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// 클라이언트는 이를 30초마다 보내줌.
	// 서버는 40초 이상동안 메시지 수신이 없는 클라이언트를 강제로 끊어줘야 함.
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
	WCHAR ID[20];			// null 포함
	WCHAR Nickname[20];		// null 포함
	char sessionKey[64];		// 인증 토큰
};
struct MSG_PACKET_CS_CHAT_RES_LOGIN {
	WORD	Type;
	BYTE	Status;				// 0:실패	1:성공
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

	//WCHAR	Message[MessageLen / 2];		// null 미포함
	//WCHAR* Message;
};
struct MSG_PACKET_CS_CHAT_RES_MESSAGE {
	WORD	Type;
	INT64	AccountNo;
	WCHAR	ID[20];						// null 포함
	WCHAR	Nickname[20];				// null 포함

	WORD	MessageLen;

	//WCHAR	Message[MessageLen / 2];		// null 미포함
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
	dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN = 30,		// 채팅서버 ChatServer 실행 여부 ON / OFF
	dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU = 31,		// 채팅서버 ChatServer CPU 사용률
	dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM = 32,		// 채팅서버 ChatServer 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_CHAT_SESSION = 33,			// 채팅서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_CHAT_PLAYER = 34,			// 채팅서버 인증성공 사용자 수 (실제 접속자)
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS = 35,		// 채팅서버 UPDATE 스레드 초당 초리 횟수
	dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL = 36,		// 채팅서버 패킷풀 사용량
	dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL = 37,	// 채팅서버 UPDATE MSG 풀 사용량
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_WORKER_CPU = 38,	// 채팅서버 UPDATE MSG 풀 사용량
};
enum en_SERVER_TYPE {
	dfSERVER_LOGIN_SERVER = 0,
	dfSERVER_ECHO_GAME_SERVER,
	dfSERVER_CHAT_SERVER,
	dfSERVER_SYSTEM
};