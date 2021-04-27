#pragma once
#include <cmath>
#include <iostream>

template<class T = int>
struct Vec2 {
    T x, y;

    template<class S> inline Vec2& operator =(const Vec2<S>& other) {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }
    template<class S> inline bool operator<(const Vec2<S>& another) const {
        return x == another.x ? y < another.y : x < another.x;
    }
    template<class S> inline bool operator==(const Vec2<S>& another) const {
        return x == another.x && y == another.y;
    }
    template<class S> inline bool operator!=(const Vec2<S>& another) const {
        return x != another.x || y != another.y;
    }
    template<class S> inline Vec2 operator+(const Vec2<S>& other) const {
        return { x + other.x, y + other.y };
    }
    template<class S> inline Vec2& operator +=(const Vec2<S>& other) {
        *this = *this+other;
        return *this;
    }
    template<class S> inline Vec2 operator -(const Vec2<S>& other) const {
        return { x - other.x, y - other.y };
    }
    template<class S> inline Vec2& operator -=(const Vec2<S>& other) {
        *this = *this - other;
        return *this;
    }
    template<class S> inline Vec2 operator *(S s) const {
        return { x*s, y*s };
    }
    template<class S> inline Vec2& operator *=(S s) {
        *this = *this*s;
        return *this;
    }
    template<class S> inline Vec2 operator /(S s) const {
        return { x/s, y/s };
    }
    template<class S> inline Vec2& operator /=(S s) {
        *this = *this/s;
        return *this;
    }

    Vec2() : x(0), y(0) {};
    template<class N, class M> Vec2(N _x, M _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) {}

    void dump() const { std::cout << '(' << x << ", " << y << ')' << std::endl; }
};

template<class T = int>
struct Vec3 {
    union {
        struct { T x,y,z; };
        struct { T r,g,b; };
    };

    template<class S> inline Vec3& operator =(const Vec3<S>& other) {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        return *this;
    }
    template<class S> inline bool operator<(const Vec3<S>& another) const {
        return x == another.x ? (y == another.y ? z < another.z : y < another.y) : x < another.x;
    }
    template<class S> inline bool operator==(const Vec3<S>& another) const {
        return x == another.x && y == another.y && z == another.z;
    }
    template<class S> inline bool operator!=(const Vec3<S>& another) const {
        return x != another.x || y != another.y || z != another.z;
    }
    template<class S> inline Vec3 operator+(const Vec3<S>& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    template<class S> inline Vec3& operator +=(const Vec3<S>& other) {
        *this = *this+other;
        return *this;
    }
    template<class S> inline Vec3 operator -(const Vec3<S>& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    template<class S> inline Vec3& operator -=(const Vec3<S>& other) {
        *this = *this-other;
        return *this;
    }
    template<class S> inline Vec3 operator *(S s) const {
        return { x*s, y*s, z*s };
    }
    template<class S> inline Vec3& operator *=(S s) {
        *this = *this*s;
        return *this;
    }
    template<class S> inline Vec3 operator /(S s) const {
        return { x/s, y/s, z/s };
    }
    template<class S> inline Vec3& operator /=(S s) {
        *this = *this/s;
        return *this;
    }

    Vec3() : x(), y(), z() {}
    template<class N, class M, class L> Vec3(N _x, M _y, L _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) {}

    void dump() { std::cout << '(' << x << ", " << y << ", " << z << ')' << std::endl; }
    double mag() {
        auto X = static_cast<double>(x);
        auto Y = static_cast<double>(y);
        auto Z = static_cast<double>(z);
        return sqrt(X*X+Y*Y+Z*Z);
    }
    template<class S>
    Vec3<T> mul_each_other(Vec3<S>& other) const {
        return { x*static_cast<T>(other.x), y*static_cast<T>(other.y), z*static_cast<T>(other.z) };
    }
    T sum() const { return x + y + z; }
};


using RGB = Vec3<unsigned char>;

template <>
void RGB::dump() {
    std::cout << static_cast<unsigned>(r) << ", " << static_cast<unsigned>(g) << ", " << static_cast<unsigned>(b) << std::endl;
}

template<class T> std::ostream &operator<<(std::ostream &out, const Vec2<T> &v) {
    out << '(' << v.x << ", " << v.y << ')';
    return out;
}
template<class T> std::ostream &operator<<(std::ostream &out, const Vec3<T> &v) {
    out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return out;
}
std::ostream& operator <<(std::ostream& os, const RGB& rgb) {
    os << '(' << static_cast<unsigned>(rgb.r) << ", " << static_cast<unsigned>(rgb.g) << ", " << static_cast<unsigned>(rgb.b)<< ')';
    return os;
}


