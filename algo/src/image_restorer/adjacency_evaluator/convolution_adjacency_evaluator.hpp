#pragma once
#include "interface.hpp"

class ConvolutionAdjacencyEvaluator : IAdjacencyEvaluator {
public:
    void get_adjacency(const Image& img, const Settings& settings, double *adjacency) override;
};