#pragma once
#include <timing_device.hpp>

class TimeManager : public TimingDevice {
    size_t ms_time_limit;
public:
    explicit TimeManager(size_t ms_time_limit);
    [[nodiscard]] bool in_time_limit() const;
    [[nodiscard]] double progress() const;
};

inline TimeManager::TimeManager(size_t ms_time_limit) : TimingDevice(), ms_time_limit(ms_time_limit) {}
inline bool TimeManager::in_time_limit() const { return get_ms() < ms_time_limit; }
inline double TimeManager::progress() const { return static_cast<double>(get_ms()) / static_cast<double>(ms_time_limit); }
