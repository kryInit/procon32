#pragma once
#include <image_restorer_lib.hpp>

class ISolver {
public:
    virtual Answer solve(double *adjacency, const Settings& settings) = 0;
};
