#pragma once
#include <cmath>
#include <iostream>
#include <direction.hpp>

template<class T = double> struct Vec2 {
    T x, y;

    constexpr Vec2() : x(), y() {}
    constexpr Vec2(T x, T y) : x(x), y(y) {}
    constexpr explicit Vec2(DirectionNS::DirectionBase dir) : x(dir.dx()), y(dir.dy()) {}
    constexpr explicit Vec2(Direction dir) : x(dir.dx()), y(dir.dy()) {}

    template<class S> constexpr explicit operator Vec2<S>() const { return Vec2<S>(x, y); }

    constexpr bool operator< (const Vec2<T>& another) const { return x == another.x ? y < another.y : x < another.x; }
    constexpr bool operator> (const Vec2<T>& another) const { return another < *this; }
    constexpr bool operator<=(const Vec2<T>& another) const { return !(*this > another); }
    constexpr bool operator>=(const Vec2<T>& another) const { return !(*this < another); }
    constexpr bool operator==(const Vec2<T>& another) const { return x == another.x && y == another.y; }
    constexpr bool operator!=(const Vec2<T>& another) const { return !(*this == another); }

    constexpr Vec2<T> operator+(const Vec2<T>& another) const { return { x + another.x, y + another.y }; }
    constexpr Vec2<T> operator-(const Vec2<T>& another) const { return { x - another.x, y - another.y }; }
    constexpr Vec2<T> operator*(const Vec2<T>& another) const { return { x * another.x, y * another.y }; }
    constexpr Vec2<T> operator/(const Vec2<T>& another) const { return { x / another.x, y / another.y }; }
    constexpr Vec2<T> operator%(const Vec2<T>& another) const { return { x % another.x, y % another.y }; }
    constexpr Vec2<T> operator+(T s) const { return { x+s, y+s }; }
    constexpr Vec2<T> operator-(T s) const { return { x-s, y-s }; }
    constexpr Vec2<T> operator*(T s) const { return { x*s, y*s }; }
    constexpr Vec2<T> operator/(T s) const { return { x/s, y/s }; }
    constexpr Vec2<T> operator%(T s) const { return { x%s, y%s }; }

    constexpr Vec2<T>& operator+=(const Vec2<T>& another) { *this = *this+another; return *this; }
    constexpr Vec2<T>& operator-=(const Vec2<T>& another) { *this = *this-another; return *this; }
    constexpr Vec2<T>& operator*=(const Vec2<T>& another) { *this = *this*another; return *this; }
    constexpr Vec2<T>& operator/=(const Vec2<T>& another) { *this = *this/another; return *this; }
    constexpr Vec2<T>& operator%=(const Vec2<T>& another) { *this = *this%another; return *this; }
    constexpr Vec2<T>& operator+=(T s) { *this = *this+s; return *this; }
    constexpr Vec2<T>& operator-=(T s) { *this = *this-s; return *this; }
    constexpr Vec2<T>& operator*=(T s) { *this = *this*s; return *this; }
    constexpr Vec2<T>& operator/=(T s) { *this = *this/s; return *this; }
    constexpr Vec2<T>& operator%=(T s) { *this = *this%s; return *this; }

    [[nodiscard]] constexpr T dot  (const Vec2<T>& another) const { return x*another.x + y*another.y; }
    [[nodiscard]] constexpr T cross(const Vec2<T>& another) const { return x*another.y - y*another.x; }

    [[nodiscard]] constexpr T square() const { return this->dot(*this); }

    [[nodiscard]] constexpr T sum() const { return x+y; }
    [[nodiscard]] constexpr double norm() const { return sqrt(square()); }

    [[nodiscard]] T calc_mh_dist(const Vec2<T>& other) const { return abs(x-other.x) + abs(y-other.y); }
    [[nodiscard]] double calc_ec_dist(const Vec2<T>& other) const { return (*this - other).norm(); }
    [[nodiscard]] constexpr bool in_area(const Vec2<T>& area) const { return 0 <= x && x < area.x && 0 <= y && y < area.y; }

    [[nodiscard]] constexpr Vec2<T> xx() const { return {x, x}; }
    [[nodiscard]] constexpr Vec2<T> yx() const { return {y, x}; }
    [[nodiscard]] constexpr Vec2<T> yy() const { return {y, y}; }
};

template<class T> std::istream& operator>>(std::istream& is, Vec2<T>& v) { is >> v.x >> v.y; return is; }
template<class T> std::ostream& operator<<(std::ostream& os, const Vec2<T>& v) { os << v.x << ' ' << v.y; return os; }

