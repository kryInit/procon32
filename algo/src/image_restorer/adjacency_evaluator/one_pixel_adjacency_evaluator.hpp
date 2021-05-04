#pragma once
#include "interface.hpp"

class OnePixelAdjacencyEvaluator : IAdjacencyEvaluator {
public:
    void get_adjacency(const Image& img, const Settings& settings, double *adjacency) override;
};
