// WindServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Server.h"
#include <thread>

using boost::asio::ip::tcp;
using namespace wind;

map<string, string> g_users;
map<int, string> g_cmds;

// 这个函数在程序退出时进行清理工作
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

void InitAuth() {
	const char* authFile = ".\\Auth.ini";

	char sections[10240] = "";
	GetPrivateProfileSectionNames(sections, sizeof(sections), authFile);

	char name[1024] = "";
	char pwd[1024] = "";
	for (char* pszSection = sections; *pszSection != '\0'; pszSection += strlen(pszSection) + 1) {
		::GetPrivateProfileString(pszSection, "name", "", name, sizeof(name), authFile);
		::GetPrivateProfileString(pszSection, "pwd", "", pwd, sizeof(pwd), authFile);

		if (strlen(name) && strlen(pwd)) {
			g_users[name] = pwd;
		}
	}
}

void InitCmd() {
	const char* cmdFile = ".\\Cmd.ini";

	char sections[10240] = "";
	GetPrivateProfileSectionNames(sections, sizeof(sections), cmdFile);

	char cmd[1024] = "";
	for (char* pszSection = sections; *pszSection != '\0'; pszSection += strlen(pszSection) + 1) {
		::GetPrivateProfileString(pszSection, "cmd", "", cmd, sizeof(cmd), cmdFile);

		if (strlen(pszSection) && strlen(cmd) ) {
			g_cmds[atoi(pszSection)] = cmd;
		}
	}
}

int main()
{
	EasyLogInit();

	InitAuth();
	InitCmd();

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

