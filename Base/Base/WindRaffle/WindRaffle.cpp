//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/27 17:09:18
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

#include "stdafx.h"

#include <sys/stat.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "WindRaffle.h"

namespace wind {

bool DirExists(string dir) {
	struct stat st;
	if (!stat(dir.c_str(), &st) && st.st_mode & S_IFDIR)
		return true;
	else
		return false;
}

uint32 GetYYMMDDHHMM(time_t tCur)
{
	if (!tCur)
		tCur = std::time(nullptr);

	struct tm t;
	localtime_s(&t, &tCur);

	return (t.tm_year - 100) * 100000000
		+ (t.tm_mon + 1) * 1000000
		+ (t.tm_mday) * 10000
		+ (t.tm_hour) * 100
		+ t.tm_min;
}

string GetTimeStamp(time_t tCur)
{
	if (!tCur)
		tCur = std::time(nullptr);

	struct tm t;
	localtime_s(&t, &tCur);

	using std::setw;
	using std::setfill;

	std::ostringstream oss;
	oss << t.tm_year - 100 
		<< "-" << setw(2) << setfill('0') << t.tm_mon + 1 
		<< "-" << setw(2) << setfill('0') << t.tm_mday 
		<< " " << setw(2) << setfill('0') << t.tm_hour 
		<< ":" << setw(2) << setfill('0') << t.tm_min 
		<< ":" << setw(2) << setfill('0') << t.tm_sec;
	return oss.str();
}

}


