#include <iostream>
#include "time_manager.hpp"

timeManager::timeManager(unsigned long ms_time_limit) : MS_TIME_LIMIT(ms_time_limit) {
    start();
}

bool timeManager::is_within_time_limit() {
    return get_ms() < MS_TIME_LIMIT;
}