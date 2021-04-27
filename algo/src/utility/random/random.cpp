#include "random.hpp"
#include <cmath>
#include <climits>

uint Random::rand_uint() {
    uint t = (seed[0]^(seed[0]<<11U));
    seed[0]=seed[1];
    seed[1]=seed[2];
    seed[2]=seed[3];
    return seed[3]=(seed[3]^(seed[3]>>19U))^(t^(t>>8U));
}

double Random::rand_prob() {
    return static_cast<double>(rand_uint()) / static_cast<double>(UINT_MAX+1);
}

uint Random::rand_range(uint r) {
    return rand_uint()%r;
}

int Random::rand_range(int l, int r) {
    return rand_uint() % (r-l) + l;
}

uint Random::simple_exp_rand(uint r) {
    return (pow(2, rand_prob())-1)*r;
}
uint Random::exp_rand(uint r, double a) {
    return pow(pow(2, rand_prob())-1, a) * r;
}