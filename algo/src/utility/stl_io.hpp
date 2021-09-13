#pragma once

#include <set>
#include <map>
#include <list>
#include <array>
#include <deque>
#include <vector>
#include <iostream>

template<class T> std::istream& operator >> (std::istream& is, std::list<T>& l) {
    for(auto& i : l) is >> i;
    return is;
}
template<class T> std::istream& operator >> (std::istream& is, std::deque<T>& d) {
    for(auto& i : d) is >> i;
    return is;
}
template<class T> std::istream& operator >> (std::istream& is, std::vector<T>& v) {
    for(auto& i : v) is >> i;
    return is;
}
template<class T, size_t N> std::istream& operator >> (std::istream& is, std::array<T, N>& a) {
    for(auto& i : a) is >> i;
    return is;
}
template<class T, class S> std::istream& operator >> (std::istream& is, std::pair<T, S>& p) {
    is >> p.first >> p.second;
    return is;
}

template<class T> std::ostream& operator << (std::ostream& os, const std::list<T>& l);
template<class T> std::ostream& operator << (std::ostream& os, const std::deque<T>& d);
template<class T> std::ostream& operator << (std::ostream& os, const std::vector<T>& v);
template<class T> std::ostream& operator << (std::ostream& os, const std::vector<std::vector<T>>& v);
template<class T, size_t N> std::ostream& operator << (std::ostream& os, const std::array<T, N>& a);
template<class T, size_t N, size_t M> std::ostream& operator << (std::ostream& os, const std::array<std::array<T, M>, N>& a);
template<class T> std::ostream& operator << (std::ostream& os, const std::set<T>& s);
template<class T, class S> std::ostream& operator << (std::ostream& os, const std::map<T,S>& m);
template<class T, class S> std::ostream& operator << (std::ostream& os, const std::pair<T, S>& p);

template<class T> std::ostream& operator << (std::ostream& os, const std::list<T>& l) {
    if (l.empty()) return os;
    os << l.front();
    for(auto itr = ++l.begin(); itr != l.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> std::ostream& operator << (std::ostream& os, const std::deque<T>& d) {
    if (d.empty()) return os;
    os << d.front();
    for(auto itr = d.begin()+1; itr != d.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> std::ostream& operator << (std::ostream& os, const std::vector<T>& v) {
    if (v.empty()) return os;
    os << v.front();
    for(auto itr = v.begin()+1; itr != v.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> std::ostream& operator << (std::ostream& os, const std::vector<std::vector<T>>& v) {
    if (v.empty()) return os;
    os << v.front();
    for(auto itr = v.begin()+1; itr != v.end(); ++itr) os << std::endl << *itr;
    return os;
}
template<class T, size_t N> std::ostream& operator << (std::ostream& os, const std::array<T, N>& a) {
    if (N == 0) return os;
    os << a.front();
    for(auto itr = a.begin()+1; itr != a.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T, size_t N, size_t M> std::ostream& operator << (std::ostream& os, const std::array<std::array<T, M>, N>& a) {
    if (N == 0) return os;
    os << a.front();
    for(auto itr = a.begin()+1; itr != a.end(); ++itr) os << std::endl << *itr;
    return os;
}
template<class T> std::ostream& operator << (std::ostream& os, const std::set<T>& s) {
    if (s.empty()) return os;
    os << *s.begin();
    for(auto itr = ++s.begin(); itr != s.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T, class S> std::ostream& operator << (std::ostream& os, const std::map<T,S>& m) {
    if (m.empty()) return os;
    os << m.begin()->first << " " << m.begin()->second;
    for(const auto& [key, val] : m) os << std::endl << key << " " << val;
    return os;
}
template<class T, class S> std::ostream& operator << (std::ostream& os, const std::pair<T, S>& p) {
    os << p.first << " " << p.second;
    return os;
}
