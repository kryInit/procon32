#pragma once
#include <settings.hpp>
#include <original_positions.hpp>
#include <procedures.hpp>

class ISolver {
public:
    virtual Procedures operator()(const OriginalPositions& original_positions, const Settings& settings) = 0;
};