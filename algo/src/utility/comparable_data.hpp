#pragma once
#include <iostream>

template<class T, class S>
struct ComparableData {
    T rating;
    S data;
    ComparableData() : rating(), data() {}
    ComparableData(T rating, S data) : rating(rating), data(data) {}
    constexpr bool operator< (const ComparableData<T, S>& another) const { return rating < another.rating; }
    constexpr bool operator> (const ComparableData<T, S>& another) const { return another < *this; }
    constexpr bool operator<=(const ComparableData<T, S>& another) const { return !(*this > another); }
    constexpr bool operator>=(const ComparableData<T, S>& another) const { return !(*this < another); }
    constexpr bool operator==(const ComparableData<T, S>& another) const { return rating == another.rating; }
    constexpr bool operator!=(const ComparableData<T, S>& another) const { return !(*this == another); }
};
template<class T, class S> std::ostream& operator << (std::ostream& os, const ComparableData<T, S> cd) {
    os << "ComparableData {rating: " << cd.eval_val << ", data: " << cd.data << "}";
    return os;
}
