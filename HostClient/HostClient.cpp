// WindClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Client.h"

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <fstream>	// ifstream
#include <sstream>
#include <conio.h>	// getch()

using namespace std;
using namespace wind;

#define EQ_LINE_SEPARATOR "=================================================="

struct SHostInfo {
	SHostInfo(int id, string ip, string port, string info) 
		: id_(id), ip_(ip), port_(port), info_(info) {}

	int id_;
	string ip_;
	string port_;
	string info_;
};
map<int, SHostInfo> hosts;


void ForkClient(int clientId) {
	// 每个线程初始化种子
	srand(static_cast<int>(::time(nullptr)) + clientId);

	try {
		boost::asio::io_service io_service;

		tcp::resolver resovler(io_service);
		auto endpoint_iterator = resovler.resolve({ "localhost", "13579" });
		Client client(io_service, endpoint_iterator, clientId);

		io_service.run();

		// 		std::thread t([&io_service]() { io_service.run(); });
		// 		t.join();

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
		cout << "Exception: " << e.what() << endl;
	}
}

void WaitAMoment() {

	vector<thread> clients;
	clients.reserve(500);
	for (int i = 0; i < 1; ++i) {
		clients.emplace_back(ForkClient, i);
	}

	// 	map<int, ostringstream> ossMap;
	// 	auto& oss1 = ossMap[1];
	// 	auto& oss2 = ossMap[2];
	// 	auto& oss3 = ossMap[3];
	// 	std::thread t1([&oss1]() {RandContent(1, oss1); });
	// 	std::thread t2([&oss2]() {RandContent(2, oss2); });
	// 	std::thread t3([&oss3]() {RandContent(3, oss3); });

	// 	for (int i = 0; i < 3; ++i) {
	// 		auto& oss = ossMap[i];
	// 	}

	// 	for (int i = 0; i < 100; ++i) {
	// 		int nRand = (rand() + time(nullptr)) % gContents.size();
	// 		cout << gContents[nRand] << endl;
	// 	}

	for_each(clients.begin(), clients.end(), [](thread& t) {
		t.join();
	});

	system("pause");
}

int main(int argc, char *argv[])
{
	hosts.insert(make_pair(1, SHostInfo(1, "localhost", "13579", "野外战场")));

	for (int i = 0; i < argc; ++i) {
		cout << argv[i] << endl;
	}

	cout << "Welcome to host client" << endl;
	cout << "enter number to choose host" << endl;
	cout << EQ_LINE_SEPARATOR << endl;
	for_each(hosts.begin(), hosts.end(), [](const pair<int, SHostInfo>& hostPair)->void {
		cout << hostPair.second.id_ << ": " << hostPair.second.info_ << endl;
	});
	cout << EQ_LINE_SEPARATOR << endl;

	int id;
	int c;
	string user;
	string pwd;
	while ((c=_getch()) != EOF) {
		system("cls");
	}
	while (cin >> id) {
		if (!hosts.count(id)) {
			cout << "invalid id, please enter again" << endl;
		}
		else {
			cout << "try to login host " << id << endl;
		}
	}

	system("pause");
	return 0;
}

