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
#include <Base/Util.h>
#include <Base/Msg.h>

namespace wind {

using namespace std;
using boost::asio::ip::tcp;

class Participant {
public:
	virtual ~Participant() {}
	virtual void Logout() = 0;
	virtual void DeliverMsg(const Msg& msg) = 0;
};
typedef shared_ptr<Participant> ParticipantRef;

class Room {
public:
	void Join(ParticipantRef participant) {
		participants_.insert(participant);
	}

	void Leave(ParticipantRef participant) {
		participants_.erase(participant);
	}

	void LeaveAll() {
		for_each(participants_.begin(), participants_.end(), [](ParticipantRef participant) {
			participant->Logout();
		});

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

class ChatSession 
	: public Participant
	, public std::enable_shared_from_this<ChatSession> 
{
public:
	ChatSession(tcp::socket socket, Room& room)
		: socket_(std::move(socket))
		, room_(room)
	{
	}

	~ChatSession() {
		LogSave("Destroy chat session...");
		//socket_.close();
	}

	void Logout() {
		LogSave("Logout chat session...");
		socket_.close();
	}

	void DeliverMsg(const Msg& msg) {
		outMsgs_.push_back(msg);
		if (outMsgs_.size() == 1) {
			WriteMsg();
		}
	}

	void Start() {
		room_.Join(shared_from_this());
		ReadMsgHeader();
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
				LogSave("Client log out");
				room_.Leave(shared_from_this());
			}
		});
	}

	void ReadMsgBody() {
		auto self(shared_from_this());
		boost::asio::async_read(socket_, boost::asio::buffer(inMsg_.Body(), inMsg_.BodyLength()),
			[this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {
				room_.DeliverMsg(inMsg_);
				ReadMsgHeader();
			}
			else {
				room_.Leave(shared_from_this());
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
				room_.Leave(shared_from_this());
			}
		});
	}

private:
	tcp::socket socket_;
	Room& room_;
	Msg inMsg_;
	deque<Msg> outMsgs_;
};

class Server {
public:
	Server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
	: acceptor_(io_service, endpoint)
	, socket_(io_service)
	{
		LogSave("Server start...");
		AcceptClient();
	}

	void DeliverMsg(const Msg& msg) { room_.DeliverMsg(msg); }

	void Close() {
		room_.LeaveAll();
	}

private:
	void AcceptClient() {
		acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
			if (!ec) {
				LogSave("Client connect...");
				std::make_shared<ChatSession>(std::move(socket_), room_)->Start();
			}
			else {
				
			}
			AcceptClient();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
	Room room_;
};

} // namespace wind
