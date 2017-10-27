//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/6 17:04:36
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

#include <memory>

namespace wind {

class Msg {
public:
	enum { HEADER_LENGTH = 4 };
	enum { MAX_BODY_LENGTH = 512 };
	enum class MsgType {
		None = 0,
		Login = 1,
		LoginAck = 2,
		Logout = 3,
		Cmd = 4,
		CmdAck = 5,
		QueryCmd = 6,
		QueryCmdAck = 7,
	};

	Msg() : bodyLength_(0) { memset(data_, 0, sizeof(data_)); }

	const char* Data() const { return data_; }
	char* Data() { return data_; }
	size_t Length() { return HEADER_LENGTH + bodyLength_; }
	const char* Body() const { return data_ + HEADER_LENGTH; }
	char* Body() { return data_ + HEADER_LENGTH; }
	size_t BodyLength() { return bodyLength_; }
	void SetBodyLength(size_t newLength) { bodyLength_ = std::min(newLength, size_t(MAX_BODY_LENGTH)); }
	void Clear() { memset(data_, 0, sizeof(data_)); }

	bool DecodeHeader() {
		char header[HEADER_LENGTH + 1] = "";
		strncat_s(header, data_, HEADER_LENGTH);
		bodyLength_ = atoi(header);
		if (bodyLength_ > MAX_BODY_LENGTH) {
			bodyLength_ = 0;
			return false;
		}
		return true;
	}

	void EncodeHeader() {
		char header[HEADER_LENGTH + 1] = "";
		sprintf_s(header, HEADER_LENGTH + 1, "%4d", static_cast<int>(bodyLength_));
		memcpy(data_, header, HEADER_LENGTH);
	}

private:
	char data_[HEADER_LENGTH + MAX_BODY_LENGTH];
	size_t bodyLength_;
};
typedef std::shared_ptr<Msg> MsgRef;

} // namespace wind