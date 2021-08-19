#pragma once
#include <original_positions.hpp>
#include <fstream>
#include <direction.hpp>
#include <utility>
#include <vec2.hpp>

struct SingleProcedure {
    Pos selected_pos;
    Path path;
    SingleProcedure();
    SingleProcedure(Pos selected_pos, Path path);
};

using Procedures = std::vector<SingleProcedure>;

std::ostream& operator << (std::ostream& os, const SingleProcedure& ans);
std::ostream& operator << (std::ostream& os, const Procedures& ans);

