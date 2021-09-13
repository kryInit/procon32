#pragma once

#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <optional>

// あんんまりよくわかってないけどconstexprつけてコンパイル時に計算させると実装をヘッダにしておかないとundefined symbolでエラーになるっぽい

namespace DirectionNS {
    using DirectionType = unsigned char;

    // [Caution] DO NOT change the following values
    inline constexpr DirectionType UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3;

    class Direction;

    class DirectionBase {
        DirectionType dir;
        constexpr DirectionBase();
        constexpr explicit DirectionBase(DirectionType dir);

        friend Direction;
    public:
        constexpr bool operator==(const DirectionBase& another) const;
        constexpr bool operator!=(const DirectionBase& another) const;
        constexpr bool operator==(const Direction& another) const;
        constexpr bool operator!=(const Direction& another) const;


        [[nodiscard]] constexpr int dx() const;
        [[nodiscard]] constexpr int dy() const;

        [[nodiscard]] constexpr char to_char() const;

        constexpr void rotate_cw(unsigned int times);
        constexpr void reverse(unsigned int times);
        constexpr void rotate_ccw(unsigned int times);
        constexpr void rotate_cw();
        constexpr void reverse();
        constexpr void rotate_ccw();
        [[nodiscard]] constexpr Direction get_dir_rotated_cw (int times) const;
        [[nodiscard]] constexpr Direction get_dir_reversed   (int times) const;
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw(int times) const;
        [[nodiscard]] constexpr Direction get_dir_rotated_cw () const;
        [[nodiscard]] constexpr Direction get_dir_reversed   () const;
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw() const;

        [[nodiscard]] constexpr bool is_vertical()   const;
        [[nodiscard]] constexpr bool is_horizontal() const;
    };

    inline constexpr DirectionBase::DirectionBase() : dir() {}
    inline constexpr DirectionBase::DirectionBase(DirectionType dir) : dir(dir) {}

    class Direction {
        DirectionBase dir;

        friend DirectionBase;
    public:
        inline static constexpr DirectionBase U{UP};
        inline static constexpr DirectionBase R{RIGHT};
        inline static constexpr DirectionBase D{DOWN};
        inline static constexpr DirectionBase L{LEFT};

        inline static constexpr std::array<DirectionBase, 4> All{U, R, D, L};

        constexpr Direction();
        constexpr Direction(DirectionBase dir);

        constexpr bool operator==(const Direction&     another) const;
        constexpr bool operator==(const DirectionBase& another) const;
        constexpr bool operator!=(const Direction&     another) const;
        constexpr bool operator!=(const DirectionBase& another) const;

        [[nodiscard]] constexpr int dx() const;
        [[nodiscard]] constexpr int dy() const;
        [[nodiscard]] constexpr char to_char() const;

        constexpr void rotate_cw(unsigned int times);
        constexpr void reverse(unsigned int times);
        constexpr void rotate_ccw(unsigned int times);
        constexpr void rotate_cw();
        constexpr void reverse();
        constexpr void rotate_ccw();
        [[nodiscard]] constexpr Direction get_dir_rotated_cw (int times) const;
        [[nodiscard]] constexpr Direction get_dir_reversed   (int times) const;
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw(int times) const;
        [[nodiscard]] constexpr Direction get_dir_rotated_cw()  const;
        [[nodiscard]] constexpr Direction get_dir_reversed()    const;
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw() const;

        [[nodiscard]] constexpr bool is_vertical()   const;
        [[nodiscard]] constexpr bool is_horizontal() const;

        static constexpr Direction from_char(char c);
        static constexpr Direction from_char_or(char c, Direction default_dir);
        static constexpr std::optional<Direction> from_char_safely(char c);
    };

    class Path : public std::vector<Direction> {
    public:
        static Path from_string(const std::string& s);
        static std::pair<Path, std::string> from_string_safely(const std::string& s);
        [[nodiscard]] std::string to_string() const;
    };

    inline constexpr Direction::Direction() : dir() {}
    inline constexpr Direction::Direction(DirectionBase dir) : dir(dir) {}

    inline constexpr bool DirectionBase::operator==(const DirectionBase& another) const { return dir == another.dir; }
    inline constexpr bool DirectionBase::operator!=(const DirectionBase& another) const { return dir != another.dir; }
    inline constexpr bool DirectionBase::operator==(const Direction& another) const { return dir == another.dir.dir; }
    inline constexpr bool DirectionBase::operator!=(const Direction& another) const { return dir != another.dir.dir; }
    inline constexpr bool Direction::operator==(const Direction& another) const { return dir == another.dir; }
    inline constexpr bool Direction::operator==(const DirectionBase& another) const { return dir == another; }
    inline constexpr bool Direction::operator!=(const Direction& another) const { return dir != another.dir; }
    inline constexpr bool Direction::operator!=(const DirectionBase& another) const { return dir != another; }

    inline constexpr int DirectionBase::dx() const { return (dir == RIGHT ? 1 : (dir == LEFT ? -1 : 0)); }
    inline constexpr int DirectionBase::dy() const { return (dir == DOWN  ? 1 : (dir == UP   ? -1 : 0)); }
    inline constexpr int Direction::dx() const { return dir.dx(); }
    inline constexpr int Direction::dy() const { return dir.dy(); }

    inline constexpr char DirectionBase::to_char() const { return (dir == UP ? 'U' : (dir == RIGHT ? 'R' : (dir == DOWN ? 'D' : 'L'))); }
    inline constexpr char Direction::to_char() const { return dir.to_char(); }

    inline constexpr void DirectionBase::rotate_cw (unsigned int times) { dir = (static_cast<unsigned int>(dir) + times  ) & 3; }
    inline constexpr void DirectionBase::reverse   (unsigned int times) { dir = (static_cast<unsigned int>(dir) + times*2) & 3; }
    inline constexpr void DirectionBase::rotate_ccw(unsigned int times) { dir = (static_cast<unsigned int>(dir) + times*3) & 3; }
    inline constexpr void DirectionBase::rotate_cw () { rotate_cw(1);  }
    inline constexpr void DirectionBase::reverse   () { reverse(1);    }
    inline constexpr void DirectionBase::rotate_ccw() { rotate_ccw(1); }
    inline constexpr void Direction::rotate_cw (unsigned int times) { dir.rotate_cw(times);  }
    inline constexpr void Direction::reverse   (unsigned int times) { dir.reverse(times);    }
    inline constexpr void Direction::rotate_ccw(unsigned int times) { dir.rotate_ccw(times); }
    inline constexpr void Direction::rotate_cw () { rotate_cw(1);  }
    inline constexpr void Direction::reverse   () { reverse(1);    }
    inline constexpr void Direction::rotate_ccw() { rotate_ccw(1); }

    inline constexpr Direction DirectionBase::get_dir_rotated_cw (int times) const { return DirectionBase((static_cast<unsigned int>(dir) + times  ) & 3); }
    inline constexpr Direction DirectionBase::get_dir_reversed   (int times) const { return DirectionBase((static_cast<unsigned int>(dir) + times*2) & 3); }
    inline constexpr Direction DirectionBase::get_dir_rotated_ccw(int times) const { return DirectionBase((static_cast<unsigned int>(dir) + times*3) & 3); }
    inline constexpr Direction DirectionBase::get_dir_rotated_cw () const { return get_dir_rotated_cw(1);  }
    inline constexpr Direction DirectionBase::get_dir_reversed   () const { return get_dir_reversed(1);    }
    inline constexpr Direction DirectionBase::get_dir_rotated_ccw() const { return get_dir_rotated_ccw(1); }
    inline constexpr Direction Direction::get_dir_rotated_cw (int times) const { return dir.get_dir_rotated_cw(times);  }
    inline constexpr Direction Direction::get_dir_reversed   (int times) const { return dir.get_dir_reversed(times);    }
    inline constexpr Direction Direction::get_dir_rotated_ccw(int times) const { return dir.get_dir_rotated_ccw(times); }
    inline constexpr Direction Direction::get_dir_rotated_cw () const { return get_dir_rotated_cw(1);  }
    inline constexpr Direction Direction::get_dir_reversed   () const { return get_dir_reversed(1);    }
    inline constexpr Direction Direction::get_dir_rotated_ccw() const { return get_dir_rotated_ccw(1); }

    inline constexpr bool DirectionBase::is_vertical()   const { return (dir == UP)    || (dir == DOWN); }
    inline constexpr bool DirectionBase::is_horizontal() const { return (dir == RIGHT) || (dir == LEFT); }

    inline constexpr bool Direction::is_vertical()   const { return dir.is_vertical();   }
    inline constexpr bool Direction::is_horizontal() const { return dir.is_horizontal(); }

    inline constexpr Direction Direction::from_char(char c) { return (c == 'U' ? U : (c == 'R' ? R : (c == 'D' ? D : L))); }
    inline constexpr Direction Direction::from_char_or(char c, Direction default_dir) { return (c == 'U' ? U : (c == 'R' ? R : (c == 'D' ? D : (c == 'L' ? L : default_dir)))); }
    inline constexpr std::optional<Direction> Direction::from_char_safely(char c) {
        if (c == 'U') return U;
        else if (c == 'R') return R;
        else if (c == 'D') return D;
        else if (c == 'L') return L;
        else return std::nullopt;
    }

    inline Path Path::from_string(const std::string& s) {
        Path result; result.reserve(s.size());
        for(const auto& c : s) result.push_back(Direction::from_char(c));
        return result;
    }
    inline std::pair<Path, std::string> Path::from_string_safely(const std::string& s) {
        Path result; result.reserve(s.size());
        for(int i=0, n=(int)s.size(); i<n; ++i) {
            auto op_dir = Direction::from_char_safely(s[i]);
            if (op_dir) result.push_back(op_dir.value());
            else return {result, s.substr(i,n-i)};
        }
        return {result, ""};
    }
    inline std::string Path::to_string() const {
        std::string result; result.reserve(this->size());
        for(const auto& dir : *this) result.push_back(dir.to_char());
        return result;
    }
}

inline std::ostream& operator << (std::ostream& os, const DirectionNS::DirectionBase& dir) { os << dir.to_char(); return os; }
inline std::ostream& operator << (std::ostream& os, const DirectionNS::Direction& dir) { os << dir.to_char(); return os; }
inline std::ostream& operator << (std::ostream& os, const DirectionNS::Path& path) { os << path.to_string(); return os; }

using DirectionNS::Direction;
using DirectionNS::Path;
