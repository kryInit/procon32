#pragma once
#include <vec.hpp>
#include <vector>
#include <ostream>

struct Answer {
    std::vector<unsigned int> rotations;
    std::vector<std::vector<Vec2<unsigned int>>> orig_idx;

    void dump(std::ostream& os);
    void dump();

    Answer();
};
