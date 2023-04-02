#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace common
{
	std::string getCert(const std::string&);
	bool charInfoCmpPred(const CHAR_INFO& first, const CHAR_INFO& second);
	bool vBuffersCompare(const std::vector<CHAR_INFO>& first, const std::vector<CHAR_INFO>& second);
	bool compareCursor(const COORD&, const COORD&);
}