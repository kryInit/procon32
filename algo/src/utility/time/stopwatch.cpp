#include <iostream>
#include "stopwatch.hpp"

stopwatch::stopwatch() : m_start(std::chrono::system_clock::time_point::min()) {}

void stopwatch::clear() {
    m_start = std::chrono::system_clock::time_point::min();
}

bool stopwatch::is_started() {
    return (m_start.time_since_epoch() != std::chrono::system_clock::time_point::min().time_since_epoch());
}

void stopwatch::start() {
    m_start = std::chrono::system_clock::now();
}

unsigned long stopwatch::get_sec() {
    if (is_started()) {
        std::chrono::system_clock::duration  diff;
        diff = std::chrono::system_clock::now() - m_start;
        return (unsigned)(std::chrono::duration_cast<std::chrono::seconds>(diff).count());
    }
    return 0;
}

unsigned long stopwatch::get_ms() {
    if (is_started()) {
        std::chrono::system_clock::duration  diff;
        diff = std::chrono::system_clock::now() - m_start;
        return (unsigned)(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
    }
    return 0;
}

unsigned long stopwatch::get_us() {
    if (is_started()) {
        std::chrono::system_clock::duration  diff;
        diff = std::chrono::system_clock::now() - m_start;
        return (unsigned)(std::chrono::duration_cast<std::chrono::microseconds>(diff).count());
    }
    return 0;
}

void stopwatch::print_sec() {
    fprintf(stderr, "%lu [sec]\n", get_sec());
}

void stopwatch::print_ms() {
    fprintf(stderr, "%lu [ms]\n", get_ms());
}

void stopwatch::print_us() {
    fprintf(stderr, "%lu [us]\n", get_us());
}