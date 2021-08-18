#pragma once
#include <string>
#include <vector>
#include <iostream>

struct Direction {
    // [Caution] DO NOT change the order in DirectionType
    enum DirectionType { U, R, D, L } type;
    constexpr Direction();
    constexpr Direction(enum DirectionType type);

    [[nodiscard]] constexpr int dx() const;
    [[nodiscard]] constexpr int dy() const;
    [[nodiscard]] constexpr char to_char() const;

    constexpr bool operator==(const Direction& another) const;
    constexpr bool operator==(const DirectionType& another) const;
    constexpr bool operator!=(const Direction& another) const;
    constexpr bool operator!=(const DirectionType& another) const;


    constexpr void rotate_cw(int times);
    constexpr void reverse(int times);
    constexpr void rotate_ccw(int times);
    constexpr void rotate_cw();
    constexpr void reverse();
    constexpr void rotate_ccw();
    [[nodiscard]] constexpr DirectionType get_dir_rotated_cw(int times) const;
    [[nodiscard]] constexpr DirectionType get_dir_reversed(int times) const;
    [[nodiscard]] constexpr DirectionType get_dir_rotated_ccw(int times) const;
    [[nodiscard]] constexpr DirectionType get_dir_rotated_cw() const;
    [[nodiscard]] constexpr DirectionType get_dir_reversed() const;
    [[nodiscard]] constexpr DirectionType get_dir_rotated_ccw() const;
};

using Path = std::vector<Direction>;
std::string to_string(const Path& path);
std::ostream& operator << (std::ostream& os, const Direction::DirectionType& dir);
std::ostream& operator << (std::ostream& os, const Direction& dir);
std::ostream& operator << (std::ostream& os, const Path& path);


// あんんまりよくわかってないけどconstexprつけてコンパイル時に計算させると実装をヘッダにしておかないとundefined symbolでエラーになるっぽい
inline constexpr Direction::Direction() : type() {}
inline constexpr Direction::Direction(enum DirectionType type) : type(type) {}

inline constexpr int Direction::dx() const {
    if (type == R) return 1;
    else if (type == L) return -1;
    else return 0;
}
inline constexpr int Direction::dy() const {
    if (type == D) return 1;
    else if (type == U) return -1;
    else return 0;
}
inline constexpr char Direction::to_char() const {
    if (type == U) return 'U';
    else if (type == R) return 'R';
    else if (type == D) return 'D';
    else return 'L';
}

inline constexpr bool Direction::operator==(const Direction& another) const { return type == another.type; }
inline constexpr bool Direction::operator==(const DirectionType& another) const { return type == another; }
inline constexpr bool Direction::operator!=(const Direction& another) const { return type != another.type; }
inline constexpr bool Direction::operator!=(const DirectionType& another) const { return type != another; }

inline constexpr void Direction::rotate_cw(int times)   { type = static_cast<DirectionType>((static_cast<int>(type) + times) % 4); }
inline constexpr void Direction::reverse(int times)     { type = static_cast<DirectionType>((static_cast<int>(type) + 2*times) % 4); }
inline constexpr void Direction::rotate_ccw(int times)  { type = static_cast<DirectionType>((static_cast<int>(type) + 3*times) % 4); }
inline constexpr void Direction::rotate_cw()    { rotate_cw(1); }
inline constexpr void Direction::reverse()      { reverse(1); }
inline constexpr void Direction::rotate_ccw()   { rotate_ccw(1); }
inline constexpr Direction::DirectionType Direction::get_dir_rotated_cw(int times)   const { return static_cast<DirectionType>((static_cast<int>(type) + times) % 4); }
inline constexpr Direction::DirectionType Direction::get_dir_reversed(int times)     const { return static_cast<DirectionType>((static_cast<int>(type) + 2*times) % 4); }
inline constexpr Direction::DirectionType Direction::get_dir_rotated_ccw(int times)  const { return static_cast<DirectionType>((static_cast<int>(type) + 3*times) % 4); }
inline constexpr Direction::DirectionType Direction::get_dir_rotated_cw()    const { return get_dir_rotated_cw(1); }
inline constexpr Direction::DirectionType Direction::get_dir_reversed()      const { return get_dir_reversed(1); }
inline constexpr Direction::DirectionType Direction::get_dir_rotated_ccw()   const { return get_dir_rotated_ccw(1); }
