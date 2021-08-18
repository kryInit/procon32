#include <timing_device.hpp>
#include <iomanip>

using namespace std::chrono;

TimingDevice::TimingDevice() : m_start(system_clock::now()) {}
TimingDevice::TimingDevice(bool start) : m_start(start ? system_clock::now() : system_clock::time_point::min()) {}

void TimingDevice::clear() { m_start = system_clock::time_point::min(); }
void TimingDevice::restart() { m_start = system_clock::now(); }
bool TimingDevice::is_started() const { return (m_start.time_since_epoch() != system_clock::time_point::min().time_since_epoch()); }
unsigned long long TimingDevice::get_sec() const {
    if (is_started()) return static_cast<unsigned long long>(duration_cast<seconds>(system_clock::now() - m_start).count());
    else return 0;
}
unsigned long long TimingDevice::get_ms() const {
    if (is_started()) return static_cast<unsigned long long>(duration_cast<milliseconds>(system_clock::now() - m_start).count());
    else return 0;
}
unsigned long long TimingDevice::get_us() const {
    if (is_started()) return static_cast<unsigned long long>(duration_cast<microseconds>(system_clock::now() - m_start).count());
    else return 0;
}

void TimingDevice::print_sec(std::ostream& os) const {
    os << get_sec() << " [sec]" << std::endl;
}
void TimingDevice::print_ms(std::ostream& os) const {
    os << get_ms() << " [ms]" << std::endl;
}
void TimingDevice::print_us(std::ostream& os) const {
    os << get_us() << " [us]" << std::endl;
}
void TimingDevice::print_elapsed(std::ostream& os) const {
    auto elapsed_us = static_cast<double>(get_us());
    os << std::fixed << std::setprecision(3);
    if (elapsed_us > 1e6) os << elapsed_us / 1e6 << " [sec]";
    else if (elapsed_us > 1e3) os << elapsed_us / 1e3 << " [ms]";
    else os << elapsed_us << " [us]";
    os << std::endl;
}

void TimingDevice::print_sec() const {
    print_sec(std::cout);
}
void TimingDevice::print_ms() const {
    print_ms(std::cout);
}
void TimingDevice::print_us() const {
    print_us(std::cout);
}
void TimingDevice::print_elapsed() const {
    print_elapsed(std::cout);
}

void TimingDevice::eprint_sec() const {
    print_sec(std::cerr);
}
void TimingDevice::eprint_ms() const {
    print_ms(std::cerr);
}
void TimingDevice::eprint_us() const {
    print_us(std::cerr);
}
void TimingDevice::eprint_elapsed() const {
    print_elapsed(std::cerr);
}
