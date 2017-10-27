// WindClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Client.h"

#include <iostream>
#include <boost/asio.hpp>
#include <json/json.h>
#include <thread>
#include <fstream>	// ifstream
#include <sstream>
#include <conio.h>	// getch()

using namespace std;
using namespace wind;

void PrintUsage() {
	EASY_LOG_FILE("main") << "usage: HostClient.exe [host] [port] [user]";
}

void PrintCmd() {
	EASY_LOG_FILE("main") << "Enter number to choose operation";
	EASY_LOG_FILE("main") << "1: query cmd";
	EASY_LOG_FILE("main") << "2: quit";
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		Beep(750, 300);
		return(TRUE);

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		Beep(600, 200);
		printf("Ctrl-Close event\n\n");
		EASY_LOG_FILE("main") << "CTRL_CLOSE_EVENT";
		return(TRUE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		Beep(900, 200);
		printf("Ctrl-Break event\n\n");
		return FALSE;

	case CTRL_LOGOFF_EVENT:
		Beep(1000, 200);
		printf("Ctrl-Logoff event\n\n");
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		Beep(750, 500);
		printf("Ctrl-Shutdown event\n\n");
		EASY_LOG_FILE("main") << "CTRL_SHUTDOWN_EVENT";
		return FALSE;

	default:
		return FALSE;
	}
}

int main(int argc, char *argv[])
{
	EasyLogInit();

	//SetConsoleCtrlHandler(CtrlHandler, TRUE);

	if (argc < 4) {
		PrintUsage();
		return 0;
	}

	string host = argv[1];
	string port = argv[2];
	string user = argv[3];

	if (host.empty() || port.empty() || user.empty()) {
		PrintUsage();
		return 0;
	}

	try {
		boost::asio::io_service io_service;

		tcp::resolver resovler(io_service);
		auto endpoint_iterator = resovler.resolve({ host, port });
		Client client(io_service, endpoint_iterator);

		std::thread t1([&io_service]() { io_service.run(); });

		while (client.ConnectState() == Client::EConnectState::Connecting) {
			EASY_LOG_FILE("main") << "Connecting...";
			Sleep(1000);
		}

		if (!client.ConnectOk()) {
			Sleep(1000);
			t1.join();
			return 1;
		}

		while (true) {
			EASY_LOG_FILE("main") << "Enter password: ";

			string pwd;
			int c;
			while ((c = _getch()) != EOF) {
				if (c == '\r') {
					cout << endl;
					break;
				}
				pwd += char(c);
			}

			client.Login(user, pwd);

			while (true) {
				Sleep(1000);

				switch (client.LoginState())
				{
				case Client::ELoginState::Ok:
					break;
				case Client::ELoginState::Logging:
					EASY_LOG_FILE("main") << "Logging...";
					continue;
				case Client::ELoginState::Fail:
					EASY_LOG_FILE("main") << "Error pwd";
					break;
				default:
					continue;
				}

				break;
			}

			if (client.LoginOk())
				break;
		}

		if (!client.LoginOk()) {
			Sleep(1000);
			t1.join();
			return 1;
		}

		volatile int opt = 0;
		thread t2([&opt, &client]()->void {
			int input = 0;
			while (true) {
				if (!client.LoginOk())
					break;

				if (!opt) {
					PrintCmd();

					cin >> input;

					if (!client.LoginOk())
						break;

					if (input > 0 && input < 100)
						opt = input;
					else
						EASY_LOG_FILE("main") << "Unknown option";
				}

				Sleep(100);
			}
		});

		while (client.LoginOk()) {
			if (opt) {
				bool bQuit = false;

				if (opt == 1) {
					client.QueryCmd();
				}
				else if (opt == 2) {
					client.Logout();
				}
				else if (opt >= 10 && opt <= 99) {
					client.SendCmd(opt);
				}

				opt = 0;
			}

			Sleep(100);
		}


		t1.join();
		EASY_LOG_FILE("main") << "exit t1";

		t2.join();
		EASY_LOG_FILE("main") << "exit t2";

		EASY_LOG_FILE("main") << "GoodBye!";

		Sleep(1000);

		// 		char szContent[Msg::MAX_BODY_LENGTH + 1] = "";
		// 		while (std::cin.getline(szContent, Msg::MAX_BODY_LENGTH + 1)) {
		// 			Msg msg;
		// 			msg.SetBodyLength(strlen(szContent));
		// 			memcpy(msg.Body(), szContent, msg.BodyLength());
		// 			msg.EncodeHeader();
		// 			client.SendMsg(msg);
		// 		}

		// 		while (true) {
		// // 			if (client.Online()) {
		// //  				string content = RandContent();
		// //  				Msg msg;
		// //  				msg.SetBodyLength(content.length());
		// //  				memcpy(msg.Body(), content.c_str(), msg.BodyLength());
		// //  				msg.EncodeHeader();
		// //  				client.SendMsg(msg);
		// // 			}
		// // 
		// // 			Sleep(rand() % 30 * 1000);
		// 
		// 			if (client.Online()) {
		// 				Sleep(5000);
		// 				client.Logout();
		// 				t.join();
		// 				break;
		// 			}
		// 		}

	}
	catch (std::exception& e) {
		EASY_LOG_FILE("main") << "Exception: " << e.what();
	}

	return 0;
}

