#pragma once
#include <state.hpp>
#include <array>
#include <tuple>

namespace StrictSorter {
    inline std::array<Procedures, 4096> best_procs_for_2x2{};

    [[nodiscard]] int calc_cost(const State& state);

    void init();

    void sort_start2finish(const Pos& first_selected_pos, const Pos& first_target, State& state);

    void sort_partially(const Pos& first_selected_pos, const Pos& UL, const Pos& dp, State& state);

    bool sort_by_roughly_sort(State& state);
};

