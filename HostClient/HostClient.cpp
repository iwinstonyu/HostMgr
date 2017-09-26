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
	cout << "usage: HostClient.exe [host] [port] [user]" << endl;
}

void PrintHelp() {
	cout << "Enter number to choose operation" << endl;
	cout << "1: restart wild" << endl;
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

	cout << "Enter password: ";

	string pwd;
	int c;
	while ((c=_getch()) != EOF ) {
		if (c == '\r') {
			cout << endl;
			break;
		}
		pwd += char(c);
	}

	if (pwd.empty()) {
		PrintUsage();
		return 0;
	}

	try {
		boost::asio::io_service io_service;

		tcp::resolver resovler(io_service);
		auto endpoint_iterator = resovler.resolve({ host, port });
		Client client(io_service, endpoint_iterator, user, pwd);

		std::thread t([&io_service]() { io_service.run(); });

		while (true) {
			LogSave("Waiting response...");
			Sleep(1000);

			if (client.Validate())
				break;
		}

		PrintHelp();

		int opt;
		while (true) {
			cin >> opt;
			switch (opt)
			{
			case 1:
				
			}
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

		client.Logout();
		t.join();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	system("pause");
	return 0;
}

