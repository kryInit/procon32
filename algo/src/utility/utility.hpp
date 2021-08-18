#pragma once

#include <set>
#include <map>
#include <list>
#include <deque>
#include <array>
#include <vector>
#include <sstream>
#include <queue>
#include <cmath>
#include <thread>
#include <iostream>
#include <type_traits>

#include <vec2.hpp>
#include <vec3.hpp>
#include <random.hpp>

#define GET_VAL_NAME(...) #__VA_ARGS__
#define DUMP_TO(os, ...) Utility::dump_to(os, __VA_ARGS__)
#define DUMP(...) Utility::dump(__VA_ARGS__)
#define EDUMP(...) Utility::edump(__VA_ARGS__)
#define PRINT_TO(os, ...) os << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::print_to(os, __VA_ARGS__)
#define PRINT(...) cout << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::print(__VA_ARGS__)
#define EPRINT(...) cerr << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::eprint(__VA_ARGS__)
#define rep(i,n) for(int i=0; (i)<static_cast<int>(n); ++(i))
#define repo(i,o,n) for(int i=(o); (i)<static_cast<int>(n); ++(i))
#define revrep(i,n) for(int i=static_cast<int>(n)-1; (i)>=0; --(i))
#define all(v) (v).begin(), (v).end()
#define rall(v) (v).rbegin(), (v).rend()

using Index = size_t;

template<class T> using reverse_priority_queue = std::priority_queue<T, std::vector<T>, std::greater<T>>;
template<class T> void chmax(T& a, const T& b) { a = std::max(a, b); }
template<class T> void chmin(T& a, const T& b) { a = std::min(a, b); }
template<class T> T min(const std::deque<T>& d) { return *min_element(d.begin(), d.end()); }
template<class T> T max(const std::deque<T>& d) { return *max_element(d.begin(), d.end()); }
template<class T> T min(const std::vector<T>& v) { return *min_element(v.begin(), v.end()); }
template<class T> T max(const std::vector<T>& v) { return *max_element(v.begin(), v.end()); }
template<class T, size_t N> constexpr T min(const std::array<T, N>& a) { return *min_element(a.begin(), a.end()); }
template<class T, size_t N> constexpr T max(const std::array<T, N>& a) { return *max_element(a.begin(), a.end()); }
template<class... Args> constexpr auto min(const Args&... args) { return min(std::initializer_list<std::common_type_t<Args...>>{args...}); }
template<class... Args> constexpr auto max(const Args&... args) { return max(std::initializer_list<std::common_type_t<Args...>>{args...}); }
template<class T> T sum(const std::deque<T>& d) { return accumulate(d.begin(), d.end(), static_cast<T>(0)); }
template<class T> T sum(const std::vector<T>& v) { return accumulate(v.begin(), v.end(), static_cast<T>(0)); }
template<class T, size_t N> T sum(const std::array<T, N>& a) { return accumulate(a.begin(), a.end(), static_cast<T>(0)); }
template<class T> T sum(const std::initializer_list<T>& t) { return accumulate(t.begin(), t.end(), static_cast<T>(0)); }
template<class... Args> auto sum(const Args&... args) { return sum(std::initializer_list<std::common_type_t<Args...>>{args...}); }

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

namespace Utility {
    using namespace std;

    inline void sleep_sec(size_t time) { this_thread::sleep_for(chrono::seconds(time)); }
    inline void sleep_millsec(size_t time) { this_thread::sleep_for(chrono::milliseconds(time)); }

    template<typename T, size_t N, typename enable_if<(N > 0), nullptr_t>::type = nullptr> struct ndvector_impl { using type = vector<typename ndvector_impl<T, N - 1>::type>; };
    template<typename T> struct ndvector_impl<T, 1> { using type = vector<T>; };
    template<typename T, size_t N> using nvector = typename ndvector_impl<T, N>::type;

    template<class T> T gen_vec0() { return static_cast<T>(0); }
    template<class T> T gen_vec(T init) { return static_cast<T>(init); }
    template<class T, class N, class... Ns> auto gen_vec0(N n, Ns... ns) {
        auto tmp = gen_vec0<T, Ns...>(ns...);
        return vector<decltype(tmp)>(n, tmp);
    }
    template<class T, class N, class... Ns> auto gen_vec(T init, N n, Ns... ns) {
        auto tmp = gen_vec<T, Ns...>(init, ns...);
        return vector<decltype(tmp)>(n, tmp);
    }
    template<class T> constexpr T gen_arr() { return static_cast<T>(0); }
    template<class T> constexpr T gen_arr(T ini) { return static_cast<T>(ini); }
    template<class T, size_t x, size_t... xs> constexpr auto gen_arr() {
        auto tmp = gen_arr<T, xs...>();
        array<decltype(tmp), x> result{};
        for(auto& var : result) var = tmp;
        return result;
    }
    template<class T, size_t x, size_t... xs> constexpr auto gen_arr(T ini) {
        auto tmp = gen_arr<T, xs...>(ini);
        array<decltype(tmp), x> result{};
        for(auto& var : result) var = tmp;
        return result;
    }

    template<class... T> void input_from(istream& is, T&... a) { (is >> ... >> a); }
    template<class... T> void input(T&... a) { input_from(cin, a...); }

    inline void print_wall(ostream& os, const string& msg, size_t size=40) {
        size_t msg_size = msg.size();
        chmax(size, msg_size+6);
        size_t l = (size-msg_size)/2 - 2, r = (size-msg_size)/2-2;
        os << ' ';
        rep(i, l) os << '=';
        if (msg.empty()) os << "==";
        else os << ' ' << msg << ' ';
        rep(i, r) os << '=';
        os << ' ';
        os << endl;
    }
    inline void print_wall(ostream& os, size_t size=40) {
        chmax(size, (size_t)3);
        os << ' ';
        rep(_, size-2) os << '=';
        os << ' ';
        os << endl;
    }
    inline void print_wall(const string& msg, size_t size=40) {
        print_wall(cout, msg, size);
    }
    inline void print_wall(size_t size=40) {
        print_wall(cout, size);
    }
    inline void eprint_wall(const string& msg, size_t size=40) {
        print_wall(cerr, msg, size);
    }
    inline void eprint_wall(size_t size=40) {
        print_wall(cerr, size);
    }

    template<class T> void print_statistical_information(const vector<T> &v) {
        const T minimum = min(v);
        const T maximum = max(v);
        const T median = (v.size()%2 == 1 ? v[v.size()/2] : (v[v.size()/2-1] + v[v.size()/2]) / 2.);
        const T v_sum = sum(v);
        const double ave = static_cast<double>(v_sum) / v.size();
        const double var = accumulate(v.begin(), v.end(), 0.,  [](double acc, T i) { return acc + (double)i*(double)i; }) / v.size() - ave*ave;
        const double std = sqrt(var);

        print_wall("print statistical information", 39);
        cout << "minimum    : " << minimum << endl;
        cout << "maximum    : " << maximum << endl;
        cout << "median     : " << median << endl;
        cout << "sum        : " << v_sum << endl;
        cout << "ave        : " << ave << endl;
        cout << "var        : " << var << endl;
        cout << "std        : " << std << endl;
        cout << "ave ±  std : [" << ave - std << ", " << ave + std << "], 68.3%" << endl;
        cout << "ave ± 2std : [" << ave - 2*std << ", " << ave + 2*std << "], 95.4%" << endl;
        cout << "ave ± 3std : [" << ave - 3*std << ", " << ave + 3*std << "], 99.7%" << endl;
        print_wall(39);
    }

    inline void exit_with_message(const string& message) {
        eprint_wall("EXIT WITH MESSAGE");
        cerr << message << endl;
        eprint_wall(39);
        exit(-1);
    }

    template<class T> string show(const list<T>& l);
    template<class T> string show(const deque<T>& d);
    template<class T> string show(const vector<T>& v);
    template<class T, size_t N> string show(const array<T, N>& a);
    template<class T> string show(const set<T>& s);
    template<class T, class S> string show(const map<T, S>& m);
    template<class T, class S> string show(const pair<T, S>& p);

    inline string show(const char *c) { return string(c); }
    inline string show(char c) { return {c}; }
    inline string show(bool f) { return (f ? "true" : "false"); }
    inline string show(const string& s) { return s; }
    template<class T> string show(T a) { stringstream ss; ss << a; return ss.str(); }
    inline string show(unsigned char a) { return show((unsigned)a); }

    template<class T> string show(Vec2<T> v) { return "(" + show(v.x) + ", " + show(v.y) + ")"; }
    template<class T> string show(Vec3<T> v) { return "(" + show(v.x) + ", " + show(v.y) + ", " + show(v.y) + ")"; }

    template<class T> string show(const list<T>& l) {
        if (l.empty()) return "{}";
        string s = "{";
        for(const auto &i : l) s += show(i) + ", ";
        s.pop_back();
        s.back() = '}';
        return s;
    }
    template<class T> string show(const deque<T>& d) {
        if (d.empty()) return "{}";
        string s = "{";
        for(const auto &i : d) s += show(i) + ", ";
        s.pop_back();
        s.back() = '}';
        return s;
    }
    template<class T> string show(const vector<T>& v) {
        if (v.empty()) return "{}";
        string s = "{";
        for(const auto &i : v) s += show(i) + ", ";
        s.pop_back();
        s.back() = '}';
        return s;
    }
    template<class T, size_t N> string show(const array<T, N>& a) {
        if (a.empty()) return "{}";
        string s = "{";
        for(const auto &i : a) s += show(i) + ", ";
        s.pop_back();
        s.back() = '}';
        return s;
    }
    template<class T> string show(const set<T>& s) {
        if (s.empty()) return "{}";
        string str = "{";
        for(const auto &i : s) str += show(i) + ", ";
        str.pop_back();
        str.back() = '}';
        return s;
    }
    template<class T, class S> string show(const map<T, S>& m) {
        if (m.empty()) return "{}";
        string s = "{";
        for(const auto &i : m) s += show(i.first) + ": " + show(i.second) + ", ";
        s.pop_back();
        s.back() = '}';
        return s;
    }
    template<class T, class S> string show(const pair<T, S>& p) { return "<" + show(p.first) + ", " + show(p.second) + ">"; }

    inline string concat_as_str(const string& delimiter) { return ""; }
    template<class T> string concat_as_str(const string& delimiter, T&& a) { return show(a); }
    template<class Head, class... Tail> string concat_as_str(const string& delimiter, Head&& head, Tail&&... tail) {
        return show(head) + delimiter + concat_as_str(delimiter, tail...);
    }

    template<class... Args> string concat(Args&&... args) { return concat_as_str("", args...); }
    template<class... Args> string concat_with_space(Args&&... args) { return concat_as_str(" ", args...); }
    template<class... Args> string concat_with_comma(Args&&... args) { return concat_as_str(", ", args...); }

    template<class... Args> void print_to(ostream& os, Args&&... args) {
        os << concat_with_comma(args...) << endl;
    }
    template<class... Args> void print(Args&&... args) {
        print_to(cout, args...);
    }
    template<class... Args> void eprint(Args&&... args) {
        print_to(cerr, args...);
    }

    inline void dump_to(ostream& os) { os << endl; }
    template<class Head, class... Args> void dump_to(ostream& os, Head&& head, Args&&... args) {
        os << head;
        if (sizeof...(Args) == 0) os << endl;
        else { os << " "; dump_to(os, args...); }
    }
    template<class... Args> void dump(Args&&... args) {
        dump_to(cout, args...);
    }
    template<class... Args> void edump(Args&&... args) {
        dump_to(cerr, args...);
    }

    template<class T> bool is_same(const T& a, const T& b) { return a == b; }
    template<class T> bool is_same(const vector<T>& v1, const vector<T>& v2) {
        if (v1.size() != v2.size()) return false;
        rep(i,v1.size()) if (!is_same(v1[i], v2[i])) return false;
        return true;
    }

    template<class T> void print_diff(const vector<T>& v1, const vector<T>& v2) {
        if (v1.size() != v2.size()) { PRINT(v1.size(), v2.size()); return; }
        rep(i,v1.size()) if (!is_same(v1[i], v2[i])) PRINT(i, v1[i], v2[i]);
    }

    template<class T> vector<T> get_subvec(const vector<T>& v, Index pos, size_t n) {
        if (v.empty()) return {};
        chmin(pos, v.size());
        chmin(n, v.size()-pos);
        return {v.begin()+pos, v.begin()+pos+n};
    }

    template<class T> size_t hash_vec(const vector<T>& v) {
        size_t seed = v.size();
        for(const auto& i : v) seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
    template<class T> void shuffle(vector<T>& v) {
        for(size_t i = v.size(); --i>0;) swap(v[i], v[Random::rand_range(i)]);
    }
}
