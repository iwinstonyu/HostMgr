//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/6 13:39:10
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
#include <json/json.h>
#include <deque>
#include <Base/Msg.h>
#include <Base/Log/Log.h>
#include <Base/SingleQueue/SingleQueue.h>
#include <thread>

namespace wind {

using boost::asio::ip::tcp;
using namespace std;

class Client {
public:
	enum class EConnectState { None, Connecting, Fail, Ok };
	enum class ELoginState { None, Logging, Fail, Ok };

public:
	Client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service)
		, socket_(io_service)
		, connectState_(EConnectState::None)
		, loginState_( ELoginState::None )
		, outMsgs_(1000)
		, inMsgs_(1000)
		, bNetWriteMsg_(false)
	{
		ConnectServer(endpoint_iterator);
	}

	~Client() {
		thProcessMsg_.join();
	}

	Client::EConnectState ConnectState() { return connectState_; }
	bool ConnectOk() { return connectState_ == EConnectState::Ok; }
	Client::ELoginState LoginState() { return loginState_; }
	bool LoginOk() { return loginState_ == ELoginState::Ok; }

	void SendMsg(const Msg& msg) {
		if (!LoginOk()) {
			LogSave("SendMsg not login");
			return;
		}

		LogSave("SendMsg:%s", msg.Body());
		WriteMsg(msg);
	}

	bool SendCmd(Msg::MsgType cmd) {
		if (!LoginOk()) {
			LogSave("SendCmd not login");
			return false;
		}

		Json::Value valUser;
		valUser["msgType"] = static_cast<int>(cmd);
		Json::FastWriter writer;
		string strMsgBody = writer.write(valUser);

		Msg msg;
		msg.SetBodyLength(strMsgBody.length());
		memcpy(msg.Body(), strMsgBody.c_str(), msg.BodyLength());
		msg.EncodeHeader();

		LogSave("SendCmd:%s", msg.Body());
		WriteMsg(msg);
		return true;
	}

	bool Login(string user, string pwd) {
		if (!ConnectOk())
			return false;

		loginState_ = ELoginState::Logging;

		Json::Value valUser;
		valUser["msgType"] = static_cast<int>(Msg::MsgType::Login);
		valUser["user"] = user;
		valUser["pwd"] = pwd;
		Json::FastWriter writer;
		string strMsgBody = writer.write(valUser);

		Msg msg;
		msg.SetBodyLength(strMsgBody.length());
		memcpy(msg.Body(), strMsgBody.c_str(), msg.BodyLength());
		msg.EncodeHeader();

		WriteMsg(msg);
		return true;
	}

	void Logout() {
		EASY_LOG << "Logout...";
		io_service_.post([this]() { socket_.close(); });
		connectState_ = EConnectState::None;
		loginState_ = ELoginState::None;
	}

private:
	void ConnectServer(tcp::resolver::iterator endpoint_iterator) {
		EASY_LOG << "Connecting...";
		connectState_ = EConnectState::Connecting;

		boost::asio::async_connect(socket_, endpoint_iterator, 
			[this](boost::system::error_code ec, tcp::resolver::iterator) {
			if (!ec) {
				connectState_ = EConnectState::Ok;
				EASY_LOG << "Connect succ";

				ReadMsgHeader();

				std::thread t1(std::bind(&Client::ProcessMsg, this));
				thProcessMsg_.swap(t1);
			}
			else {
				EASY_LOG << "Connect fail";
			}
		});
	}

	void ProcessMsg() {
		while (ConnectOk()) {
			Sleep(1000);

			MsgRef msgRef;
			while (ConnectOk() && (msgRef = inMsgs_.Read())) {
				Json::Value valMsg;
				Json::Reader reader;
				if (!reader.parse(msgRef->Body(), msgRef->Body() + msgRef->BodyLength(), valMsg)) {
					EASY_LOG << "ProcessMsg fail parse: " << msgRef->Body();
					continue;
				}

				switch (valMsg["msgType"].asUInt())
				{
				case Msg::MsgType::LoginAck:
				{
					if (valMsg["result"].asInt() == 0) {
						loginState_ = ELoginState::Ok;
						EASY_LOG << "Login succ";
					}
					else {
						loginState_ = ELoginState::Fail;
						EASY_LOG << "Login fail";
					}
				}
				break;
				default:
					EASY_LOG << "ProcessMsg unknown: " << msgRef->Body();
					break;
				}
			}
		}
	}

	void ReadMsgHeader() {
		inMsg_.Clear();
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Data(), Msg::HEADER_LENGTH),
			[this](boost::system::error_code ec, size_t length) {
			if (!ec && inMsg_.DecodeHeader()) {
				ReadMsgBody();
			}
			else {
				EASY_LOG << "ReadMsgHeader fail";
				socket_.close();
			}
		});
	}

	void ReadMsgBody() {
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Body(), inMsg_.BodyLength()),
			[this](boost::system::error_code ec, size_t length) {
			if (!ec) {
				EASY_LOG << "ReadMsg: " << inMsg_.Body();

				inMsgs_.Write(make_shared<Msg>(inMsg_));

				ReadMsgHeader();
			}
			else {
				EASY_LOG << "ReadMsgBody fail";
				socket_.close();
			}
		});
	}

	void WriteMsg(const Msg& msg) {
		outMsgs_.Write(make_shared<Msg>(msg));
		
		if (!bNetWriteMsg_) 
			NetWriteMsg();
	}

	void NetWriteMsg() {
		MsgRef msgRef;
		if (msgRef = outMsgs_.Read()) {
			bNetWriteMsg_ = true;
			boost::asio::async_write(socket_, boost::asio::buffer(msgRef->Data(), msgRef->Length()),
				[this](boost::system::error_code ec, size_t length) {
				if (!ec) {
					EASY_LOG << "WriteMsg succ";
					NetWriteMsg();
				}
				else {
					EASY_LOG << "WriteMsg fail";
					socket_.close();
				}
			});
		}
		else {
			bNetWriteMsg_ = false;
		}
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	Msg inMsg_;
	EConnectState connectState_;
	ELoginState loginState_;
	SingleQueue<Msg> outMsgs_;
	SingleQueue<Msg> inMsgs_;
	bool bNetWriteMsg_;
	thread thProcessMsg_;
};

} // namespace wind