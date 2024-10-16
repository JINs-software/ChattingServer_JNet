#include "ChattingServer.h"
#include "ChattingServerMont.h"
#include "CRedisConn.h"

using namespace std;

bool ChattingServer::Start()
{
	if (m_ConnToRedis) {
		// Connect to Redis
		bool firstConn = true;
		for (uint16 i = 0; i < m_NumOfIocpWorkers; i++) {
			RedisCpp::CRedisConn* redisConn = new RedisCpp::CRedisConn();	// 형식 지정자가 필요합니다.
			if (redisConn == NULL) {
				cout << "[LoginServer::Start] new RedisCpp::CRedisConn() return NULL" << endl;
				return false;
			}

			if (!redisConn->connect(TOKEN_AUTH_REDIS_IP, TOKEN_AUTH_REDIS_PORT)) {
				cout << "[ChattingServer::Start] new RedisCpp::CRedisConn(); return NULL" << endl;
				return false;
			}

			if (!redisConn->ping()) {
				cout << "[ChattingServer::Start] m_RedisConn->connect(..) return FALSE" << endl;
				return false;
			}

			m_RedisConnPool.Enqueue(redisConn);
		}
	}

	m_AccountPool = new AccountObjectPool(CHATTING_SERVER_MAX_OF_CONNECTIONS);

	m_ChatServerMont = new ChattingServerMont(this,
		MONT_SERVER_IP, MONT_SERVER_PORT,
		MONT_SERVER_PROTOCOL_CODE,
		MONT_CLIENT_IOCP_CONCURRENT_THRD, MONT_CLIENT_IOCP_WORKER_THRD_CNT,
		MONT_CLIENT_MEM_POOL_UNIT_CNT, MONT_CLIENT_MEM_POOL_UNIT_CAPACITY,
		MONT_CLIENT_MEM_POOL_BUFF_ALLOC_SIZE,
		MONT_CLIENT_RECV_BUFF_SIZE
		);
	
	return JNetServer::Start();
}

void ChattingServer::Stop()
{
	m_StopFlag = true;

	if (m_ConnToRedis) {
		while (m_RedisConnPool.GetSize() > 0) {
			RedisCpp::CRedisConn* redisConn = NULL;
			m_RedisConnPool.Dequeue(redisConn);
			if (redisConn != NULL) {
				delete redisConn;
			}
		}
	}

	JNetServer::Stop();
}

bool ChattingServer::OnWorkerThreadCreate(HANDLE thHnd)
{
#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	static uint16 eventIdx = 0;
	HANDLE recvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_WorkersRecvEvents[eventIdx++] = recvEvent;				// m_WorkersRecvEvents, 채팅 메시지 프로세싱 스레드가 참조
	DWORD thID = GetThreadId(thHnd);
	m_ThrdEventHndMap.insert({ thID, recvEvent });			// m_ThrdEventHndMap, IOCP 작업자 스레드의 OnWorkerThreadStart 호출 내
																// 스레드 자신의 id를 바탕으로 이벤트 객체 핸들을 가져와 TLS에 삽입할 때 참조
	return true;
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
	return true;
#endif
}

void ChattingServer::OnAllWorkerThreadCreate()
{
	// processing(update) thread
	m_ProcessThreadHnd = (HANDLE)_beginthreadex(NULL, 0, ProcessThreadFunc, this, 0, NULL);
}

void ChattingServer::OnWorkerThreadStart()
{
#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	if (TlsGetValue(m_RecvEventTlsIndex) == NULL) {
		DWORD thID = GetThreadId(GetCurrentThread());
		HANDLE thEventHnd = m_ThrdEventHndMap[thID];
		TlsSetValue(m_RecvEventTlsIndex, thEventHnd);					
		m_ThrdEventRecvInfoMap.insert({ thEventHnd, LockFreeQueue<stRecvInfo>() });
		// => IOCP 작업자 스레드는 자신의 TLS에 미리 생성된 이벤트 객체 핸들(IOCP 작업자 별 미리 생성된)을 TLS에 저장
	}
#if defined(ASSERT)
	else {
		DebugBreak();
	}
#endif
#endif
}

void ChattingServer::OnClientJoin(UINT64 sessionID, const SOCKADDR_IN& clientSockAddr)
{
#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	AcquireSRWLockExclusive(&m_SessionMessageQueueLock);
	m_SessionMessageQueue.insert({ sessionID, LockFreeQueue<JBuffer*>() });
	ReleaseSRWLockExclusive(&m_SessionMessageQueueLock);

#if defined(ASSERT)
	if (m_LoginWaitSessions.find(sessionID) != m_LoginWaitSessions.end()) DebugBreak();
#endif
	m_LoginWaitSessions.insert(sessionID);

#else
	JBuffer* msg = AllocSerialBuff();
	*msg << (WORD)en_SESSION_JOIN;
	m_MessageLockFreeQueue.Enqueue({ {sessionID, clock()}, msg });
#endif
}

void ChattingServer::OnClientLeave(UINT64 sessionID)
{
	JBuffer* msg = AllocSerialBuff();
	*msg << (WORD)en_SESSION_RELEASE;

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	AcquireSRWLockShared(&m_SessionMessageQueueLock);
	auto iter = m_SessionMessageQueue.find(sessionID);
	if (iter != m_SessionMessageQueue.end()) {
		iter->second.Enqueue(msg);
	}
	else {
#if defined(ASSERT)
		DebugBreak();
#endif
		FreeSerialBuff(msg);
		return;
	}
	ReleaseSRWLockShared(&m_SessionMessageQueueLock);

	HANDLE recvEvent = (HANDLE)TlsGetValue(m_RecvEventTlsIndex);
	m_ThrdEventRecvInfoMap[recvEvent].Enqueue(stRecvInfo{ sessionID, 1 });
	SetEvent(recvEvent);
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
	m_MessageLockFreeQueue.Enqueue({ {sessionID, clock()}, msg });
#endif
}

void ChattingServer::OnRecv(UINT64 sessionID, JBuffer& recvBuff)
{
	WORD type;
	recvBuff.Peek(&type);

	JBuffer* message = AllocSerialBuff();

	switch (type)
	{
	case en_PACKET_CS_CHAT_REQ_LOGIN:
	{
		recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_LOGIN));
		message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_LOGIN));
	}
	break;
	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
	{
		recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE));
		message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE));
	}
	break;
	case en_PACKET_CS_CHAT_REQ_MESSAGE:
	{
		WORD messageLen;
		recvBuff.Peek(sizeof(WORD) + sizeof(INT64), (BYTE*)&messageLen, sizeof(WORD));
		recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_MESSAGE) + messageLen);
		message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_MESSAGE) + messageLen);
	}
	break;
#if defined(ASSERT)
	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
	{
		DebugBreak();
	}
	break;
#endif
	default:
#if defined(ASSERT)
		DebugBreak();
#endif
		break;
	}

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	// enqueue msg to session's msg queue
	bool enqFlag = false;
	AcquireSRWLockShared(&m_SessionMessageQueueLock);
	auto iter = m_SessionMessageQueue.find(sessionID);
	if (iter != m_SessionMessageQueue.end()) {
		iter->second.Enqueue(message);
		enqFlag = true;
	}
	else {
#if defined(ASSERT)
		DebugBreak();
#endif
		FreeSerialBuff(message);
	}
	ReleaseSRWLockShared(&m_SessionMessageQueueLock);

	if (enqFlag) {
		HANDLE recvEvent = (HANDLE)TlsGetValue(m_RecvEventTlsIndex);
		m_ThrdEventRecvInfoMap[recvEvent].Enqueue(stRecvInfo{ sessionID, 1 });
		SetEvent(recvEvent);
	}
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
	m_MessageLockFreeQueue.Enqueue({ {sessionID, clock() }, message });
#endif
}

void ChattingServer::OnRecv(UINT64 sessionID, JSerialBuffer& recvBuff)
{
	LONG recvMsgCnt = 0;

	while (recvBuff.GetUseSize() > sizeof(WORD)) {
		WORD type;
		recvBuff.Peek((BYTE*)&type, sizeof(type));

		JBuffer* message = AllocSerialBuff();
		switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN:
		{
			recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_LOGIN));
			message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_LOGIN));
		}
		break;
		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		{
			recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE));
			message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE));
		}
		break;
		case en_PACKET_CS_CHAT_REQ_MESSAGE:
		{
			MSG_PACKET_CS_CHAT_REQ_MESSAGE reqMsg;
			recvBuff.Peek((BYTE*)&reqMsg, sizeof(reqMsg));
			recvBuff.Dequeue(message->GetDequeueBufferPtr(), sizeof(MSG_PACKET_CS_CHAT_REQ_MESSAGE) + reqMsg.MessageLen);
			message->DirectMoveEnqueueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_MESSAGE) + reqMsg.MessageLen);
		}
		break;
#if defined(ASSERT)
		case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		{
			DebugBreak();
		}
		break;
#endif
		default:
#if defined(ASSERT)
			DebugBreak();
#endif
			break;
		}

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
		AcquireSRWLockShared(&m_SessionMessageQueueLock);
		auto iter = m_SessionMessageQueue.find(sessionID);
		if (iter != m_SessionMessageQueue.end()) {
			iter->second.Enqueue(message);
			recvMsgCnt++;
		}
		else {
			FreeSerialBuff(message);
		}
		ReleaseSRWLockShared(&m_SessionMessageQueueLock);
#else
		m_MessageLockFreeQueue.Enqueue({ {sessionID, clock()}, message });
#endif
	}

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	if (recvMsgCnt > 0) {
		HANDLE recvEvent = (HANDLE)TlsGetValue(m_RecvEventTlsIndex);
		m_ThrdEventRecvInfoMap[recvEvent].Enqueue(stRecvInfo{ sessionID, recvMsgCnt });
		SetEvent(recvEvent);
	}
#endif

}

void ChattingServer::OnPrintLogOnConsole()
{
	cout << "<Chatting Server>" << std::endl;
	cout << "[Token Auth] Token Auth Wait Session    : " << m_LoginWaitSessions.size() << std::endl;
	cout << "[Token Auth] Token Auth Success Session : " << m_SessionIdAccountMap.size() << std::endl;
	cout << "[Account] Number of Allocated Account   : " << m_AccountPool->GetAllocatedObjectCnt() << std::endl;
}

void ChattingServer::ProcessMessage(uint64 sessionID, JBuffer* msg)
{
	WORD type;
	msg->Peek(&type);

	switch (type)
	{
	case en_SESSION_JOIN:
		Proc_SessionJoin(sessionID);
		break;
	case en_SESSION_RELEASE:
		Proc_SessionRelease(sessionID);
		break;
	case en_PACKET_CS_CHAT_REQ_LOGIN:
	{
		//MSG_PACKET_CS_CHAT_REQ_LOGIN loginReqMsg;
		//(*msg) >> loginReqMsg;	// 복사 없이 내부 버퍼를 그대로 캐스팅하는 방법은??
		//Proc_REQ_LOGIN(sessionID, loginReqMsg);
		// =>복사 생략
		MSG_PACKET_CS_CHAT_REQ_LOGIN* loginReqMsg = (MSG_PACKET_CS_CHAT_REQ_LOGIN*)msg->GetDequeueBufferPtr();
		Proc_REQ_LOGIN(sessionID, *loginReqMsg);
	}
	break;
	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
	{
		MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE* moveReqMsg = (MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE*)msg->GetDequeueBufferPtr();
		Proc_REQ_SECTOR_MOVE(sessionID, *moveReqMsg);
	}
	break;
	case en_PACKET_CS_CHAT_REQ_MESSAGE:
	{
		MSG_PACKET_CS_CHAT_REQ_MESSAGE* chatReqMsg = (MSG_PACKET_CS_CHAT_REQ_MESSAGE*)msg->GetDequeueBufferPtr();
		msg->DirectMoveDequeueOffset(sizeof(MSG_PACKET_CS_CHAT_REQ_MESSAGE));
		BYTE* chatMsg = msg->GetDequeueBufferPtr();
		Proc_REQ_MESSAGE(sessionID, *chatReqMsg, chatMsg);
	}
	break;
	//case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
	//{..} break;
	default:
		break;
	}

	//delete msg;
	FreeSerialBuff(msg);
	m_UpdateThreadTransaction++;
}

void ChattingServer::Proc_REQ_LOGIN(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_LOGIN& body)
{
	bool releaseBeforeLogin = false;
	{
		if (m_LoginWaitSessions.find(sessionID) != m_LoginWaitSessions.end()) {
			m_LoginWaitSessions.erase(sessionID);
		}
		else {
			// 로그인 요청 메시지 이전에 세션이 종료될 수 있다.
			releaseBeforeLogin = true;
		}
	}

	if (!releaseBeforeLogin) {
		BYTE loginStatus = en_Login_Status::SUCCESS;

		std::string accountNoStr = to_string(body.AccountNo);
		std::string sessionKey = "";

		if (m_ConnToRedis) {
			RedisCpp::CRedisConn* redisConn = NULL;
			while (true) {	// redisConnect 획득까지 폴링
				m_RedisConnPool.Dequeue(redisConn);
				if (redisConn != NULL) {
					break;
				}
			}

			if (!redisConn->get(accountNoStr, sessionKey)) {
				loginStatus = en_Login_Status::FAIL;
				DebugBreak();
			}
			else {
				uint32_t ret;
				if (!redisConn->del(accountNoStr, ret)) {
					loginStatus = en_Login_Status::FAIL;
					DebugBreak();
				}
			}
			m_RedisConnPool.Enqueue(redisConn);

			if (memcmp(body.sessionKey, sessionKey.c_str(), sizeof(body.sessionKey)) != 0) {
				loginStatus = en_Login_Status::FAIL;
				DebugBreak();
			}
		}
		if (loginStatus == 0) {
			Send_RES_LOGIN(sessionID, loginStatus, body.AccountNo);
			return;
		}

		stAccoutInfo* accountInfo = m_AccountPool->Alloc();
		if (accountInfo == NULL) {
#if defined(ASSERT)
			DebugBreak();
#endif
			accountInfo = new stAccoutInfo;
		}
		memcpy(accountInfo, &body.AccountNo, sizeof(stAccoutInfo));
		accountInfo->X = -1;
		m_SessionIdAccountMap.insert({ sessionID, accountInfo });
		Send_RES_LOGIN(sessionID, loginStatus, accountInfo->AccountNo);
	}
}

void ChattingServer::Send_RES_LOGIN(UINT64 sessionID, BYTE STATUS, INT64 AccountNo)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_PACKET_CS_CHAT_RES_LOGIN));
	(*reply) << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << STATUS << AccountNo;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void ChattingServer::Proc_REQ_SECTOR_MOVE(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_SECTOR_MOVE& body)
{
	auto iter = m_SessionIdAccountMap.find(sessionID);
	if (iter == m_SessionIdAccountMap.end()) {
#if defined(ASSERT)
		DebugBreak();
#endif
		stAccoutInfo* newAccount = m_AccountPool->Alloc();
		if (newAccount == NULL) {
			newAccount = new stAccoutInfo;
		}
		memcpy(newAccount, &body.AccountNo, sizeof(stAccoutInfo));
		newAccount->X = -1;
		m_SessionIdAccountMap.insert({ sessionID, newAccount });
		iter = m_SessionIdAccountMap.find(sessionID);
	}
	stAccoutInfo* accountInfo = iter->second;

	if (accountInfo->X >= 0 && accountInfo->X <= dfSECTOR_X_MAX && accountInfo->Y >= 0 && accountInfo->Y <= dfSECTOR_Y_MAX) {
		std::map<UINT64, stAccoutInfo*>& sector = m_SectorMap[accountInfo->Y][accountInfo->X];
		sector.erase(sessionID);
	}

#if defined(ASSERT)
	if (body.SectorX < 0 || body.SectorX > dfSECTOR_X_MAX || body.SectorY < 0 || body.SectorY > dfSECTOR_Y_MAX) DebugBreak(); // 범위 초과
#endif
	accountInfo->X = body.SectorX;
	accountInfo->Y = body.SectorY;
	m_SectorMap[accountInfo->Y][accountInfo->X].insert({ sessionID, accountInfo });

	Send_RES_SECTOR_MOVE(sessionID, accountInfo->AccountNo, accountInfo->X, accountInfo->Y);
}

void ChattingServer::Send_RES_SECTOR_MOVE(UINT64 sessionID, INT64 AccountNo, WORD SectorX, WORD SectorY)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_PACKET_CS_CHAT_RES_SECTOR_MOVE));
	(*reply) << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << AccountNo << SectorX << SectorY;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void ChattingServer::Proc_REQ_MESSAGE(UINT64 sessionID, MSG_PACKET_CS_CHAT_REQ_MESSAGE& body, BYTE* message)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_PACKET_CS_CHAT_RES_MESSAGE) + body.MessageLen);

	auto iter = m_SessionIdAccountMap.find(sessionID);
#if defined(ASSERT)
	if (iter == m_SessionIdAccountMap.end()) {
		DebugBreak();
	}
#endif

	stAccoutInfo* accountInfo = iter->second;
	(*reply) << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
	(*reply) << accountInfo->AccountNo << accountInfo->ID << accountInfo->Nickname << body.MessageLen;
	reply->Enqueue(message, body.MessageLen);

	for (int y = accountInfo->Y - 1; y <= accountInfo->Y + 1; y++) {
		for (int x = accountInfo->X - 1; x <= accountInfo->X + 1; x++) {
			if (y < 0 || y > dfSECTOR_Y_MAX || x < 0 || x > dfSECTOR_X_MAX) continue;

			std::map<UINT64, stAccoutInfo*>& sector = m_SectorMap[y][x];
			for (auto iter = sector.begin(); iter != sector.end(); iter++) {
				AddRefSerialBuff(reply);
				if (iter->first == sessionID) {
					if (!SendPacket(sessionID, reply, true, false)) FreeSerialBuff(reply);
					else  iter->second->sendDelayed = false;
				}
				else {
					// 주변 영역의 플레이어들에겐 packet 버퍼링 후 send
					if (!BufferSendPacket(iter->first, reply, false)) FreeSerialBuff(reply);
					else  iter->second->sendDelayed = true;
				}
			}
		}
	}

	FreeSerialBuff(reply);
}

void ChattingServer::Proc_SessionJoin(UINT64 sessionID)
{
#if defined(ASSERT)
	if (m_LoginWaitSessions.find(sessionID) != m_LoginWaitSessions.end()) DebugBreak();
#endif
	m_LoginWaitSessions.insert(sessionID);
}

void ChattingServer::Proc_SessionRelease(UINT64 sessionID)
{
	auto iter = m_LoginWaitSessions.find(sessionID);
	if (iter != m_LoginWaitSessions.end()) {
		m_LoginWaitSessions.erase(sessionID);
	}
	else {
		if (m_SessionIdAccountMap.find(sessionID) == m_SessionIdAccountMap.end()) {
#if defined(ASSERT)
			DebugBreak();
#endif
			return;
		}

		// account 및 섹터 자료구조 정리
		stAccoutInfo* accountInfo = m_SessionIdAccountMap[sessionID];
		if (accountInfo->X >= 0 && accountInfo->X <= dfSECTOR_X_MAX && accountInfo->Y >= 0 && accountInfo->Y <= dfSECTOR_Y_MAX) {
			std::map<UINT64, stAccoutInfo*>& sector = m_SectorMap[accountInfo->Y][accountInfo->X];
			sector.erase(sessionID);
		}
		m_SessionIdAccountMap.erase(sessionID);
		m_AccountPool->Free(accountInfo);
	}

#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
	auto siter = m_SessionMessageQueue.find(sessionID);
	AcquireSRWLockShared(&m_SessionMessageQueueLock);
	if (siter != m_SessionMessageQueue.end()) {
		LockFreeQueue<JBuffer*>& msgQueue = siter->second;
		while (msgQueue.GetSize() > 0) {
			JBuffer* msg;
			if (msgQueue.Dequeue(msg)) {
				FreeSerialBuff(msg);
			}
		}
	}
	ReleaseSRWLockShared(&m_SessionMessageQueueLock);

	AcquireSRWLockExclusive(&m_SessionMessageQueueLock);
	if (siter != m_SessionMessageQueue.end()) {
		m_SessionMessageQueue.erase(siter);
	}
	ReleaseSRWLockExclusive(&m_SessionMessageQueueLock);
#endif
}

UINT __stdcall ChattingServer::ProcessThreadFunc(void* arg)
{
	ChattingServer* server = reinterpret_cast<ChattingServer*>(arg);
	server->AllocTlsMemPool();

	clock_t timestamp = clock();
	while (!server->m_StopFlag) {
#if defined(PROCESSING_THREAD_WAKE_BY_WORKERS_TLS_EVENT)
		DWORD ret = WaitForMultipleObjects(CHATTING_SERVER_IOCP_WORKER_THREAD, server->m_WorkersRecvEvents, FALSE, INFINITE);
		if (WAIT_OBJECT_0 <= ret && ret < WAIT_OBJECT_0 + CHATTING_SERVER_IOCP_WORKER_THREAD) {
			HANDLE recvEvent = server->m_WorkersRecvEvents[ret];

			LockFreeQueue<stRecvInfo>& recvInfoQueue = server->m_ThrdEventRecvInfoMap[recvEvent];
			long recvInfoQueueSize = recvInfoQueue.GetSize();
			for (; recvInfoQueueSize > 0; recvInfoQueueSize--) {
				stRecvInfo recvInfo;
				recvInfoQueue.Dequeue(recvInfo, true);

#if defined(ASSERT)
				if (server->m_SessionMessageQueue.find(recvInfo.sessionID) == server->m_SessionMessageQueue.end()) {
					DebugBreak();
				}
#endif

				LockFreeQueue<JBuffer*>& sessionMessageQueue = server->m_SessionMessageQueue[recvInfo.sessionID];
				for (; recvInfo.recvMsgCnt > 0; recvInfo.recvMsgCnt--) {
					JBuffer* chatMsg;
					sessionMessageQueue.Dequeue(chatMsg);
					server->ProcessMessage(recvInfo.sessionID, chatMsg);
				}
			}
		}
#else	// PROCESSING_THREAD_POLLING_SINGLE_MESSAGE_QUEUE
//		LONG messageQueueSize = server->m_MessageLockFreeQueue.GetSize();
//		if (messageQueueSize > 0) {
//			--messageQueueSize;
//
//			pair<SessionTimeStamp, JBuffer*> jobMsg;
//			if (!server->m_MessageLockFreeQueue.Dequeue(jobMsg, true)) {
//#if defined(ASSERT)
//				DebugBreak();
//#endif
//				break;
//			}
//			server->ProcessMessage(jobMsg.first.sessionID, jobMsg.second);
//		}
		while (server->m_MessageLockFreeQueue.GetSize() > 0) {
			pair<SessionTimeStamp, JBuffer*> jobMsg;
			if (!server->m_MessageLockFreeQueue.Dequeue(jobMsg, true)) {
#if defined(ASSERT)
				DebugBreak();
#endif
				break;
			}
			server->ProcessMessage(jobMsg.first.sessionID, jobMsg.second);			
		}
#endif
	}

	// send buffered packets
	clock_t now = clock();
	if (now - timestamp > 100) {
		for (auto iter = server->m_SessionIdAccountMap.begin(); iter != server->m_SessionIdAccountMap.end(); iter++) {
			UINT64 sessionID = iter->first;
			stAccoutInfo* account = iter->second;
			if (account->sendDelayed) {
				server->SendBufferedPacket(sessionID, true);
				account->sendDelayed = false;
			}
		}
	}
	return 0;
}
