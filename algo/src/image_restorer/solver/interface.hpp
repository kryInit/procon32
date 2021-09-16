#pragma once
#include <answer.hpp>
#include <settings.hpp>

class ISolver {
public:
    virtual Answer operator()(double *adjacency, const Settings& settings, int argc, char *argv[]) = 0;
};