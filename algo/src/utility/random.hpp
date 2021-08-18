#pragma once

namespace Random {
    using uint = unsigned int;

    void set_random_state(uint s);
    uint rand_uint();
    double rand_prob();
    uint rand_range(uint r);
    int rand_range(int l, int r);
    uint simple_exp_rand(uint r);
    uint exp_rand(uint r, double a);
}
