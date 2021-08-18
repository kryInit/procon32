#pragma once

#include <string>
#include <vec2.hpp>

class Settings {
    unsigned int frag_size{};
    Vec2<unsigned int> div_num;
    unsigned int selectable_times{};
    unsigned int choice_cost{}, swap_cost{};

public:
    void load(const std::string& path);
    void dump() const;
    [[nodiscard]] unsigned int FRAG_SIZE() const;
    [[nodiscard]] Vec2<unsigned int> DIV_NUM() const;
    [[nodiscard]] unsigned int SELECTABLE_TIMES() const;
    [[nodiscard]] unsigned int CHOICE_COST() const;
    [[nodiscard]] unsigned int SWAP_COST() const;
};