#include <simulated_annealing_manager.hpp>

double SimulatedAnnealingManager::temp() const {return start_temp + (end_temp - start_temp) * progress(); }
double SimulatedAnnealingManager::prob(double score_diff) const { return (score_diff >= 0 ? 1 : exp(score_diff / temp())); }

SimulatedAnnealingManager::SimulatedAnnealingManager(size_t ms_time_limit, double start_temp, double end_temp) : TimeManager(ms_time_limit), start_temp(start_temp), end_temp(end_temp) {}
bool SimulatedAnnealingManager::permit_transition(double score_diff) const { return prob(score_diff) >= Random::rand_prob(); }
