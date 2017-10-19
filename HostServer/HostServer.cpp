// WindServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Server.h"
#include <thread>

using boost::asio::ip::tcp;
using namespace wind;

map<string, string> g_users;

// ��������ڳ����˳�ʱ����������
std::function<void()> gCtrlRear;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	EASY_LOG_FILE("main") << "CtrlHandler: " << fdwCtrlType;

	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		gCtrlRear();
		return TRUE;
	default:
		return FALSE;
	}
}

int main()
{
	EasyLogInit();

	g_users["ywj"] = "123";
	g_users["cyq"] = "123";
	g_users["llc"] = "123";

	try {
		Server s;

		gCtrlRear = std::bind(&Server::Stop, &s);
		SetConsoleCtrlHandler(CtrlHandler, TRUE);

		s.Run();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	EASY_LOG_FILE("main") << "Main exit";

	return 0;
}

