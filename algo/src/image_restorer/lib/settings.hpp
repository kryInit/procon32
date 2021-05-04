#pragma once
#include "vec.hpp"
#include <string>

class Settings {
    unsigned int frag_size{};
    Vec2<unsigned int> div_num;
    unsigned int selectable_times{};
    unsigned int choice_cost{}, repl_cost{};

public:
    void load(const std::string& path);
    void dump();
    [[nodiscard]] const unsigned int& FRAG_SIZE() const { return frag_size; }
    [[nodiscard]] const Vec2<unsigned int>& DIV_NUM() const { return div_num; }
    [[nodiscard]] const unsigned int& SELECTABLE_TIMES() const { return selectable_times; }
    [[nodiscard]] const unsigned int& CHOICE_COST() const { return choice_cost; }
    [[nodiscard]] const unsigned int& REPL_COST() const { return repl_cost; }

};

