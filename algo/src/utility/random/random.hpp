#pragma once

namespace Random {
    using uint = unsigned int;

    uint seed[4] = {1,2,3,4};

    inline void init_rand_seed(uint s);
    inline uint rand_uint();
    inline double rand_prob();
    inline uint rand_range(uint r);
    inline int rand_range(int l, int r);
    inline uint simple_exp_rand(uint r);
    inline uint exp_rand(uint r, double a);
}
