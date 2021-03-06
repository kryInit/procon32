#pragma once
#include "interface.hpp"

class SideBeamSearchSolver : ISolver {
public:
    Answer operator()(double *adjacency, const Settings& settings, int argc, char *argv[]) override;
};