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
	EASY_LOG << "usage: HostClient.exe [host] [port] [user]";
}

void PrintCmd() {
	EASY_LOG << "Enter number to choose operation";
	EASY_LOG << "1: restart wild";
	EASY_LOG << "2: quit";
}

int main(int argc, char *argv[])
{
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

		while (true) {
			Sleep(1000);

			switch (client.ConnectState())
			{
			case Client::EConnectState::Ok:
				break;
			case Client::EConnectState::Connecting:
				EASY_LOG << "Connecting...";
				continue;
			case Client::EConnectState::Fail:
				return 1;
			default:
				continue;
			}

			if (client.ConnectOk())
				break;
		}

		while (true) {
			EASY_LOG << "Enter password: ";

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
					EASY_LOG << "Logging...";
					continue;
				case Client::ELoginState::Fail:
					EASY_LOG << "Error pwd";
					break;
				default:
					continue;
				}

				break;
			}

			if (client.LoginOk())
				break;
		}

		PrintCmd();

		int opt = 0;
		bool bQuit = false;
		while (true) {
			cin >> opt;
			switch (opt)
			{
			case 1:
				client.SendCmd(Msg::MsgType::RestartWild);
				break;
			case 2:
				client.SendCmd(Msg::MsgType::Logout);
				client.Logout();
				bQuit = true;
				break;
			default:
				EASY_LOG << "Unknown operation";
				break;
			}
			
			if (bQuit)
				break;
		}

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

		t1.join();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	system("pause");
	return 0;
}

