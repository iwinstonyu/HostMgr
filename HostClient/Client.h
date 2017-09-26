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
#include <Base/Util.h>

namespace wind {

using boost::asio::ip::tcp;
using namespace std;

class Client {
public:
	Client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator, string user, string pwd)
		: io_service_(io_service)
		, socket_(io_service)
		, online_(false)
		, connecting_(false)
		, user_(user)
		, pwd_(pwd)
		, validate_(false)
	{
		ConnectServer(endpoint_iterator);
	}

	bool Online() { return online_; }
	bool Validate() { return validate_; }

	void SendMsg(const Msg& msg) {
		if (!connecting_) {
			LogSave("SendMsg not connecting");
			return;
		}

		LogSave("SendMsg:%s", msg.Body());
		io_service_.post([this, msg](){
			outMsgs_.push_back(msg);
			if (outMsgs_.size() == 1) {
				WriteMsg();
			}
		});
	}

	void Logout() {
		LogSave("Client[%d] logout...", clientId_);
		io_service_.post([this]() { socket_.close(); });
		online_ = false;
		connecting_ = false;
	}

private:
	void ConnectServer(tcp::resolver::iterator endpoint_iterator) {
		LogSave("Connecting server...");
		boost::asio::async_connect(socket_, endpoint_iterator, 
			[this](boost::system::error_code ec, tcp::resolver::iterator) {
			if (!ec) {
				connecting_ = true;
				LogSave("Validate user...");
				
				Json::Value valUser;
				valUser["msgType"] = Msg::MSG_TYPE_LOGIN;
				valUser["user"] = user_;
				valUser["pwd"] = pwd_;
				Json::FastWriter writer;
				string strMsgBody = writer.write(valUser);
				
				Msg msg;
				msg.SetBodyLength(strMsgBody.length());
				memcpy(msg.Body(), strMsgBody.c_str(), msg.BodyLength());
				msg.EncodeHeader();
				SendMsg(msg);

				ReadMsgHeader();
			}
		});
	}

	void ReadMsgHeader() {
		inMsg_.Clear();
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Data(), Msg::HEADER_LENGTH),
			[this](boost::system::error_code ec, size_t length) {
			if (!ec && inMsg_.DecodeHeader()) {
				ReadMsgBody();
			}
			else {
				socket_.close();
			}
		});
	}

	void ReadMsgBody() {
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Body(), inMsg_.BodyLength()),
			[this](boost::system::error_code ec, size_t length) {
			if (!ec) {
				LogSave("ReadMsg: %s", inMsg_.Body());

				Json::Value valMsg;
				Json::Reader reader;
				if (!reader.parse(inMsg_.Body(), inMsg_.Body() + inMsg_.BodyLength(), valMsg)) {
					LogSave("ReadMsg fail parse: %s", inMsg_.Body());
					socket_.close();
				}

				if (valMsg["msgType"] == Msg::MSG_TYPE_LOGIN_ACK) {
					if (valMsg["result"] == 0) {
						validate_ = true;
						LogSave("Login succ");
					}
					else {
						LogSave("Fail validate user");
						socket_.close();
					}
				}

				ReadMsgHeader();
			}
			else {
				socket_.close();
			}
		});
	}

	void WriteMsg() {
		boost::asio::async_write(socket_, boost::asio::buffer(outMsgs_.front().Data(), outMsgs_.front().Length()),
			[this](boost::system::error_code ec, size_t length) {
			if (!ec) {
				outMsgs_.pop_front();
				if (!outMsgs_.empty())
					WriteMsg();
			}
			else {
				socket_.close();
			}
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	Msg inMsg_;
	deque<Msg> outMsgs_;
	int clientId_;
	bool online_;
	bool connecting_;
	bool validate_;
	string user_;
	string pwd_;
};

} // namespace wind