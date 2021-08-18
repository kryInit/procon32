#pragma once
#include <cmath>
#include <random.hpp>
#include <time_manager.hpp>

class SimulatedAnnealingManager : public TimeManager {
    double start_temp, end_temp;

    [[nodiscard]] double temp() const;
    [[nodiscard]] double prob(double score_diff) const;

public:
    SimulatedAnnealingManager(size_t ms_time_limit, double start_temp, double end_temp);
    [[nodiscard]] bool permit_transition(double score_diff) const;
};
