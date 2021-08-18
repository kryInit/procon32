#include <time_manager.hpp>

TimeManager::TimeManager(size_t ms_time_limit) : TimingDevice(), ms_time_limit(ms_time_limit) {}
bool TimeManager::in_time_limit() const {
    return get_ms() < ms_time_limit;
}
double TimeManager::progress() const {
    return static_cast<double>(get_ms()) / static_cast<double>(ms_time_limit);
}
