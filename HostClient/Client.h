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
#include <deque>
#include <Base/Msg.h>
#include <Base/Util.h>

namespace wind {

using boost::asio::ip::tcp;
using namespace std;

class Client {
public:
	Client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator, int clientId)
		: io_service_(io_service)
		, socket_(io_service)
		, clientId_(clientId)
		, online_(false)
	{
		ConnectServer(endpoint_iterator);
	}

	bool Online() { return online_; }

	void SendMsg(const Msg& msg) {
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
	}

private:
	void ConnectServer(tcp::resolver::iterator endpoint_iterator) {
		LogSave("Client[%d] conecting...", clientId_);
		boost::asio::async_connect(socket_, endpoint_iterator, 
			[this](boost::system::error_code ec, tcp::resolver::iterator) {
			if (!ec) {
				online_ = true;
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
				LogSave("%s", inMsg_.Body());
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
};

} // namespace wind