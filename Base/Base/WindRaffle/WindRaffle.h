//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/9/27 17:07:37
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

#include <iterator>
#include <string>
#include <algorithm>
#include <string>

using std::string;

namespace wind
{

typedef unsigned int uint32;

template<class TDataVec, class TDataMap>
void CopyMapValueIntoVec1(TDataVec& dataVec, TDataMap& dataMap) {
	for_each(dataMap.begin(), dataMap.end(), [&dataVec](const typename TDataMap::value_type& dataPair)->void {
		dataVec.push_back(dataPair.second);
	});
}

template<class TDataVec, class TDataMap>
void CopyMapValueIntoVec2(TDataVec& dataVec, TDataMap& dataMap) {
	std::transform(dataMap.begin(), dataMap.end(), std::back_inserter(dataVec), [](auto& dataPair) {
		return dataPair.second;
	});
}

template<class TDataVec, class TDataMap>
void CopyMapValueIntoVec3(TDataVec& dataVec, TDataMap& dataMap) {
	std::transform(dataMap.begin(), dataMap.end(), std::back_inserter(dataVec), std::bind(&TDataMap::value_type::second, std::placeholders::_1));
}

template<class TDataVec, class TDataMap>
void CopyMapKeyIntoVec(TDataVec& dataVec, TDataMap& dataMap) {
	for_each(dataMap.begin(), dataMap.end(), [&dataVec](const typename TDataMap::value_type& dataPair)->void {
		dataVec.push_back(dataPair.first);
	});
}

inline bool IsOdd(int nNum) { return (nNum & 1); }

// É¾³ý×Ö·û´®µÄ¿Õ¸ñ
// boost::erase_all(str, " ")
inline string RemoveSpace1(string str) { str.erase(remove_if(str.begin(), str.end(), isspace), str.end()); }
inline string RemoveSpace2(string str) { str.erase(remove(str.begin(), str.end(), ' '), str.end()); }

bool DirExists(string dir);

uint32 GetYYMMDDHHMM(time_t tCur = 0);

// yy-mm-dd hh:mm:ss
string GetTimeStamp(time_t tCur = 0);

} // namespace wind