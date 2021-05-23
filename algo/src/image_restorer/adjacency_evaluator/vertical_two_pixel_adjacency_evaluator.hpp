#pragma once
#include "interface.hpp"

class VerticalTwoPixelAdjacencyEvaluator : IAdjacencyEvaluator {
public:
    void get_adjacency(const Image& img, const Settings& settings, double *adjacency) override;
};
