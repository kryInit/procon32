#pragma once
#include <fstream>
#include <direction.hpp>
#include <pos.hpp>

struct SingleProcedure {
    Pos selected_pos;
    Path path;
    SingleProcedure();
    SingleProcedure(Pos selected_pos, Path path);
};

using Procedures = std::vector<SingleProcedure>;

Procedures input_procedure(const std::string& path);

std::ostream& operator << (std::ostream& os, const SingleProcedure& ans);
std::ostream& operator << (std::ostream& os, const Procedures& ans);

