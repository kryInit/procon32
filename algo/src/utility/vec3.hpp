#pragma once
#include <cmath>
#include <iostream>

template<class T = int> struct Vec3;
using RGB = Vec3<unsigned char>;

template<class T> struct Vec3 {
    union {
        struct { T x,y,z; };
        struct { T r,g,b; };
    };

    constexpr Vec3() : x(0), y(0), z(0) {};
    constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    template<class N, class M, class L> constexpr Vec3(N x, M y, L z) : x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)) {}

    constexpr bool operator<(const Vec3<T>& another) const {
        return x == another.x ? (y == another.y ? z < another.z : y < another.y) : x < another.x;
    }
    constexpr bool operator>(const Vec3<T>& another) const {
        return another < *this;
    }
    constexpr bool operator<=(const Vec3<T>& another) const {
        return !(*this > another);
    }
    constexpr bool operator>=(const Vec3<T>& another) const {
        return !(*this < another);
    }

    constexpr bool operator==(const Vec3<T>& another) const {
        return x == another.x && y == another.y && z == another.z;
    }
    constexpr bool operator!=(const Vec3<T>& another) const {
        return !(*this == another);
    }
    constexpr Vec3 operator+(const Vec3<T>& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    constexpr Vec3& operator +=(const Vec3<T>& other) {
        *this = *this+other;
        return *this;
    }
    constexpr Vec3 operator -(const Vec3<T>& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    constexpr Vec3& operator -=(const Vec3<T>& other) {
        *this = *this - other;
        return *this;
    }
    constexpr Vec3 operator *(T s) const {
        return { x*s, y*s, z*s };
    }
    constexpr Vec3& operator *=(T s) {
        *this = *this*s;
        return *this;
    }
    constexpr Vec3 operator /(T s) const {
        return { x/s, y/s, z/s };
    }
    constexpr Vec3& operator /=(T s) {
        *this = *this/s;
        return *this;
    }

    template<class S> constexpr Vec3 operator *(const Vec3<S>& v) const {
        return { x*v.x, y*v.y, z*v.z };
    }
    template<class S> constexpr Vec3& operator *=(const Vec3<S>& v) {
        *this = *this*v;
        return *this;
    }

    template<class S> constexpr explicit operator Vec3<S>() const {
        return Vec3<S>(x, y, z);
    }

    [[nodiscard]] constexpr double norm() const {
        const auto X = static_cast<double>(x), Y = static_cast<double>(y), Z = static_cast<double>(z);
        return sqrt(X*X + Y*Y + Z*Z);
    }
    [[nodiscard]] constexpr T sum() const { return x + y + z; }
    [[nodiscard]] constexpr T dot(const Vec3<T>& other) const { return x * other.x + y * other.y + z * other.z; }
    [[nodiscard]] constexpr Vec3<T> cross(const Vec3<T>& other) const { return { y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x }; }
    constexpr static double norm(const Vec3<T>& v) { return v.norm(); }
    constexpr static T sum(const Vec3<T>& v) { return v.sum(); }
    constexpr static T dot(const Vec3<T>& v1, const Vec3<T>& v2) { return v1.dot(v2); }
    constexpr static T cross(const Vec3<T>& v1, const Vec3<T>& v2) { return v1.cross(v2); }

    constexpr static Vec3<double> normalize(const RGB& rgb) {
        return { static_cast<double>(rgb.r) / 255., static_cast<double>(rgb.g) / 255., static_cast<double>(rgb.b) / 255. };
    }
};

template<class T> inline std::istream& operator >> (std::istream& is, Vec3<T>& v) {
    is >> v.x >> v.y >> v.z;
    return is;
}

template<class T> inline std::ostream& operator << (std::ostream& os, const Vec3<T> v) {
    os << v.x << ' ' << v.y << ' ' << v.z;
    return os;
}

template<> inline std::ostream& operator << (std::ostream& os, const RGB v) {
    os << static_cast<unsigned>(v.x) << ' ' << static_cast<unsigned>(v.y) << ' ' << static_cast<unsigned>(v.z);
    return os;
}


