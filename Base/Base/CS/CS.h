//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/27 19:16:59
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

#include <Windows.h>

namespace wind {

class WindCS {
public:
	WindCS() { ::InitializeCriticalSection(&cs_); }
	~WindCS() { ::DeleteCriticalSection(&cs_); }

	void Lock() { ::EnterCriticalSection(&cs_); }
	void Unlock() { ::LeaveCriticalSection(&cs_); }

private:
	WindCS(const WindCS&);
	WindCS& operator=(const WindCS&);

	::CRITICAL_SECTION cs_;
};

class WindLocker {
public:
	explicit WindLocker(WindCS* pwcs) : pwcs_(pwcs) { pwcs_->Lock(); }
	~WindLocker() { pwcs_->Unlock(); }

private:
	WindLocker(const WindLocker&);
	WindLocker& operator=(const WindLocker&);

	WindCS* pwcs_;
};

} // namespace wind