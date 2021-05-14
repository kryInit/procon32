#include "simulated_annealing.hpp"
#include <random.hpp>
#include <cmath>

bool SimulatedAnnealing::permit_continuation() {
    return tm.is_within_time_limit();
}

double SimulatedAnnealing::progress() {
    return (double)tm.get_ms() / (double)ms_time_limit;
}

double SimulatedAnnealing::temperature() {
    return START_TEMP + (END_TEMP - START_TEMP) * progress();
}

double SimulatedAnnealing::probability(double diff, double t) const {
    if (diff <= 0) return 1;
    else return exp(-diff/t);
}

bool SimulatedAnnealing::permit_transition(double diff) {
    return probability(diff, temperature()) > Random::rand_prob();
}