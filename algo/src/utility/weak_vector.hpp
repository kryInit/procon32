#pragma once
#include <array>

template<class T, size_t N> class weak_vector {
    int head;
    std::array<T, N> data;

public:
    weak_vector() : head(), data() {}
    int size() const { return head; }
    bool empty() const { return head == 0; }
    void clear() { head = 0; }
    void resize(size_t new_head) { head = new_head; }
    void push_back(const T& val) { data[head] = val; head++; }
    template<class... Args> void emplace_back(const Args&... args) { data[head] = {args...}; head++; }
    void swap_and_erase(size_t idx) { swap(data[idx], data[head-1]); pop_back(); }
    T get_and_pop_back() { return data[--head]; }
    void pop_back() { head--; }
    T back() const { return data[head-1]; }
    T front() const { return data[0]; }
    T* begin() { return data.begin(); }
    T* end() { return data.begin()+head; }
    T& operator[](size_t idx) { return data[idx]; }
    const T* begin() const { return data.begin(); }
    const T* end() const { return data.begin()+head; }
    const T& operator[](size_t idx) const { return data[idx]; }
};

