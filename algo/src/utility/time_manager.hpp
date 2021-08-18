#pragma once
#include <timing_device.hpp>

class TimeManager : public TimingDevice {
    size_t ms_time_limit;
public:
    explicit TimeManager(size_t ms_time_limit);
    [[nodiscard]] bool in_time_limit() const;
    [[nodiscard]] double progress() const;
};
