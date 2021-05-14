#pragma once
#include <time_manager.hpp>

class SimulatedAnnealing {
    const unsigned int ms_time_limit;
    const double START_TEMP;
    const double END_TEMP;
    timeManager tm;
    [[nodiscard]] double temperature();
    [[nodiscard]] double probability(double diff, double t) const;

protected:
    double progress();
    bool permit_transition(double diff);
    bool permit_continuation();

public:
    struct Parameters {
        unsigned int ms_time_limit;
        double START_TEMP;
        double END_TEMP;

        Parameters() : ms_time_limit(0), START_TEMP(1), END_TEMP(1) {}
    };
    explicit SimulatedAnnealing(Parameters params)
            : ms_time_limit(params.ms_time_limit)
            , START_TEMP(params.START_TEMP)
            , END_TEMP(params.END_TEMP)
            , tm(params.ms_time_limit) {}
};
