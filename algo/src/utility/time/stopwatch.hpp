#pragma once
#include <chrono>

class stopwatch {
    std::chrono::system_clock::time_point m_start;
public:

    stopwatch();

    void clear();
    bool is_started();
    void start();
    unsigned long get_sec();
    unsigned long get_ms();
    unsigned long get_us();
    void print_sec();
    void print_ms();
    void print_us();
};
