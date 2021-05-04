#pragma once
#include "interface.hpp"

class SideBeamSearchSolver : ISolver {
public:
    Answer solve(double *adjacency, const Settings& settings) override;
};