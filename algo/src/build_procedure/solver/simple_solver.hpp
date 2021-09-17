#pragma once
#include "interface.hpp"

class SimpleSolver : ISolver {
public:
    Procedures operator()(const OriginalPositions& original_positions, const Settings& settings, int argc, char *argv[]) override;
};
