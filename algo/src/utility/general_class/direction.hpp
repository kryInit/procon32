#pragma once
#include <iostream>

// [Caution] DO NOT change the order in Direction
enum class Direction { U, R, D, L };
inline std::ostream& operator << (std::ostream& os, const Direction& dir) {
    if (dir == Direction::U) os << "U";
    else if (dir == Direction::R) os << "R";
    else if (dir == Direction::D) os << "D";
    else if (dir == Direction::L) os << "L";
    return os;
}

