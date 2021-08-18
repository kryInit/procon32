#pragma once
#include "interface.hpp"

class OnePixelAdjacencyEvaluator : IAdjacencyEvaluator {
public:
    void operator()(const Image& img, const Settings& settings, double *adjacency) override;
};