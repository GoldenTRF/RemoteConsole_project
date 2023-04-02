#include "CommonFunc.h"

#include <iostream>
#include <fstream>

bool common::charInfoCmpPred(const CHAR_INFO& first, const CHAR_INFO& second)
{
    if (first.Char.AsciiChar == second.Char.AsciiChar) return true;
    return false;
}

bool common::vBuffersCompare(const std::vector<CHAR_INFO>& first, const std::vector<CHAR_INFO>& second)
{
    return std::equal(first.begin(), first.end(), second.begin(), common::charInfoCmpPred);
}

bool common::compareCursor(const COORD& first, const COORD& second)
{
    if (first.X == second.X && first.Y == second.Y) return true; // Cursors have the sme coordinates
    return false; // Cursors are different
}