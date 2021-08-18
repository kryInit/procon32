#pragma once
#include <iostream>
#include <chrono>

class TimingDevice {
    std::chrono::system_clock::time_point m_start;
public:

    TimingDevice();
    explicit TimingDevice(bool start);

    void clear();
    void restart();
    [[nodiscard]] bool is_started() const;
    [[nodiscard]] unsigned long long get_sec() const;
    [[nodiscard]] unsigned long long get_ms() const;
    [[nodiscard]] unsigned long long get_us() const;

    void print_sec(std::ostream& os) const;
    void print_ms(std::ostream& os) const;
    void print_us(std::ostream& os) const;
    void print_elapsed(std::ostream& os) const;

    void print_sec() const;
    void print_ms() const;
    void print_us() const;
    void print_elapsed() const;

    void eprint_sec() const;
    void eprint_ms() const;
    void eprint_us() const;
    void eprint_elapsed() const;
};
