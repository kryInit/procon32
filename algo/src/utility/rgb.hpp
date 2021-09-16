#pragma once
#include <iostream>
#include <string>
#include <vec3.hpp>

struct RGB {
    unsigned char r, g, b;

    constexpr RGB() : r(), g(), b() {}
    constexpr RGB(int r, int g, int b) : r(r), g(g), b(b) {}
    constexpr RGB(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}

    template<class S> constexpr explicit operator Vec3<S>() const { return Vec3<S>(r, g, b); }

    constexpr bool operator==(const RGB& another) const { return r == another.r && g == another.g && b == another.b; }
    constexpr bool operator!=(const RGB& another) const { return !(*this == another); }

    constexpr RGB operator+(const RGB& another) const { return { r + another.r, g + another.g, b + another.b }; }
    constexpr RGB operator-(const RGB& another) const { return { r - another.r, g - another.g, b - another.b }; }
    constexpr RGB operator*(const RGB& another) const { return { r * another.r, g * another.g, b * another.b }; }
    constexpr RGB operator/(const RGB& another) const { return { r / another.r, g / another.g, b / another.b }; }
    constexpr RGB operator%(const RGB& another) const { return { r % another.r, g % another.g, b % another.b }; }
    constexpr RGB operator+(unsigned char s) const { return { r+s, g+s, b+s }; }
    constexpr RGB operator-(unsigned char s) const { return { r-s, g-s, b-s }; }
    constexpr RGB operator*(unsigned char s) const { return { r*s, g*s, b*s }; }
    constexpr RGB operator/(unsigned char s) const { return { r/s, g/s, b/s }; }
    constexpr RGB operator%(unsigned char s) const { return { r%s, g%s, b%s }; }

    constexpr RGB& operator+=(const RGB& another) { *this = *this+another; return *this; }
    constexpr RGB& operator-=(const RGB& another) { *this = *this-another; return *this; }
    constexpr RGB& operator*=(const RGB& another) { *this = *this*another; return *this; }
    constexpr RGB& operator/=(const RGB& another) { *this = *this/another; return *this; }
    constexpr RGB& operator%=(const RGB& another) { *this = *this%another; return *this; }
    constexpr RGB& operator+=(unsigned char s) { *this = *this+s; return *this; }
    constexpr RGB& operator-=(unsigned char s) { *this = *this-s; return *this; }
    constexpr RGB& operator*=(unsigned char s) { *this = *this*s; return *this; }
    constexpr RGB& operator/=(unsigned char s) { *this = *this/s; return *this; }
    constexpr RGB& operator%=(unsigned char s) { *this = *this%s; return *this; }

    [[nodiscard]] inline constexpr Vec3<double> get_normalized() const {
        return { static_cast<double>(r) / 255., static_cast<double>(g) / 255., static_cast<double>(b) / 255. };
    }

    [[nodiscard]] inline Vec3<int> value() const { return {r, g, b}; }
};

inline std::istream& operator>>(std::istream& is, RGB& c) { is >> c.r >> c.g >> c.b; return is; }
inline std::ostream& operator<<(std::ostream& os, const RGB& c) { os << "\033[48;2;" << (int)c.r << ";" << (int)c.g << ";" << (int)c.b << "m  \033[m"; return os; }

