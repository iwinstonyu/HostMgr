// WindServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Server.h"
#include <thread>

using boost::asio::ip::tcp;
using namespace wind;

extern map<string, string> g_users;


int main()
{
	g_users["ywj"] = "123";

	try {
		boost::asio::io_service io_service;

		tcp::endpoint endpoint(tcp::v4(), 13579);
		Server server(io_service, endpoint);

		io_service.run();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	system("pause");
	return 0;
}

