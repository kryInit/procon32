#include <direction.hpp>
#include <string>
#include <iostream>


std::string to_string(const Path& path) {
    std::string result; result.reserve(path.size());
    for(const auto& dir : path) result.push_back(dir.to_char());
    return result;
}
std::ostream& operator << (std::ostream& os, const Direction::DirectionType& dir) {
    if (dir == Direction::U) os << "U";
    else if (dir == Direction::R) os << "R";
    else if (dir == Direction::D) os << "D";
    else if (dir == Direction::L) os << "L";
    return os;
}
std::ostream& operator << (std::ostream& os, const Direction& dir) {
    os << dir.to_char();
    return os;
}
std::ostream& operator << (std::ostream& os, const Path& path) {
    os << to_string(path);
    return os;
}
