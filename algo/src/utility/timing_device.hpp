#pragma once

#include <iostream>
#include <iomanip>
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

inline TimingDevice::TimingDevice() : m_start(std::chrono::system_clock::now()) {}
inline TimingDevice::TimingDevice(bool start) : m_start(start ? std::chrono::system_clock::now() : std::chrono::system_clock::time_point::min()) {}

inline void TimingDevice::clear() { m_start = std::chrono::system_clock::time_point::min(); }
inline void TimingDevice::restart() { m_start = std::chrono::system_clock::now(); }
inline bool TimingDevice::is_started() const { return (m_start.time_since_epoch() != std::chrono::system_clock::time_point::min().time_since_epoch()); }
inline unsigned long long TimingDevice::get_sec() const {
    if (is_started()) return static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_start).count());
    else return 0;
}
inline unsigned long long TimingDevice::get_ms() const {
    if (is_started()) return static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_start).count());
    else return 0;
}
inline unsigned long long TimingDevice::get_us() const {
    if (is_started()) return static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - m_start).count());
    else return 0;
}

inline void TimingDevice::print_sec(std::ostream& os) const { os << get_sec() << " [sec]" << std::endl; }
inline void TimingDevice::print_ms(std::ostream& os) const { os << get_ms() << " [ms]" << std::endl; }
inline void TimingDevice::print_us(std::ostream& os) const { os << get_us() << " [us]" << std::endl; }
inline void TimingDevice::print_elapsed(std::ostream& os) const {
    auto elapsed_us = static_cast<double>(get_us());
    os << std::fixed << std::setprecision(3);
    if (elapsed_us > 1e6) os << elapsed_us / 1e6 << " [sec]";
    else if (elapsed_us > 1e3) os << elapsed_us / 1e3 << " [ms]";
    else os << elapsed_us << " [us]";
    os << std::endl;
}

inline void TimingDevice::print_sec() const { print_sec(std::cout); }
inline void TimingDevice::print_ms() const { print_ms(std::cout); }
inline void TimingDevice::print_us() const { print_us(std::cout); }
inline void TimingDevice::print_elapsed() const { print_elapsed(std::cout); }

inline void TimingDevice::eprint_sec() const { print_sec(std::cerr); }
inline void TimingDevice::eprint_ms() const { print_ms(std::cerr); }
inline void TimingDevice::eprint_us() const { print_us(std::cerr); }
inline void TimingDevice::eprint_elapsed() const { print_elapsed(std::cerr); }
