#pragma once
#include <image_restorer_lib.hpp>

class IAdjacencyEvaluator {
public:
    virtual void get_adjacency(const Image& img, const Settings& settings, double *adjacency) = 0;
};
