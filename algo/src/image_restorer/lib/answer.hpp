#pragma once
#include <vec2.hpp>
#include <vector>
#include <ostream>

struct Answer {
    std::vector<unsigned int> rotations;
    std::vector<std::vector<Vec2<unsigned int>>> orig_idx;

    Answer();
};

std::ostream& operator << (std::ostream& os, const Answer& ans);
