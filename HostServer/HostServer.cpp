// WindServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Server.h"
#include <thread>

using boost::asio::ip::tcp;
using namespace wind;

map<string, string> g_users;
map<int, SCmdData> g_cmds;

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
	char info[1024] = "";
	for (char* pszSection = sections; *pszSection != '\0'; pszSection += strlen(pszSection) + 1) {
		::GetPrivateProfileString(pszSection, "cmd", "", cmd, sizeof(cmd), cmdFile);
		::GetPrivateProfileString(pszSection, "info", "", info, sizeof(info), cmdFile);

		if (strlen(pszSection) && strlen(cmd) && strlen(info)) {
			SCmdData cmdData;
			cmdData.opt = atoi(pszSection);
			cmdData.cmd = cmd;
			cmdData.info = info;
			g_cmds[cmdData.opt] = cmdData;
		}
	}
}

int main(int argc, char *argv[])
{
	EasyLogInit();

	if (argc < 2) {
		EASY_LOG_FILE("main") << "usage: HostServer.exe [port]";
		return 1;
	}
	
	string ip = argv[1];
	int port = atoi(argv[1]);
	if (port <= 0) {
		EASY_LOG_FILE("main") << "usage: HostServer.exe [port]";
		return 1;
	}

	InitAuth();
	InitCmd();

	try {
		Server s(port);

		//gCtrlRear = {std::bind(&Server::Stop, &s);
		gCtrlRear = [&s]()->void {
			s.Stop();
			Sleep(1000);
			EASY_LOG_FILE("main") << "Goodbye!";
		};
		SetConsoleCtrlHandler(CtrlHandler, TRUE);

		s.Run();
	}
	catch (std::exception& e) {
		LogSave("Exception: %s", e.what());
	}

	EASY_LOG_FILE("main") << "Main exit";

	return 0;
}

