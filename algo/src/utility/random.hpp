#pragma once

#include <cmath>
#include <climits>

namespace Random {
    using uint = unsigned int;

    inline uint seed[4] = {1,2,3,4};

    inline void set_random_state(uint s);
    inline uint rand_uint();
    inline double rand_prob();
    inline uint rand_range(uint r);
    inline int rand_range(int l, int r);
    inline double simple_exp_rand();
    inline uint simple_exp_rand(uint r);
    inline double exp_rand(double a);
    inline uint exp_rand(uint r, double a);
    inline bool rand_bool();
}

inline void Random::set_random_state(uint s) {
    const uint magic_number = 1812433253;
    for(int i=1; i<=4; i++){
        seed[i-1] = s = magic_number * (s^(s>>30U)) + (uint)i;
    }
}
inline Random::uint Random::rand_uint() {
    uint t = (seed[0]^(seed[0]<<11U));
    seed[0]=seed[1];
    seed[1]=seed[2];
    seed[2]=seed[3];
    return seed[3]=(seed[3]^(seed[3]>>19U))^(t^(t>>8U));
}

inline double Random::rand_prob() { return (double)rand_uint() / ((double)UINT_MAX+1.); }
inline Random::uint Random::rand_range(uint r) { return rand_uint()%r; }
inline int Random::rand_range(int l, int r) { return static_cast<int>(rand_uint() % static_cast<uint>(r-l)) + l; }
inline double Random::simple_exp_rand() { return pow(2, rand_prob())-1; }
inline Random::uint Random::simple_exp_rand(uint r) { return static_cast<Random::uint>(simple_exp_rand()*r); }
inline double Random::exp_rand(double a) { return pow(simple_exp_rand(), a); }
inline Random::uint Random::exp_rand(uint r, double a) { return static_cast<Random::uint>(exp_rand(a)*r); }
inline bool Random::rand_bool() { return rand_uint()&1; }
