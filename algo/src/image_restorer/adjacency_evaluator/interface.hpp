#pragma once
#include <image.hpp>

class IAdjacencyEvaluator {
public:
    virtual void operator()(const Image& img, const Settings& settings, double *adjacency) = 0;
};