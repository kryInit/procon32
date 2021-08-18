#pragma once
#include <iostream>

template<class T, class S>
struct ComparableData {
    T eval_val;
    S data;
    ComparableData() : eval_val(), data() {}
    ComparableData(T eval_val, S data) : eval_val(eval_val), data(data) {}
    constexpr bool operator<(const ComparableData<T, S>& another) const {
        return eval_val < another.eval_val;
    }
    constexpr bool operator>(const ComparableData<T, S>& another) const {
        return another < *this;
    }
    constexpr bool operator<=(const ComparableData<T, S>& another) const {
        return !(*this > another);
    }
    constexpr bool operator>=(const ComparableData<T, S>& another) const {
        return !(*this < another);
    }
    constexpr bool operator==(const ComparableData<T, S>& another) const {
        return eval_val == another.eval_val;
    }
    constexpr bool operator!=(const ComparableData<T, S>& another) const {
        return !(*this == another);
    }
};
template<class T, class S> std::ostream& operator << (std::ostream& os, const ComparableData<T, S> cd) {
    os << "ComparableData {eval_val: " << cd.eval_val << ", data: " << cd.data << "}";
    return os;
}
