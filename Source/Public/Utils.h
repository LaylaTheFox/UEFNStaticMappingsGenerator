#pragma once
#include <string>
#include <algorithm>
#include <iterator>

inline std::string WStringToString(std::wstring narrowString) {
	std::string str;
	std::transform(narrowString.begin(), narrowString.end(), std::back_inserter(str), [](wchar_t c) { return (char)c; });
	return str;
}

using uint8 = unsigned char;
using int16 = short;
using uint16 = unsigned short;
using int32 = int;
using uint32 = unsigned int;
using int64 = __int64;
using uint64 = unsigned __int64;
using EEnumFlags = uint8;
using EObjectFlags = uint32;
using ETypeConstructPhase = int32;
using EArrayPropertyFlags = uint8;
using EMapPropertyFlags = uint8;