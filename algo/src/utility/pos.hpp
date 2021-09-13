#pragma once
#include <iostream>
#include <vec2.hpp>

struct Pos : public Vec2<int> {
    constexpr Pos() : Vec2<int>() {}
    constexpr Pos(Vec2<int> p) : Vec2<int>(p) {}
    template<class N, class M> constexpr Pos(N x, M y) : Vec2<int>(x,y) {}
    constexpr explicit Pos(DirectionNS::DirectionBase dir) : Vec2<int>(dir) {}
    constexpr explicit Pos(Direction dir) : Vec2<int>(dir) {}

    constexpr void move(Direction dir) { *this += Pos(dir); }
    constexpr void move(const Pos& dp) { *this += dp; }
    void move(const Path& path) { for(const auto& dir : path) move(dir); }
    constexpr void move_toraly(Direction dir, const Pos& area) {
        move(dir);
        x = (x == -1 ? area.x-1 : (x==area.x ? 0 : x));
        y = (y == -1 ? area.y-1 : (y==area.y ? 0 : y));
    }
    constexpr void move_toraly(const Pos& dp, const Pos& area) {
        *this += (dp%area);
        x = (x + area.x) % area.x;
        y = (y + area.y) % area.y;
    }
    void move_toraly(const Path& path, const Pos& area) { for(const auto& dir : path) move_toraly(dir, area); }

    [[nodiscard]] constexpr Pos get_moved_pos(Direction dir)    const { Pos p = *this; p.move(dir ); return p; }
    [[nodiscard]] constexpr Pos get_moved_pos(const Pos& dp)    const { Pos p = *this; p.move(dp); return p; }
    [[nodiscard]] constexpr Pos get_moved_toraly_pos(Direction dir,    const Pos& area) const { Pos p = *this; p.move_toraly(dir,  area); return p; }
    [[nodiscard]] constexpr Pos get_moved_toraly_pos(const Pos& dp,    const Pos& area) const { Pos p = *this; p.move_toraly(dp,   area); return p; }
    [[nodiscard]] Pos get_moved_pos(const Path& path) const { Pos p = *this; p.move(path); return p; }
    [[nodiscard]] Pos get_moved_toraly_pos(const Path& path, const Pos& area) const { Pos p = *this; p.move_toraly(path, area); return p; }

    [[nodiscard]] int calc_mh_dist_toraly(const Pos& other, const Pos& area) const { return std::min(area.x-abs(x-other.x), abs(x-other.x)) + std::min(area.y-abs(y-other.y), abs(y-other.y)); }

    [[nodiscard]] constexpr int to_idx(int w) const { return y * w + x; }
    [[nodiscard]] constexpr int to_idx(const Pos& shape) const { return y * shape.x + x; }
    static constexpr Pos from_idx(int idx, int w) { return {idx%w, idx/w}; }
    static constexpr Pos from_idx(int idx, const Pos& shape) { return {idx%shape.x, idx/shape.x}; }
};
