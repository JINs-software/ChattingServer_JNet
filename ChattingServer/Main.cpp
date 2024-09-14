#include "ChattingServer.h"
#include <conio.h>
#include <time.h>

int main() {
	std::cout << "Priority Boost: ";
	int prior;
	std::cin >> prior;
	HANDLE hProcess = GetCurrentProcess();
	// 프로세스 우선순위를 REALTIME_PRIORITY_CLASS로 설정
	//ABOVE_NORMAL_PRIORITY_CLASS: 높은 우선순위, HIGH_PRIORITY_CLASS : 매우 높은 우선순위
	if (prior == 1) {
		if (SetPriorityClass(hProcess, ABOVE_NORMAL_PRIORITY_CLASS)) {
			std::cout << "Process priority successfully set to REALTIME_PRIORITY_CLASS." << std::endl;
		}
		else {
			std::cerr << "Failed to set process priority." << std::endl;
		}
	}
	else if (prior == 2) {
		if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
			std::cout << "Process priority successfully set to REALTIME_PRIORITY_CLASS." << std::endl;
		}
		else {
			std::cerr << "Failed to set process priority." << std::endl;
		}
	}

	ChattingServer chatserver(
		false,
		CHATTING_SERVER_IP, CHATTING_SERVER_PORT, CHATTING_SERVER_MAX_OF_CONNECTIONS,
		MONT_SERVER_PROTOCOL_CODE, CHATTING_SERVER_PROTOCOL_CODE, CHATTING_SERVER_PACKET_KEY,
		CHATTING_SERVER_RECV_BUFFERING_MODE,
		CHATTING_SERVER_MAX_OF_CONNECTIONS,
		CHATTING_SERVER_IOCP_CONCURRENT_THREAD, CHATTING_SERVER_IOCP_WORKER_THREAD,
		CHATTING_SERVER_TLS_MEM_POOL_UNIT_CNT, CHATTING_SERVER_TLS_MEM_POOL_UNIT_CAPACITY,
		CHATTING_SERVER_SERIAL_BUFFER_SIZE,
		CHATTING_SERVER_SESSION_RECV_BUFF_SIZE,
		CHATTING_SERVER_CALC_TPS_THREAD
	);

	if (!chatserver.Start()) {
		return 0;
	}

	char ctr;
	clock_t ct = 0;
	while (true) {
		if (_kbhit()) {
			ctr = _getch();
			if (ctr == 'q' || ctr == 'Q') {
				break;
			}
		}


		chatserver.PrintServerInfoOnConsole();
		Sleep(1000);
	}

	chatserver.Stop();
}