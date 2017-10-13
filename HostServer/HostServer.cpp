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
	g_users["cyq"] = "123";
	g_users["llc"] = "123";

	try {
		boost::asio::io_service io_service;

		tcp::endpoint endpoint(tcp::v4(), 31235);
		shared_ptr<Server> serverRef(make_shared<Server>(io_service, endpoint));
		//Server server(io_service, endpoint);

		SetConsoleCtrlHandler([this](DWORD CtrlType)->BOOL WINAPI {
			ofstream ofs("main.txt", ios::app);
			switch (CtrlType)
			{
			case CTRL_C_EVENT:
			case CTRL_CLOSE_EVENT:
				serverRef->Close();
				io_service.stop();
				break;
			default:
				break;
			}
			ofs.close();

			return TRUE;
		}, TRUE);

		io_service.run();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	return 0;
}

