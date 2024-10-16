#pragma once

#define ASSERT
//#define DELAY_TIME_CHECK

/*********************************************
* ä�� ���� ���� ��(������ �μ�)
**********************************************/
/// @def ä�� ���� Ŭ���̾�Ʈ ��ū ��ġ ���� Ȯ��
#define CHATTING_SERVER_CONNECT_TO_REDIS				false	
/// @def ä�� ���� ���ε� IP
#define CHATTING_SERVER_IP								NULL	//"127.0.0.1"
/// @def ä�� ���� ���ε� ��Ʈ ��ȣ
#define CHATTING_SERVER_PORT							12130
/// @def ä�� ���� ���� �ִ� ���뷮
#define CHATTING_SERVER_MAX_OF_CONNECTIONS				18000
/// @def ä�� ���� �������� �ڵ� (ä�� �������� ��� �ڵ� ��)
#define CHATTING_SERVER_PROTOCOL_CODE					119
/// @def ä�� ���� ��Ŷ �ڵ� (��Ŷ En/Decode �� ���Ǵ� ����-Ŭ�� ��ĪŰ)
#define CHATTING_SERVER_PACKET_KEY						50
/// @def ä�� ���� Ŭ���̾�Ʈ ���� ���۸� ���
#define CHATTING_SERVER_RECV_BUFFERING_MODE				true
/// @def ä�� ���� IOCP 'Concurrent thread' �μ�
#define CHATTING_SERVER_IOCP_CONCURRENT_THREAD			0
/// @def ä�� ���� IOCP �۾��� ������ ����
#define CHATTING_SERVER_IOCP_WORKER_THREAD				2
/// @def TLS �޸� Ǯ(��ü: �۽� ����ȭ ����) �� �ʱ� ��ü �Ҵ� ����
#define CHATTING_SERVER_TLS_MEM_POOL_UNIT_CNT			0
/// @def TLS �޸� Ǯ( "" ) �Ҵ� ���� ��ü �ִ� ����(����)
#define CHATTING_SERVER_TLS_MEM_POOL_UNIT_CAPACITY		1000
/// @def ä�� ���� �۽� ����ȭ ���� ũ��
#define CHATTING_SERVER_SERIAL_BUFFER_SIZE				500
/// @def ä�� ���� ���� ���� ũ��
#define CHATTING_SERVER_SESSION_RECV_BUFF_SIZE			1000
/// @def TPS(Transaction Per Second) ���� �÷���
#define CHATTING_SERVER_CALC_TPS_THREAD					true


/*********************************************
* ä�� ���� ���� ���� ��
**********************************************/
/// @def �ʵ� X ����
#define dfSECTOR_X_MAX								50
/// @def �ʵ� Y ����
#define dfSECTOR_Y_MAX								50
/// @def �̱� ���μ��� ������ + �̱� �޽��� ť ����
#define PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
/// @def �̱� ���μ��� ������ + EVENT Ȱ�� ����
//#define PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT
/// @def Redis ��ū ���� IP
#define TOKEN_AUTH_REDIS_IP							"10.0.2.2"
/// @def Redis ��ū ���� ��Ʈ ��ȣ
#define TOKEN_AUTH_REDIS_PORT						6379							

/*******************************************************************************
* �α��� ����͸� Ŭ���̾�Ʈ(����͸� ���� ���� Ŭ���̾�Ʈ) ���� ��(������ �μ�)
********************************************************************************/
#define MONT_SERVER_IP							"127.0.0.1"
#define	MONT_SERVER_PORT						12121
#define MONT_SERVER_PROTOCOL_CODE				99
#define MONT_SERVER_PACKET_KEY					30
#define MONT_CLIENT_IOCP_CONCURRENT_THRD		0
#define MONT_CLIENT_IOCP_WORKER_THRD_CNT		1
#define MONT_CLIENT_MEM_POOL_UNIT_CNT			0
#define MONT_CLIENT_MEM_POOL_UNIT_CAPACITY		10
#define MONT_CLIENT_MEM_POOL_BUFF_ALLOC_SIZE	200
#define MONT_CLIENT_RECV_BUFF_SIZE				100

enum en_MONT_PACKET_TYPE
{
	//------------------------------------------------------
	// Monitor Server Protocol
	//------------------------------------------------------


	////////////////////////////////////////////////////////
	//
	//   MonitorServer & MoniterTool Protocol / ������ ���� ����.
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Monitor Server  Protocol
	//------------------------------------------------------
	en_PACKET_SS_MONITOR = 20000,
	//------------------------------------------------------
	// Server -> Monitor Protocol
	//------------------------------------------------------
	//------------------------------------------------------------
	// LoginServer, GameServer , ChatServer  �� ����͸� ������ �α��� ��
	//
	// 
	//	{
	//		WORD	Type
	//
	//		int		ServerNo		//  �� �������� ���� ��ȣ�� �ο��Ͽ� ���
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_LOGIN,

	//------------------------------------------------------------
	// ������ ����͸������� ������ ����
	// �� ������ �ڽ��� ����͸����� ��ġ�� 1�ʸ��� ����͸� ������ ����.
	//
	// ������ �ٿ� �� ��Ÿ ������ ����͸� �����Ͱ� ���޵��� ���ҋ��� ����Ͽ� TimeStamp �� �����Ѵ�.
	// �̴� ����͸� Ŭ���̾�Ʈ���� ���,�� ����Ѵ�.
	// 
	//	{
	//		WORD	Type
	//
	//		BYTE	DataType				// ����͸� ������ Type �ϴ� Define ��.
	//		int		DataValue				// �ش� ������ ��ġ.
	//		int		TimeStamp				// �ش� �����͸� ���� �ð� TIMESTAMP  (time() �Լ�)
	//										// ���� time �Լ��� time_t Ÿ�Ժ����̳� 64bit �� ���񽺷����
	//										// int �� ĳ�����Ͽ� ����. �׷��� 2038�� ������ ��밡��
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_DATA_UPDATE
};
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