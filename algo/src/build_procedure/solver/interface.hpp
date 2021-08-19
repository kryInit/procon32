#pragma once
#include <settings.hpp>
#include <procedure.hpp>

class ISolver {
public:
    virtual Procedures operator()(const OriginalPositions& original_positions, const Settings& settings) = 0;
};