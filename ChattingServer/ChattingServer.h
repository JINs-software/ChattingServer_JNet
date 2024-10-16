#pragma once
#include "JNetCore.h"
#include "Account.h"
#include "PerformanceCounter.h"
#include "Protocol.h"
#include "Configuration.h"

using namespace jnet;
class ChattingServerMont;
namespace RedisCpp {
	class CRedisConn;
}

class ChattingServer : public JNetServer
{
private:
	uint32										m_NumOfIocpWorkers;
	bool										m_ConnToRedis;
	bool										m_StopFlag;
	int											m_UpdateThreadTransaction;

	std::set<SessionID64>						m_LoginWaitSessions;
	AccountObjectPool*							m_AccountPool;
	std::unordered_map<SessionID64, stAccoutInfo*>	m_SessionIdAccountMap;

	LockFreeQueue<RedisCpp::CRedisConn*>		m_RedisConnPool;

	HANDLE										m_ProcessThreadHnd;

	ChattingServerMont*							m_ChatServerMont;

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	std::mutex									m_LoginWaitSetMtx;	// insert: accept thread, erase: iocp workers

	struct stRecvInfo {
		uint64	sessionID;
		long	recvMsgCnt;
	};
	DWORD													m_RecvEventTlsIndex;
	HANDLE													m_WorkersRecvEvents[CHATTING_SERVER_IOCP_WORKER_THREAD];
	std::map<DWORD, HANDLE>									m_ThrdEventHndMap;
	std::unordered_map<HANDLE, LockFreeQueue<stRecvInfo>>	m_ThrdEventRecvInfoMap;
	SRWLOCK													m_SessionMessageQueueLock;
	std::unordered_map<uint64, LockFreeQueue<JBuffer*>>		m_SessionMessageQueue;
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
	struct SessionTimeStamp {
		UINT64	sessionID;
		clock_t timeStamp;
	};
	LockFreeQueue<std::pair<SessionTimeStamp, JBuffer*>>				m_MessageLockFreeQueue;
#endif

#if defined(MOW_CHAT_SERVER_MODE)
	std::set<SessionID64>									m_LobbyMap;
	std::unordered_map<SessionID64, uint16>					m_SessionMatchMap;
	std::unordered_map<uint16, std::set<SessionID64>>		m_MatchRoomMap;
#else
	std::map<uint64, stAccoutInfo*> m_SectorMap[dfSECTOR_Y_MAX + 1][dfSECTOR_X_MAX + 1];
#endif

public: 
	ChattingServer(
		bool	connToRedis,
		const char*	serverIP,	uint16	serverPort,	uint16	maximumOfConnections,
		BYTE	packetCode_LAN,	BYTE	packetCode,	BYTE	packetSymmetricKey,
		bool	recvBufferingMode,
		uint16	maximumOfSessions,
		uint32	numOfIocpConcurrentThrd,	uint16	numOfIocpWorkerThrd,
		size_t	tlsMemPoolUnitCnt,	size_t	tlsMemPoolUnitCapacity,
		uint32	memPoolBuffAllocSize,
		uint32	sessionRecvBuffSize,
		bool	calcTpsThread
	) : JNetServer(
		serverIP, serverPort, maximumOfConnections,
		packetCode_LAN, packetCode, packetSymmetricKey,
		recvBufferingMode,
		maximumOfSessions,
		numOfIocpConcurrentThrd, numOfIocpWorkerThrd,
		tlsMemPoolUnitCnt, tlsMemPoolUnitCapacity,
		memPoolBuffAllocSize,
		sessionRecvBuffSize,
		calcTpsThread
	),
		m_ConnToRedis(connToRedis), m_NumOfIocpWorkers(numOfIocpWorkerThrd), m_StopFlag(false), m_UpdateThreadTransaction(0)
	{}

public:
	/////////////////////////////////////////////////////////////////////////////////////
	// ChattingServer::Start
	// - 계정 객체 풀 생성, CHAT_SERV_LIMIT_ACCEPTANCE: 최대 수용량
	// - 모니터링 카운터 생성
	/////////////////////////////////////////////////////////////////////////////////////
	bool Start();
	/////////////////////////////////////////////////////////////////////////////////////
	// ChattingServer::Stop
	// - 계정 객체 풀 생성, CHAT_SERV_LIMIT_ACCEPTANCE: 최대 수용량
	// - 모니터링 카운터 생성
	/////////////////////////////////////////////////////////////////////////////////////
	void Stop();

	bool ServerStop() { return m_StopFlag; }
	inline int GetUpdateThreadTransaction() { return m_UpdateThreadTransaction; m_UpdateThreadTransaction = 0;}
	inline int GetUpdateMessageQueueSize() {
#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
		return 0;
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
		return m_MessageLockFreeQueue.GetSize(); 
#endif
	}
	inline int GetPlayerCount() { return m_SessionIdAccountMap.size(); }

private:
	virtual bool OnWorkerThreadCreate(HANDLE thHnd) override;
	virtual void OnAllWorkerThreadCreate() override;	// -> 프로세싱(업데이트) 스레드 생성
	// -> 모니터링 연동 및 카운팅 스레드 생성

	virtual void OnWorkerThreadStart() override;
	virtual void OnClientJoin(UINT64 sessionID, const SOCKADDR_IN& clientSockAddr) override;	// -> 세션 접속 메시지 업데이트 메시지 큐에 큐잉
	virtual void OnClientLeave(UINT64 sessionID) override;	// -> 세션 해제 메시지 업데이트 메시지 큐에 큐잉 
	virtual void OnRecv(UINT64 sessionID, JBuffer& recvBuff) override;
	virtual void OnRecv(UINT64 sessionID, JSerialBuffer& recvBuff) override;
	virtual void OnPrintLogOnConsole() override;
private:
#if defined(MOW_CHAT_SERVER_MODE)
	void ProcessMessage(uint64 sessionID, JBuffer* msg);

	void Proc_REQ_LOGIN(SessionID64 sessionID, const MSG_REQ_LOGIN& body);
	void Proc_REQ_ENTER_MATCH(SessionID64 sessionID, const MSG_REQ_ENTER_MATCH& body);
	void Proc_REQ_LEAVE_MATCH(SessionID64 sessionID, const MSG_REQ_LEAVE_MATCH& body);
	void Proc_SEND_CHAT_MSG(SessionID64 sessionID, const MSG_SEND_CHAT_MSG& body);

	void Send_REPLY_CODE(SessionID64 sessionID, uint16 replyCode);
	//void Send_CHAT_MSG(SessionID64 sessionID, const MSG_SEND_CHAT_MSG& body);
#else
	void ProcessMessage(uint64 sessionID, JBuffer* msg);

	void Proc_REQ_LOGIN(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_LOGIN& body);
	void Send_RES_LOGIN(UINT64 sessionID, BYTE STATUS, INT64 AccountNo);
	void Proc_REQ_SECTOR_MOVE(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE& body);
	void Send_RES_SECTOR_MOVE(UINT64 sessionID, INT64 AccountNo, WORD SectorX, WORD SectorY);
	void Proc_REQ_MESSAGE(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_MESSAGE& body, BYTE* message);
#endif
	void Proc_SessionJoin(UINT64 sessionID);
	void Proc_SessionRelease(UINT64 sessionID);
	//void Proc_REQ_HEARTBEAT();

private:
	static UINT __stdcall ProcessThreadFunc(void* arg);
};

