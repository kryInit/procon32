#pragma once
#include "stopwatch.hpp"

class timeManager : public stopwatch {
    const unsigned long MS_TIME_LIMIT;
public:
    explicit timeManager(unsigned long ms_time_limit);
    bool is_within_time_limit();
};
