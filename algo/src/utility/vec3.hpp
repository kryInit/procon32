#pragma once
#include <cmath>
#include <iostream>

template<class T = double> struct Vec3 {
    T x, y, z;

    constexpr Vec3() : x(), y(), z() {}
    constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    template<class S> constexpr explicit operator Vec3<S>() const { return Vec3<S>(x, y, z); }

    constexpr bool operator< (const Vec3<T>& another) const { return x == another.x ? (y == another.y ? z < another.z : y < another.y) : x < another.x; }
    constexpr bool operator> (const Vec3<T>& another) const { return another < *this; }
    constexpr bool operator<=(const Vec3<T>& another) const { return !(*this > another); }
    constexpr bool operator>=(const Vec3<T>& another) const { return !(*this < another); }
    constexpr bool operator==(const Vec3<T>& another) const { return x == another.x && y == another.y && z == another.z; }
    constexpr bool operator!=(const Vec3<T>& another) const { return !(*this == another); }

    constexpr Vec3<T> operator+(const Vec3<T>& another) const { return { x + another.x, y + another.y, z + another.z }; }
    constexpr Vec3<T> operator-(const Vec3<T>& another) const { return { x - another.x, y - another.y, z - another.z }; }
    constexpr Vec3<T> operator*(const Vec3<T>& another) const { return { x * another.x, y * another.y, z * another.z }; }
    constexpr Vec3<T> operator/(const Vec3<T>& another) const { return { x / another.x, y / another.y, z / another.z }; }
    constexpr Vec3<T> operator%(const Vec3<T>& another) const { return { x % another.x, y % another.y, z % another.z }; }
    constexpr Vec3<T> operator+(T s) const { return { x+s, y+s, z+s }; }
    constexpr Vec3<T> operator-(T s) const { return { x-s, y-s, z-s }; }
    constexpr Vec3<T> operator*(T s) const { return { x*s, y*s, z*s }; }
    constexpr Vec3<T> operator/(T s) const { return { x/s, y/s, z/s }; }
    constexpr Vec3<T> operator%(T s) const { return { x%s, y%s, z%s }; }

    constexpr Vec3<T>& operator+=(const Vec3<T>& another) { *this = *this+another; return *this; }
    constexpr Vec3<T>& operator-=(const Vec3<T>& another) { *this = *this-another; return *this; }
    constexpr Vec3<T>& operator*=(const Vec3<T>& another) { *this = *this*another; return *this; }
    constexpr Vec3<T>& operator/=(const Vec3<T>& another) { *this = *this/another; return *this; }
    constexpr Vec3<T>& operator%=(const Vec3<T>& another) { *this = *this%another; return *this; }
    constexpr Vec3<T>& operator+=(T s) { *this = *this+s; return *this; }
    constexpr Vec3<T>& operator-=(T s) { *this = *this-s; return *this; }
    constexpr Vec3<T>& operator*=(T s) { *this = *this*s; return *this; }
    constexpr Vec3<T>& operator/=(T s) { *this = *this/s; return *this; }
    constexpr Vec3<T>& operator%=(T s) { *this = *this%s; return *this; }

    [[nodiscard]] constexpr T dot  (const Vec3<T>& another) const { return x*another.x + y*another.y + z*another.z; }
    [[nodiscard]] constexpr Vec3<T> cross(const Vec3<T>& other) const { return { y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x }; }

    [[nodiscard]] constexpr T square() const { return this->dot(*this); }

    [[nodiscard]] constexpr T sum() const { return x+y+z; }
    [[nodiscard]] constexpr double norm() const { return sqrt(square()); }

    [[nodiscard]] T calc_mh_dist(const Vec3<T>& other) const { return abs(x-other.x) + abs(y-other.y) + abs(z-other.z); }
    [[nodiscard]] double calc_ec_dist(const Vec3<T>& other) const { return (*this - other).norm(); }
    [[nodiscard]] constexpr bool in_area(const Vec3<T>& area) const { return 0 <= x && x < area.x && 0 <= y && y < area.y && 0 <= z && z < area.z; }
};

template<class T> std::istream& operator>>(std::istream& is, Vec3<T>& v) { is >> v.x >> v.y >> v.z; return is; }
template<class T> std::ostream& operator<<(std::ostream& os, const Vec3<T>& v) { os << v.x << ' ' << v.y << ' ' << v.z; return os; }


