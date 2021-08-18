#include <random.hpp>
#include <cmath>
#include <climits>

using uint = unsigned int;

uint seed[4] = {1,2,3,4};

void Random::set_random_state(uint s) {
    const uint magic_number = 1812433253;
    for(int i=1; i<=4; i++){
        seed[i-1] = s = magic_number * (s^(s>>30U)) + (uint)i;
    }
}

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
    return static_cast<int>(rand_uint()) % (r-l) + l;
}

uint Random::simple_exp_rand(uint r) {
    return static_cast<uint>((pow(2, rand_prob())-1)*r);
}
uint Random::exp_rand(uint r, double a) {
    return static_cast<uint>(pow(pow(2, rand_prob())-1, a) * r);
}