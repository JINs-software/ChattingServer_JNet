#pragma once

#define ASSERT
//#define DELAY_TIME_CHECK

/*********************************************
* 채팅 서버 설정 값(생성자 인수)
**********************************************/
/// @def 채팅 서버 클라이언트 토큰 일치 여부 확인
#define CHATTING_SERVER_CONNECT_TO_REDIS				false	
/// @def 채팅 서버 바인딩 IP
#define CHATTING_SERVER_IP								NULL	//"127.0.0.1"
/// @def 채팅 서버 바인딩 포트 번호
#define CHATTING_SERVER_PORT							12130
/// @def 채팅 서버 연결 최대 수용량
#define CHATTING_SERVER_MAX_OF_CONNECTIONS				18000
/// @def 채팅 서버 프로토콜 코드 (채팅 프로토콜 헤더 코드 부)
#define CHATTING_SERVER_PROTOCOL_CODE					119
/// @def 채팅 서버 패킷 코드 (패킷 En/Decode 시 사용되는 서버-클라 대칭키)
#define CHATTING_SERVER_PACKET_KEY						50
/// @def 채팅 서버 클라이언트 수신 버퍼링 모드
#define CHATTING_SERVER_RECV_BUFFERING_MODE				true
/// @def 채팅 서버 IOCP 'Concurrent thread' 인수
#define CHATTING_SERVER_IOCP_CONCURRENT_THREAD			0
/// @def 채팅 서버 IOCP 작업자 스레드 갯수
#define CHATTING_SERVER_IOCP_WORKER_THREAD				2
/// @def TLS 메모리 풀(객체: 송신 직렬화 버퍼) 내 초기 객체 할당 갯수
#define CHATTING_SERVER_TLS_MEM_POOL_UNIT_CNT			0
/// @def TLS 메모리 풀( "" ) 할당 가능 객체 최대 갯수(제한)
#define CHATTING_SERVER_TLS_MEM_POOL_UNIT_CAPACITY		1000
/// @def 채팅 서버 송신 직렬화 버퍼 크기
#define CHATTING_SERVER_SERIAL_BUFFER_SIZE				500
/// @def 채팅 세션 수신 버퍼 크기
#define CHATTING_SERVER_SESSION_RECV_BUFF_SIZE			1000
/// @def TPS(Transaction Per Second) 측정 플래그
#define CHATTING_SERVER_CALC_TPS_THREAD					true


/*********************************************
* 채팅 서버 내부 설정 값
**********************************************/
/// @def 필드 X 범위
#define dfSECTOR_X_MAX								50
/// @def 필드 Y 범위
#define dfSECTOR_Y_MAX								50
/// @def 싱글 프로세싱 스레드 + 싱글 메시지 큐 구조
#define PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
/// @def 싱글 프로세싱 스레드 + EVENT 활용 구조
//#define PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT
/// @def Redis 토큰 서버 IP
#define TOKEN_AUTH_REDIS_IP							"10.0.2.2"
/// @def Redis 토큰 서버 포트 번호
#define TOKEN_AUTH_REDIS_PORT						6379							

/*******************************************************************************
* 로그인 모니터링 클라이언트(모니터링 서버 기준 클라이언트) 설정 값(생성자 인수)
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
	//   MonitorServer & MoniterTool Protocol / 응답을 받지 않음.
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
	// LoginServer, GameServer , ChatServer  가 모니터링 서버에 로그인 함
	//
	// 
	//	{
	//		WORD	Type
	//
	//		int		ServerNo		//  각 서버마다 고유 번호를 부여하여 사용
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_LOGIN,

	//------------------------------------------------------------
	// 서버가 모니터링서버로 데이터 전송
	// 각 서버는 자신이 모니터링중인 수치를 1초마다 모니터링 서버로 전송.
	//
	// 서버의 다운 및 기타 이유로 모니터링 데이터가 전달되지 못할떄를 대비하여 TimeStamp 를 전달한다.
	// 이는 모니터링 클라이언트에서 계산,비교 사용한다.
	// 
	//	{
	//		WORD	Type
	//
	//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
	//		int		DataValue				// 해당 데이터 수치.
	//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
	//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
	//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_DATA_UPDATE
};
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