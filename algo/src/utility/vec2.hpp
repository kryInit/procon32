#pragma once
#include <array>
#include <cmath>
#include <iostream>
#include <direction.hpp>

template<class T = int> struct Vec2;
using Pos = Vec2<int>;

template<class T> struct Vec2 {
    T x, y;


    constexpr Vec2() : x(0), y(0) {};
    constexpr Vec2(T x, T y) : x(x), y(y) {}
    constexpr explicit Vec2(Direction dir) : x(static_cast<T>(dir.dx())), y(static_cast<T>(dir.dy())) {}
    template<class N, class M> constexpr Vec2(N x, M y) : x(static_cast<T>(x)), y(static_cast<T>(y)) {}

    constexpr bool operator<(const Vec2<T>& another) const {
        return x == another.x ? y < another.y : x < another.x;
    }
    constexpr bool operator>(const Vec2<T>& another) const {
        return another < *this;
    }
    constexpr bool operator<=(const Vec2<T>& another) const {
        return !(*this > another);
    }
    constexpr bool operator>=(const Vec2<T>& another) const {
        return !(*this < another);
    }

    constexpr bool operator==(const Vec2<T>& another) const {
        return x == another.x && y == another.y;
    }
    constexpr bool operator!=(const Vec2<T>& another) const {
        return !(*this == another);
    }
    constexpr Vec2 operator+(const Vec2<T>& other) const {
        return { x + other.x, y + other.y };
    }
    constexpr Vec2& operator +=(const Vec2<T>& other) {
        *this = *this+other;
        return *this;
    }
    constexpr Vec2 operator -(const Vec2<T>& other) const {
        return { x - other.x, y - other.y };
    }
    constexpr Vec2& operator -=(const Vec2<T>& other) {
        *this = *this - other;
        return *this;
    }
    constexpr Vec2 operator *(T s) const {
        return { x*s, y*s };
    }
    constexpr Vec2& operator *=(T s) {
        *this = *this*s;
        return *this;
    }
    constexpr Vec2 operator /(T s) const {
        return { x/s, y/s };
    }
    constexpr Vec2& operator /=(T s) {
        *this = *this/s;
        return *this;
    }

    template<class S> constexpr Vec2 operator *(const Vec2<S>& v) const {
        return { x*v.x, y*v.y };
    }
    template<class S> constexpr Vec2& operator *=(const Vec2<S>& v) {
        *this = *this*v;
        return *this;
    }

    template<class S> constexpr explicit operator Vec2<S>() const {
        return Vec2<S>(x, y);
    }

    [[nodiscard]] constexpr double norm() const {
        const auto X = static_cast<double>(x), Y = static_cast<double>(y);
        return sqrt(X*X + Y*Y);
    }
    [[nodiscard]] constexpr T sum() const { return x + y; }
    [[nodiscard]] constexpr T dot(const Vec2<T>& other) const { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr T cross(const Vec2<T>& other) const { return x * other.y - y * other.x; }
    constexpr static double norm(const Vec2<T>& v) { return v.norm(); }
    constexpr static T sum(const Vec2<T>& v) { return v.sum(); }
    constexpr static T dot(const Vec2<T>& v1, const Vec2<T>& v2) { return v1.dot(v2); }
    constexpr static T cross(const Vec2<T>& v1, const Vec2<T>& v2) { return v1.cross(v2); }

    void move(Direction dir) { *this += Pos(dir); }
    void move(const Path& path) { for(const auto& dir : path) move(dir); }
    static void move(Pos& p, Direction dir) { p.move(dir); }
    static void move(Pos& p, const Path& path) { p.move(path); }

    [[nodiscard]] bool is_safe_idx(Pos limit) const { return 0 <= x && x < limit.x && 0 <= y && y < limit.y; }
    static bool is_safe_idx(Pos p, Pos limit) { return p.is_safe_idx(limit); }

    [[nodiscard]] int pos2idx(int w) const { return y * w + x; }
    static int pos2idx(Pos p, int w) { return p.pos2idx(w); }
    static Pos idx2pos(int idx, int w) { return {idx%w, idx/w}; }

    static constexpr std::array<Pos, 4> get_neighborhood4() {
        return { Pos(Direction::U), Pos(Direction::R), Pos(Direction::D), Pos(Direction::L) };
    }
    static constexpr std::array<Pos, 8> get_neighborhood8() {
        return { Pos(Direction::U), Pos(Direction::U) + Pos(Direction::R), Pos(Direction::R), Pos(Direction::R) + Pos(Direction::D), Pos(Direction::D), Pos(Direction::D) + Pos(Direction::L), Pos(Direction::L), Pos(Direction::L) + Pos(Direction::U) };
    }
};

template<class T> std::istream& operator >> (std::istream& is, Vec2<T>& v) {
    is >> v.x >> v.y;
    return is;
}
template<class T> std::ostream& operator << (std::ostream& os, const Vec2<T>& v) {
    os << v.x << ' ' << v.y;
    return os;
}
