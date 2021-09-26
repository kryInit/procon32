#pragma once
#include <state.hpp>
#include <optional>

namespace RoughSorter {
    std::optional<Pos> get_max_penalty_pos(const State& state);

    int calc_penalty_sum(const State& state);
    double parallelized_sort_roughly(State& state, int loose_time_limit = 5000, int strict_time_limit = 5000, int depth = 9);
    double sort_roughly(State& state, int loose_time_limit = 5000, int strict_time_limit = 5000, int depth = 9);
    double sort_roughly_greedily(State& state, int depth = 9);
};

