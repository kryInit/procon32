#pragma once

#include <vec2.hpp>

struct Settings {
    int frag_size;
    Vec2<int> div_num;
    int selectable_times;
    int choice_cost, swap_cost;
    Settings() = delete;
    explicit Settings(const std::string& path);
    void load(const std::string& path);
    void dump() const;
};