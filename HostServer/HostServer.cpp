// WindServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Server.h"
#include <thread>

using boost::asio::ip::tcp;
using namespace wind;

int main()
{
	try {

		boost::asio::io_service io_service;

		tcp::endpoint endpoint(tcp::v4(), 13579);
		Server server(io_service, endpoint);

		thread t([&io_service, &server]() {
			while (true) {
				io_service.post([&server]() {
					char szContent[1024] = "";
					sprintf_s(szContent, 1023, "hello world %I64d", ::time(NULL));
					Msg msg;
					msg.SetBodyLength(strlen(szContent));
					memcpy(msg.Body(), szContent, msg.BodyLength());
					msg.EncodeHeader();

					server.DeliverMsg(msg);

					cout << "Deliver msg: " << szContent << endl;
				});

				Sleep(rand() % 30 * 1000);
			}
		});

		//thread t([&server]() { Sleep(5000); LogSave("Close server..."); server.Close(); });

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what();
	}

	system("pause");
	return 0;
}

