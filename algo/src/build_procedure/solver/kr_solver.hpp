#pragma once
#include "interface.hpp"

class KrSolver : ISolver {
public:
    Procedures operator()(const OriginalPositions& original_positions, const Settings& settings, int argc, char *argv[]) override;
};
