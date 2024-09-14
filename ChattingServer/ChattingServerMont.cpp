#include "ChattingServerMont.h"
#include "Configuration.h"
#include "Protocol.h"

void ChattingServerMont::OnConnectionToServer()
{
	JBuffer* loginPacket = AllocSerialSendBuff(sizeof(WORD) + sizeof(int));
	(*loginPacket) << (WORD)en_PACKET_SS_MONITOR_LOGIN << (int)dfSERVER_LOGIN_SERVER;
	if (!SendPacket(loginPacket)) FreeSerialBuff(loginPacket);
	else m_MontServerConnected = true;
}

void ChattingServerMont::OnDisconnectionFromServer()
{
	m_MontServerConnected = false;
}

UINT __stdcall ChattingServerMont::PerformanceMontFunc(void* arg)
{
    ChattingServerMont* mont = reinterpret_cast<ChattingServerMont*>(arg);
    mont->AllocTlsMemPool();

	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_SESSION , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_PLAYER , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL , {0} });
	mont->m_MontDataMap.insert({ dfMONITOR_DATA_TYPE_CHAT_UPDATE_WORKER_CPU , {0} });

    mont->m_PerfCounter = new PerformanceCounter();
    mont->m_PerfCounter->SetCpuUsageCounter();
	mont->m_PerfCounter->SetProcessCounter(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM, dfQUERY_PROCESS_USER_VMEMORY_USAGE, L"ChattingServer");

	while (!mont->m_ChattingServer->ServerStop()) {
		if (!mont->m_MontServerConnected) {
			mont->ResetPerfCount();
			mont->SendPerfCountToMontServer();
		}
		else {
			if (mont->ConnectToServer()) mont->m_MontServerConnected = true;
		}
		Sleep(1000);
	}

    return 0;
}

void ChattingServerMont::ResetPerfCount()
{
	time_t now = time(NULL);

	m_PerfCounter->ResetPerfCounterItems();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN].dataValue = 1;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU].dataValue = m_PerfCounter->ProcessTotal();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM].dataValue = m_PerfCounter->GetPerfCounterItem(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM);
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM].dataValue /= (1024 * 1024);	// 서버 메모리 사용량 MB
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SESSION].dataValue = m_ChattingServer->GetCurrentSessions();		// GetSessionCount
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_SESSION].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_PLAYER].dataValue = m_ChattingServer->GetPlayerCount();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_PLAYER].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS].dataValue = m_ChattingServer->GetUpdateThreadTransaction();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL].dataValue = m_ChattingServer->GetCurrentAllocatedMemUnitCnt();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL].dataValue = m_ChattingServer->GetUpdateMessageQueueSize();
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL].timeStamp = now;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATE_WORKER_CPU].dataValue = 0;
	m_MontDataMap[dfMONITOR_DATA_TYPE_CHAT_UPDATE_WORKER_CPU].timeStamp = now;
}

void ChattingServerMont::SendPerfCountToMontServer()
{
	for (const auto& it : m_MontDataMap) {
		BYTE counterType = it.first;
		const stMontData& montData = it.second;
		JBuffer* perfMsg = AllocSerialSendBuff(sizeof(stMSG_MONITOR_DATA_UPDATE));

		(*perfMsg) << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
		(*perfMsg) << counterType << montData.dataValue << montData.timeStamp;
		if (!SendPacket(perfMsg)) {
			FreeSerialBuff(perfMsg);
		}
	}
}
