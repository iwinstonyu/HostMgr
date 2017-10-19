//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/4 15:34:42
//	Version 1.0
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <deque>
#include <thread>
#include <Base/Msg.h>
#include <json/json.h>
#include <map>
#include <string>
#include <fstream>
#include <Base/Log/Log.h>
using namespace std;

extern map<string, string> g_users;

namespace wind {

using boost::asio::ip::tcp;

class Participant {
public:
	virtual ~Participant() {}
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void DeliverMsg(const Msg& msg) = 0;
};
typedef shared_ptr<Participant> ParticipantRef;

class Room {
public:
	void Start(ParticipantRef participant) {
		participants_.insert(participant);
		participant->Start();
	}

	void Stop(ParticipantRef participant) {
		participants_.erase(participant);
		participant->Stop();
	}

	void StopAll() {
		for_each(participants_.begin(), participants_.end(),
			std::bind(&Participant::Stop, std::placeholders::_1));

		participants_.clear();
	}

	void DeliverMsg(const Msg& msg) {
		for (auto participant : participants_) {
			participant->DeliverMsg(msg);
		}
	}

private:
	std::set<ParticipantRef> participants_;
};

class Server;

class ChatSession 
	: public Participant
	, public std::enable_shared_from_this<ChatSession> 
{
public:
	ChatSession(tcp::socket socket, Room& room, Server& server)
		: socket_(std::move(socket))
		, room_(room)
		, server_(server)
	{
	}

	void Start() {
		ReadMsgHeader();
	}

	void Stop() {
		EASY_LOG_FILE("main") << "Stop Client";
		socket_.close();
	}

	void DeliverMsg(const Msg& msg) {
		outMsgs_.push_back(msg);
		if (outMsgs_.size() == 1) {
			WriteMsg();
		}
	}

private:
	void ReadMsgHeader() {
		inMsg_.Clear();
		auto self(shared_from_this());
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Data(), Msg::HEADER_LENGTH),
			[this, self](boost::system::error_code ec, size_t length) {
			if (!ec && inMsg_.DecodeHeader()) {
				ReadMsgBody();
			}
			else {
				room_.Stop(shared_from_this());
				return;
			}
		});
	}

	void ReadMsgBody() {
		auto self(shared_from_this());
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Body(), inMsg_.BodyLength()),
			[this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {
				EASY_LOG_FILE("main") << "inMsg: " << inMsg_.Body();

				Json::Value valMsg;
				Json::Reader reader;
				if (!reader.parse(inMsg_.Body(), inMsg_.Body() + inMsg_.BodyLength(), valMsg)) {
					room_.Stop(shared_from_this());
					return;
				}

				switch (valMsg["msgType"].asInt())
				{
				case Msg::MsgType::Login:
				{
					Json::Value valAck;
					valAck["msgType"] = static_cast<int>(Msg::MsgType::LoginAck);

					string user = valMsg["user"].asString();
					string pwd = valMsg["pwd"].asString();
					bool result = g_users.count(user) && g_users[user] == pwd;

					valAck["result"] = result ? 0 : 1;
					Json::FastWriter writer;
					string strMsgBody = writer.write(valAck);

					Msg msg;
					msg.SetBodyLength(strMsgBody.length());
					memcpy(msg.Body(), strMsgBody.c_str(), msg.BodyLength());
					msg.EncodeHeader();
					DeliverMsg(msg);

					if (!result) {
						EASY_LOG_FILE("main") << "Fail validate user";
					}
				}
				break;
				case Msg::MsgType::Logout:
				{

				}
				break;
				case Msg::MsgType::RestartWild:
				{
					system("..\\killgs.bat");
					system("..\\killlp.bat");
					Sleep(3000);
					system("..\\startlp.bat");
					system("..\\startgs.bat");
				}
				break; 
				default:
					break;
				}

				ReadMsgHeader();
			}
			else {
				room_.Stop(shared_from_this());
			}
		});
	}

	void WriteMsg() {
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(outMsgs_.front().Data(), outMsgs_.front().Length()), 
			[this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {
				outMsgs_.pop_front();
				if (!outMsgs_.empty())
					WriteMsg();
			}
			else {
				room_.Stop(shared_from_this());
			}
		});
	}

private:
	tcp::socket socket_;
	Room& room_;
	Server& server_;
	Msg inMsg_;
	deque<Msg> outMsgs_;
};

class Server {
public:
	Server()
		: io_service_()
		, acceptor_(io_service_)
		, socket_(io_service_)
		, signals_(io_service_)
		, stop_(false)
	{
//		signals_.add(SIGINT);
//		signals_.add(SIGTERM);
//		signals_.add(SIGBREAK);
//#if defined(SIGQUIT)
//		signals_.add(SIGQUIT);
//#endif // defined(SIGQUIT)
//		signals_.async_wait(std::bind(&Server::Stop, this));

		tcp::endpoint endpoint(tcp::v4(), 31234);
		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();

		AcceptClient();

		EASY_LOG_FILE("main") << "Server start";
	}

	~Server() {
		EASY_LOG_FILE("main") << "Server destroy";
	}

	void Run() {
		io_service_.run();
	}

	void Stop() {
		if (stop_)
			return;
		stop_ = true;

		EASY_LOG_FILE("main") << "Server stop";

		acceptor_.close();
		room_.StopAll();
	}

	void DeliverMsg(const Msg& msg) { room_.DeliverMsg(msg); }

private:
	void AcceptClient() {
		acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
			if (!acceptor_.is_open()) {
				EASY_LOG_FILE("main") << "Acceptor not open";
				return;
			}

			if (!ec) {
				EASY_LOG_FILE("main") << "Client connect";
				room_.Start(std::make_shared<ChatSession>(std::move(socket_), room_, *this));
			}
			else {
				
			}
			AcceptClient();
		});
	}

	boost::asio::io_service io_service_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	Room room_;
	boost::asio::signal_set signals_;
	bool stop_;
};

} // namespace wind
