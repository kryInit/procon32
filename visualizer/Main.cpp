#include <Siv3D.hpp>
#include <fstream>
#include <queue>
#include <thread>

using namespace std;

/*/
 *
 * 目標的なもの
 * restore_image:
 *     推定画像の視覚化
 *     手動操作
 *
 * build_procedure:
 *     手順の視覚化
 *     手動操作
 *     部分的な手動操作
 *     プログラムへの司令
 *
 *
 * 画面の大きさなどの管理をする人
 *
 *
 *
/*/

#define UNREACHABLE Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] unreachable"));
#define UNIMPLEMENTED Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] unimplemented"));
#define EXIT_DEBUG(...) Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] ") + Utility::concat_with_space(__VA_ARGS__))
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

template<class T> using reverse_priority_queue = priority_queue<T, vector<T>, greater<T>>;
template<class T> void chmax(T& a, const T& b) { a = max(a, b); }
template<class T> void chmin(T& a, const T& b) { a = min(a, b); }
template<class T> T min(const deque<T>& d) { return *min_element(d.begin(), d.end()); }
template<class T> T max(const deque<T>& d) { return *max_element(d.begin(), d.end()); }
template<class T> T min(const vector<T>& v) { return *min_element(v.begin(), v.end()); }
template<class T> T max(const vector<T>& v) { return *max_element(v.begin(), v.end()); }
template<class T, size_t N> constexpr T min(const array<T, N>& a) { return *min_element(a.begin(), a.end()); }
template<class T, size_t N> constexpr T max(const array<T, N>& a) { return *max_element(a.begin(), a.end()); }
template<class... Args> constexpr auto min(const Args&... args) { return min(initializer_list<common_type_t<Args...>>{args...}); }
template<class... Args> constexpr auto max(const Args&... args) { return max(initializer_list<common_type_t<Args...>>{args...}); }
template<class T> T sum(const deque<T>& d) { return accumulate(d.begin(), d.end(), static_cast<T>(0)); }
template<class T> T sum(const vector<T>& v) { return accumulate(v.begin(), v.end(), static_cast<T>(0)); }
template<class T, size_t N> T sum(const array<T, N>& a) { return accumulate(a.begin(), a.end(), static_cast<T>(0)); }
template<class T> T sum(const initializer_list<T>& t) { return accumulate(t.begin(), t.end(), static_cast<T>(0)); }
template<class... Args> auto sum(const Args&... args) { return sum(initializer_list<common_type_t<Args...>>{args...}); }

template<class T> istream& operator >> (istream& is, list<T>& l) {
    for(auto& i : l) is >> i;
    return is;
}
template<class T> istream& operator >> (istream& is, deque<T>& d) {
    for(auto& i : d) is >> i;
    return is;
}
template<class T> istream& operator >> (istream& is, vector<T>& v) {
    for(auto& i : v) is >> i;
    return is;
}
template<class T, size_t N> istream& operator >> (istream& is, array<T, N>& a) {
    for(auto& i : a) is >> i;
    return is;
}
template<class T, class S> istream& operator >> (istream& is, pair<T, S>& p) {
    is >> p.first >> p.second;
    return is;
}

template<class T> ostream& operator << (ostream& os, const list<T>& l);
template<class T> ostream& operator << (ostream& os, const deque<T>& d);
template<class T> ostream& operator << (ostream& os, const vector<T>& v);
template<class T> ostream& operator << (ostream& os, const vector<vector<T>>& v);
template<class T, size_t N> ostream& operator << (ostream& os, const array<T, N>& a);
template<class T, size_t N, size_t M> ostream& operator << (ostream& os, const array<array<T, M>, N>& a);
template<class T> ostream& operator << (ostream& os, const set<T>& s);
template<class T, class S> ostream& operator << (ostream& os, const map<T,S>& m);
template<class T, class S> ostream& operator << (ostream& os, const pair<T, S>& p);

template<class T> ostream& operator << (ostream& os, const list<T>& l) {
    if (l.empty()) return os;
    os << l.front();
    for(auto itr = ++l.begin(); itr != l.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> ostream& operator << (ostream& os, const deque<T>& d) {
    if (d.empty()) return os;
    os << d.front();
    for(auto itr = d.begin()+1; itr != d.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> ostream& operator << (ostream& os, const vector<T>& v) {
    if (v.empty()) return os;
    os << v.front();
    for(auto itr = v.begin()+1; itr != v.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T> ostream& operator << (ostream& os, const vector<vector<T>>& v) {
    if (v.empty()) return os;
    os << v.front();
    for(auto itr = v.begin()+1; itr != v.end(); ++itr) os << endl << *itr;
    return os;
}
template<class T, size_t N> ostream& operator << (ostream& os, const array<T, N>& a) {
    if (N == 0) return os;
    os << a.front();
    for(auto itr = a.begin()+1; itr != a.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T, size_t N, size_t M> ostream& operator << (ostream& os, const array<array<T, M>, N>& a) {
    if (N == 0) return os;
    os << a.front();
    for(auto itr = a.begin()+1; itr != a.end(); ++itr) os << endl << *itr;
    return os;
}
template<class T> ostream& operator << (ostream& os, const set<T>& s) {
    if (s.empty()) return os;
    os << *s.begin();
    for(auto itr = ++s.begin(); itr != s.end(); ++itr) os << " " << *itr;
    return os;
}
template<class T, class S> ostream& operator << (ostream& os, const map<T,S>& m) {
    if (m.empty()) return os;
    os << m.begin()->first << " " << m.begin()->second;
    for(const auto& [key, val] : m) os << endl << key << " " << val;
    return os;
}
template<class T, class S> ostream& operator << (ostream& os, const pair<T, S>& p) {
    os << p.first << " " << p.second;
    return os;
}


namespace DirectionNS {
    using DirectionType = unsigned char;

    // [Caution] DO NOT change the following values
    constexpr DirectionType UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3;

    class Direction;

    class DirectionBase {
        DirectionType dir;
        constexpr DirectionBase() : dir() {}
        constexpr explicit DirectionBase(DirectionType dir) : dir(dir) {}

        friend Direction;
    public:
        constexpr bool operator==(const DirectionBase& another) const { return dir == another.dir; }
        constexpr bool operator!=(const DirectionBase& another) const { return dir != another.dir; }
        constexpr bool operator==(const Direction& another) const;
        constexpr bool operator!=(const Direction& another) const;


        [[nodiscard]] constexpr int dx() const {
            return (dir == RIGHT ? 1 : (dir == LEFT ? -1 : 0));
        }
        [[nodiscard]] constexpr int dy() const {
            return (dir == DOWN ? 1 : (dir == UP ? -1 : 0));
        }

        [[nodiscard]] constexpr char to_char() const {
            if (dir == UP) return 'U';
            else if (dir == RIGHT) return 'R';
            else if (dir == DOWN) return 'D';
            else return 'L';
        }

        constexpr void rotate_cw(unsigned int times)   { dir = (static_cast<unsigned int>(dir) + times  ) & 3; }
        constexpr void reverse(unsigned int times)     { dir = (static_cast<unsigned int>(dir) + times*2) & 3; }
        constexpr void rotate_ccw(unsigned int times)  { dir = (static_cast<unsigned int>(dir) + times*3) & 3; }
        constexpr void rotate_cw()    { rotate_cw(1);  }
        constexpr void reverse()      { reverse(1);    }
        constexpr void rotate_ccw()   { rotate_ccw(1); }
        [[nodiscard]] constexpr DirectionBase get_dir_rotated_cw(int times)   const { return DirectionBase((static_cast<unsigned int>(dir) + times  ) & 3); }
        [[nodiscard]] constexpr DirectionBase get_dir_reversed(int times)     const { return DirectionBase((static_cast<unsigned int>(dir) + times*2) & 3); }
        [[nodiscard]] constexpr DirectionBase get_dir_rotated_ccw(int times)  const { return DirectionBase((static_cast<unsigned int>(dir) + times*3) & 3); }
        [[nodiscard]] constexpr DirectionBase get_dir_rotated_cw()    const { return get_dir_rotated_cw(1); }
        [[nodiscard]] constexpr DirectionBase get_dir_reversed()      const { return get_dir_reversed(1); }
        [[nodiscard]] constexpr DirectionBase get_dir_rotated_ccw()   const { return get_dir_rotated_ccw(1); }

        [[nodiscard]] constexpr bool is_vertical()   const { return (dir == UP)    || (dir == DOWN); }
        [[nodiscard]] constexpr bool is_horizontal() const { return (dir == RIGHT) || (dir == LEFT); }
    };

    class Direction {
        DirectionBase dir;

        friend DirectionBase;
    public:
        static constexpr DirectionBase U{UP};
        static constexpr DirectionBase R{RIGHT};
        static constexpr DirectionBase D{DOWN};
        static constexpr DirectionBase L{LEFT};

        static constexpr array<DirectionBase, 4> All{U, R, D, L};

        constexpr Direction() : dir() {}
        constexpr Direction(DirectionBase dir) : dir(dir) {}

        constexpr bool operator==(const Direction& another) const { return dir == another.dir; }
        constexpr bool operator==(const DirectionBase& another) const { return dir == another; }
        constexpr bool operator!=(const Direction& another) const { return dir != another.dir; }
        constexpr bool operator!=(const DirectionBase& another) const { return dir != another; }

        [[nodiscard]] constexpr int dx() const { return dir.dx(); }
        [[nodiscard]] constexpr int dy() const { return dir.dy(); }
        [[nodiscard]] constexpr char to_char() const { return dir.to_char(); }

        constexpr void rotate_cw(unsigned int times)   { dir.rotate_cw(times);  }
        constexpr void reverse(unsigned int times)     { dir.reverse(times);    }
        constexpr void rotate_ccw(unsigned int times)  { dir.rotate_ccw(times); }
        constexpr void rotate_cw()    { rotate_cw(1);  }
        constexpr void reverse()      { reverse(1);    }
        constexpr void rotate_ccw()   { rotate_ccw(1); }
        [[nodiscard]] constexpr Direction get_dir_rotated_cw(int times)   const { return dir.get_dir_rotated_cw(times);  }
        [[nodiscard]] constexpr Direction get_dir_reversed(int times)     const { return dir.get_dir_reversed(times);    }
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw(int times)  const { return dir.get_dir_rotated_ccw(times); }
        [[nodiscard]] constexpr Direction get_dir_rotated_cw()    const { return get_dir_rotated_cw(1); }
        [[nodiscard]] constexpr Direction get_dir_reversed()      const { return get_dir_reversed(1); }
        [[nodiscard]] constexpr Direction get_dir_rotated_ccw()   const { return get_dir_rotated_ccw(1); }

        [[nodiscard]] constexpr bool is_vertical()   const { return dir.is_vertical();   }
        [[nodiscard]] constexpr bool is_horizontal() const { return dir.is_horizontal(); }

        static constexpr Direction from_char(char c) {
            return (c == 'U' ? U : (c == 'R' ? R : (c == 'D' ? D : L)));
        }
        static constexpr Direction from_char_or(char c, Direction default_dir) {
            return (c == 'U' ? U : (c == 'R' ? R : (c == 'D' ? D : (c == 'L' ? L : default_dir))));
        }
        static constexpr optional<Direction> from_char_safely(char c) {
            if (c == 'U') return U;
            else if (c == 'R') return R;
            else if (c == 'D') return D;
            else if (c == 'L') return L;
            else return nullopt;
        }
    };

    class Path : public vector<Direction> {
    public:
        static Path from_string(const string& s) {
            Path result; result.reserve(s.size());
            for(const auto& c : s) result.push_back(Direction::from_char(c));
            return result;
        }
        static pair<Path, string> from_string_safely(const string& s) {
            Path result; result.reserve(s.size());
            for(int i=0, n=(int)s.size(); i<n; ++i) {
                auto op_dir = Direction::from_char_safely(s[i]);
                if (op_dir) result.push_back(op_dir.value());
                else return {result, s.substr(i,n-i)};
            }
            return {result, ""};
        }
        [[nodiscard]] string to_string() const {
            string result; result.reserve(this->size());
            for(const auto& dir : *this) result.push_back(dir.to_char());
            return result;
        }
    };

    constexpr bool DirectionBase::operator==(const Direction& another) const {
        return dir == another.dir.dir;
    }
    constexpr bool DirectionBase::operator!=(const Direction& another) const {
        return dir != another.dir.dir;
    }

    ostream& operator << (ostream& os, const DirectionBase& dir) {
        os << dir.to_char();
        return os;
    }
    ostream& operator << (ostream& os, const Direction& dir) {
        os << dir.to_char();
        return os;
    }
    ostream& operator << (ostream& os, const Path& path) {
        os << path.to_string();
        return os;
    }
}
using DirectionNS::Direction;
using DirectionNS::Path;


struct Pos {
    int x, y;

    constexpr bool operator< (const Pos& another) const { return x == another.x ? y < another.y : x < another.x; }
    constexpr bool operator> (const Pos& another) const { return another < *this; }
    constexpr bool operator<=(const Pos& another) const { return !(*this > another); }
    constexpr bool operator>=(const Pos& another) const { return !(*this < another); }
    constexpr bool operator==(const Pos& another) const { return x == another.x && y == another.y; }
    constexpr bool operator!=(const Pos& another) const { return !(*this == another); }

    constexpr Pos operator+(const Pos& another) const { return { x + another.x, y + another.y }; }
    constexpr Pos operator-(const Pos& another) const { return { x - another.x, y - another.y }; }
    constexpr Pos operator*(const Pos& another) const { return { x * another.x, y * another.y }; }
    constexpr Pos operator/(const Pos& another) const { return { x / another.x, y / another.y }; }
    constexpr Pos operator%(const Pos& another) const { return { x % another.x, y % another.y }; }
    constexpr Pos operator+(int s) const { return { x+s, y+s }; }
    constexpr Pos operator-(int s) const { return { x-s, y-s }; }
    constexpr Pos operator*(int s) const { return { x*s, y*s }; }
    constexpr Pos operator/(int s) const { return { x/s, y/s }; }
    constexpr Pos operator%(int s) const { return { x%s, y%s }; }

    constexpr Pos& operator+=(const Pos& another) { *this = *this+another; return *this; }
    constexpr Pos& operator-=(const Pos& another) { *this = *this-another; return *this; }
    constexpr Pos& operator*=(const Pos& another) { *this = *this*another; return *this; }
    constexpr Pos& operator/=(const Pos& another) { *this = *this/another; return *this; }
    constexpr Pos& operator%=(const Pos& another) { *this = *this%another; return *this; }
    constexpr Pos& operator+=(int s) { *this = *this+s; return *this; }
    constexpr Pos& operator-=(int s) { *this = *this-s; return *this; }
    constexpr Pos& operator*=(int s) { *this = *this*s; return *this; }
    constexpr Pos& operator/=(int s) { *this = *this/s; return *this; }
    constexpr Pos& operator%=(int s) { *this = *this%s; return *this; }

    [[nodiscard]] constexpr int dot  (const Pos& another) const { return x * another.x + y * another.y; }
    [[nodiscard]] constexpr int cross(const Pos& another) const { return x * another.y - y * another.x; }

    constexpr Pos() : x(0), y(0) {}
    constexpr Pos(int x, int y) : x(x), y(y) {}
    constexpr explicit Pos(DirectionNS::DirectionBase dir) : x(dir.dx()), y(dir.dy()) {}
    constexpr explicit Pos(Direction dir) : x(dir.dx()), y(dir.dy()) {}

    constexpr void move(Direction dir) { *this += Pos(dir); }
              void move(const Path& path) { for(const auto& dir : path) move(dir); }
    constexpr void move_toraly(Direction dir, const Pos& area) {
        move(dir);
        x = (x == -1 ? area.x-1 : (x==area.x ? 0 : x));
        y = (y == -1 ? area.y-1 : (y==area.y ? 0 : y));
    }
    constexpr void move_toraly(const Pos& dp, const Pos& area) {
        *this += (dp%area);
        x = (x + area.x) % area.x;
        y = (y + area.y) % area.y;
    }
              void move_toraly(const Path& path, const Pos& area) { for(const auto& dir : path) move_toraly(dir, area); }

    constexpr Pos get_moved_pos(Direction dir)    const { Pos p = *this; p.move(dir ); return p; }
              Pos get_moved_pos(const Path& path) const { Pos p = *this; p.move(path); return p; }
    constexpr Pos get_moved_toraly_pos(Direction dir,    const Pos& area) const { Pos p = *this; p.move_toraly(dir,  area); return p; }
    constexpr Pos get_moved_toraly_pos(const Pos& dp,    const Pos& area) const { Pos p = *this; p.move_toraly(dp,   area); return p; }
              Pos get_moved_toraly_pos(const Path& path, const Pos& area) const { Pos p = *this; p.move_toraly(path, area); return p; }

    [[nodiscard]] int calc_mh_dist(const Pos& other) const { return abs(x-other.x) + abs(y-other.y); }
    [[nodiscard]] int calc_mh_dist_toraly(const Pos& other, const Pos& area) const { return min(area.x-abs(x-other.x), abs(x-other.x)) + min(area.y-abs(y-other.y), abs(y-other.y)); }

    [[nodiscard]] constexpr bool in_area(const Pos& area) const { return 0 <= x && x < area.x && 0 <= y && y < area.y; }

    [[nodiscard]] constexpr int to_idx(int w) const { return y * w + x; }
    [[nodiscard]] constexpr int to_idx(const Pos& shape) const { return y * shape.x + x; }
    static constexpr Pos from_idx(int idx, int w) { return {idx%w, idx/w}; }
    static constexpr Pos from_idx(int idx, const Pos& shape) { return {idx%shape.x, idx/shape.x}; }
};
ostream& operator<<(ostream& os, const Pos v) {
    os << v.x << ' ' << v.y;
    return os;
}
istream& operator>>(istream& is, Pos& v) {
    is >> v.x >> v.y;
    return is;
}

namespace Random {
    using uint = unsigned int;

    uint seed[4] = {1,2,3,4};

    void set_random_state(uint s) {
        const uint magic_number = 1812433253;
        for(int i=1; i<=4; i++){
            seed[i-1] = s = magic_number * (s^(s>>30U)) + (uint)i;
        }
    }
    uint rand_uint() {
        uint t = (seed[0]^(seed[0]<<11U));
        seed[0]=seed[1];
        seed[1]=seed[2];
        seed[2]=seed[3];
        return seed[3]=(seed[3]^(seed[3]>>19U))^(t^(t>>8U));
    }
    double rand_prob() { return (double)rand_uint() / ((double)UINT_MAX+1.); }
    uint rand_range(uint r) { return rand_uint()%r; }
    int rand_range(int l, int r) { return static_cast<int>(rand_uint() % static_cast<uint>(r-l)) + l; }
    uint simple_exp_rand(uint r) { return (pow(2, rand_prob())-1)*r; }
    uint exp_rand(uint r, double a) { return pow(pow(2, rand_prob())-1, a) * r; }
}

namespace Utility {
    template<class... T> void input_from(istream& is, T&... a) { (is >> ... >> a); }
    template<class... T> void input(T&... a) { input_from(cin, a...); }

    template<class T> string show(const list<T>& l);
    template<class T> string show(const deque<T>& d);
    template<class T> string show(const vector<T>& v);
    template<class T, size_t N> string show(const array<T, N>& a);
    template<class T> string show(const set<T>& s);
    template<class T, class S> string show(const map<T, S>& m);
    template<class T, class S> string show(const pair<T, S>& p);

    string show(const char *c) { return {c}; }
    string show(char c) { return {c}; }
    string show(bool f) { return (f ? "true" : "false"); }
    string show(const string& s) { return s; }
    string show(const Path& p) { stringstream ss; ss << p; return ss.str(); }
    template<class T> string show(T a) { stringstream ss; ss << a; return ss.str(); }
    string show(unsigned char a) { return show((unsigned)a); }

    template<class T> string show(Pos p) { return "(" + show(p.x) + ", " + show(p.y) + ")"; }

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

    string concat_as_str(const string& delimiter) { return ""; }
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

    void dump_to(ostream& os) { os << endl; }
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

    constexpr array<Pos, 4> dpos4() {
        return { Pos(Direction::U), Pos(Direction::R), Pos(Direction::D), Pos(Direction::L) };
    }
    constexpr array<Pos, 8> dpos8() {
        constexpr Direction U = Direction::U, R = Direction::R, D = Direction::D, L = Direction::L;
        return { Pos(U), Pos(U)+Pos(R), Pos(R), Pos(R)+Pos(D), Pos(D), Pos(D)+Pos(L), Pos(L), Pos(L)+Pos(U) };
    }
}


namespace Constant {
    constexpr Size WINDOW_RATIO(16, 9);
    constexpr Size SCENE_SIZE(1920, 1080);

    constexpr Size MARGIN(10,10);

    constexpr Size MAXIMUM_FRAGS_AREA_SIZE(SCENE_SIZE.y, SCENE_SIZE.y);
    constexpr Size FRAGS_AREA_SIZE = MAXIMUM_FRAGS_AREA_SIZE - MARGIN*2;
    constexpr Size FRAGS_AREA_TL = MARGIN;
    constexpr Size FRAGS_AREA_BR = FRAGS_AREA_TL + FRAGS_AREA_SIZE;

    constexpr Size MAXIMUM_RIGHT_BAR_AREA_SIZE(SCENE_SIZE.x - MAXIMUM_FRAGS_AREA_SIZE.x, SCENE_SIZE.y);
    constexpr Size RIGHT_BAR_AREA_SIZE = MAXIMUM_RIGHT_BAR_AREA_SIZE - MARGIN*2;
    constexpr Size RIGHT_BAR_AREA_TL = MARGIN + Size(MAXIMUM_FRAGS_AREA_SIZE.x, 0);
    constexpr Size RIGHT_BAR_AREA_BR = RIGHT_BAR_AREA_TL + RIGHT_BAR_AREA_SIZE;

    constexpr int MAX_DIV_NUM_Y = 16;
    constexpr int MAX_DIV_NUM_X = 16;
    constexpr Size MAX_DIV_NUM(MAX_DIV_NUM_X, MAX_DIV_NUM_Y);

    constexpr Vec3 WORST_VCOLOR(0, 163, 129);
    constexpr Vec3 BEST_VCOLOR(23,24,75);

    const Vec4 SBCS_VCOLOR(255,255,255,255);
    const Vec4 SBCE_VCOLOR(255,255,255,190);

    constexpr Color TEXT_ON_FRAG_COLOR(234, 244, 252);

    constexpr Color BACK_GROUND_COLOR(13, 0, 21);

    constexpr Color PLAY_BUTTON_COLOR(0, 163, 129);
    constexpr Color PAUSE_BUTTON_COLOR(158, 161, 163);
    constexpr Color REPEAT_BUTTON_COLOR(56, 180, 139);
    constexpr Color DO_NOT_REPEAT_BUTTON_COLOR(234, 244, 252);
}

namespace krGUI {
    bool seek_bar(double& value, const double min_val, const double max_val, const Vec2& s, const Vec2& t, const string& id) {
        static map<string, Transition> transitions;
        static map<string, bool> holding;

        const Vec2 v_pos = s + (t-s) * (value / (max_val - min_val));

        if (transitions.count(id) == 0) transitions[id] = Transition(0.1s, 0.1s);

        Line(s,v_pos).draw(LineStyle::RoundCap, 6, Palette::Red);
        Line(v_pos,t).draw(LineStyle::RoundCap, 6, Palette::Gray);

        const Circle circle(v_pos, 7);

        if (holding[id] && !MouseL.pressed()) holding[id] = false;
        if ((circle.scaled(2).mouseOver() && MouseL.down()) || holding[id]) {
            value = (double)(Cursor::Pos().x - s.x) / (t.x-s.x) * (max_val - min_val);
            chmin(value, max_val);
            chmax(value, min_val);
            holding[id] = true;
        }

        transitions[id].update(circle.scaled(2).mouseOver() || holding[id]);

        const double ratio = transitions[id].value();

        const Vec4 color = Constant::SBCS_VCOLOR*(1.-ratio) + Constant::SBCE_VCOLOR*ratio;
        circle.scaled(1. + 1.*ratio).draw(Color(color.x, color.y, color.z, color.w));

        return holding[id];
    }
}

struct ProblemSettings {
    Pos div_num;
    int selectable_times;
    int select_cost, swap_cost;
    ProblemSettings() : div_num(1,1), selectable_times(0), select_cost(INT_MAX), swap_cost(INT_MAX) {}
    bool load(const string& prob_dir_path) {
        Pos pix_num;
        ifstream ifs(prob_dir_path + "/prob.txt");
        if (!ifs) { *this = ProblemSettings(); return false; }
        Utility::input_from(ifs, div_num, selectable_times, select_cost, swap_cost, pix_num);
        if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
            cerr << "h_pix / h_div != w_pix / w_div" << endl;
        }
        return true;
    }
};

struct FragmentImages {
    Grid<Texture> textures;
    FragmentImages() : textures(Constant::MAX_DIV_NUM) {}
    bool load(const string& path, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        rep(i, div_num.y) rep(j,div_num.x) {
            stringstream ss;
            ss << hex << uppercase << j << i << ".jpg";
            textures[i][j] = Texture(Unicode::Widen(path) + U"/frags/" + Unicode::Widen(ss.str()));
            if (!textures[i][j]) return false;
        }
        return true;
    }
};

struct OriginalState {
    Grid<double> rotations;
    Grid<Pos> orig_pos;
    OriginalState() : rotations(Constant::MAX_DIV_NUM), orig_pos(Constant::MAX_DIV_NUM) {}
    bool load(const string& path, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        ifstream ifs(path + "/original_state.txt");
        if (!ifs) return false;
        string srotations;
        ifs >> srotations;
        rep(i, div_num.y) rep(j,div_num.x) {
            string s;
            ifs >> s;
            int idx = stoi(s, nullptr, 16);
            int x = idx / 16, y = idx % 16;
            orig_pos[y][x] = Pos(j,i);
            rotations[y][x] = (srotations[i*div_num.x+j]-'0')*90_deg;
        }
        return true;
    }
};

struct Procedures {
    struct SingleProcedure {
        Pos selected_pos;
        Direction dir;
        SingleProcedure() = default;
        SingleProcedure(Pos selected_pos, Direction dir) : selected_pos(selected_pos), dir(dir) {}
    };
    int swap_times;
    int max_istep;
    vector<vector<SingleProcedure>> procs;

    Procedures() : procs() {}
    bool load(const string& prob_dir_path, const ProblemSettings& settings) {
        ifstream ifs(prob_dir_path + "/procedure.txt");
        if (!ifs) { *this = Procedures(); return false; }

        const auto& div_num = settings.div_num;

        swap_times = 0;
        string rotations;
        ifs >> rotations;

        int n;
        ifs >> n;
        rep(_,n) {
            int m;
            string sidx, spath;
            ifs >> sidx >> m >> spath;
            int idx = stoi(sidx, nullptr, 16);
            Pos selected_pos(idx/16, idx%16);
            Path path = Path::from_string(spath);
            vector<SingleProcedure> tmp;
            for(const auto dir : path) {
                tmp.emplace_back(selected_pos, dir);
                selected_pos.move_toraly(dir, div_num);
            }
            procs.push_back(tmp);
            swap_times += path.size();
        }
        max_istep = procs.size() + swap_times + 1;
        return true;
    }

    tuple<int, int, int, int> get_progress(int istep) const {
        const int n = procs.size();
        int m = 0;
        if (istep == 0) return {-1, n, 0, m};
        istep--;
        rep(i,procs.size()) {
            m = procs[i].size();
            if (istep <= m) return {i, n, istep, m};
            istep -= procs[i].size()+1;
        }
        return {n, n, 0, 0};
    }
};

enum Progress { failed, initial, restoration, procedure };

struct Context {
    int score;
    Progress progress;
    string prob_hash;
    string prob_dir_path;
    ProblemSettings settings;
    FragmentImages frags;
    OriginalState orig_state;
    Procedures procedures;

    void init(const string& path) {
        prob_dir_path = path;
        prob_hash = filesystem::path(path).filename();
        reload();
    }
    void reload() {
        if (!settings.load(prob_dir_path) || !frags.load(prob_dir_path, settings)) progress = Progress::failed;
        else if (!orig_state.load(prob_dir_path, settings)) progress = Progress::initial;
        else if (!procedures.load(prob_dir_path, settings)) progress = Progress::restoration;
        else progress = Progress::procedure;
    }
};

using App = SceneManager<String, Context>;

class InitialSetting : public App::Scene {
public:

    InitialSetting(const InitData& init) : IScene(init) {}

    void update() override {}
    void draw() const override {}
};

class Menu : public App::Scene {
public:

    Menu(const InitData& init) : IScene(init) {}

    void update() override {}
    void draw() const override {}
};

tuple<Vec2, Vec2, int> get_TL_BR_length_for_frags(const Pos& div_num) {
    constexpr Vec2 AREA_TL = Constant::FRAGS_AREA_TL;
    constexpr Vec2 AREA_BR = Constant::FRAGS_AREA_BR;
    constexpr int w = Constant::FRAGS_AREA_BR.x - Constant::FRAGS_AREA_TL.x;
    constexpr int h = Constant::FRAGS_AREA_BR.y - Constant::FRAGS_AREA_TL.y;
    const int frag_length = min(w/div_num.x, h/div_num.y);
    const Vec2 space(w-frag_length*div_num.x, h-frag_length*div_num.y);

    const Vec2 TL = AREA_TL + space/2;
    const Vec2 BR = AREA_BR - space/2;

    return { TL, BR, frag_length };
}

// todo: save as text, sent to server
class ImageRestorer : public App::Scene {
    struct State {
        inline static int cum_z_idx;
        Vec2 pos;
        int z_idx;
        double rotation;
    };
    Grid<State> pos;
    optional<Pos> moving;

    void draw_frag() const {
        const auto& div_num = getData().settings.div_num;
        const auto& frags = getData().frags.textures;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);
        const Size size(frag_length, frag_length);
        vector<pair<int, Pos>> v;
        rep(i,div_num.y) rep(j,div_num.x) v.emplace_back(pos[i][j].z_idx, Pos(j,i));
        sort(all(v));
        for(const auto& a : v) {
            const int i = a.second.y, j = a.second.x;
            frags[i][j].resized(size).rotated(pos[i][j].rotation).drawAt(pos[i][j].pos);
        }
    }
    void draw_info() const {
        const auto& settings = getData().settings;
        const auto& div_num = settings.div_num;

        const Array<String> info_arr {
            U"hash         : {}"_fmt(Unicode::Widen(getData().prob_hash)),
            U"mode         : Restoration(manual)",
            U"div num      : {}x{}"_fmt(div_num.x, div_num.y),
            };

        const Array<int> h_offset {
            0,
            100,
            200,
            };

        const int w = 1100, h = 800;
        rep(i,info_arr.size()) {
            FontAsset(U"info")(info_arr[i]).draw(Arg::topLeft = Vec2(w,h+h_offset[i]),Palette::White);
        }
    };
    void move_and_rotate_frag() {
        static Vec2 offset;
        if (MouseL.up()) moving = nullopt;

        const auto& div_num = getData().settings.div_num;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);

        const bool rotation_flag = MouseR.down() || (KeyR.down() && !KeyCommand.pressed() && !KeyControl.pressed());
        const bool move_flag = MouseL.down();
        if (rotation_flag || move_flag) {
            optional<Pos> opp;
            rep(i,div_num.y) rep(j,div_num.x) {
                const Point center((int)pos[i][j].pos.x, (int)pos[i][j].pos.y);
                if (Rect(Arg::center(center), frag_length, frag_length).mouseOver()) {
                    if (opp) {
                        const auto& p = opp.value();
                        if(pos[p.y][p.x].z_idx < pos[i][j].z_idx) opp = Pos(j,i);
                    } else opp = Pos(j,i);
                }
            }
            if (opp) {
                const auto& p = opp.value();
                if (rotation_flag) pos[p.y][p.x].rotation += 90_deg;
                else {
                    moving = opp;
                    pos[p.y][p.x].z_idx = State::cum_z_idx++;
                    offset = pos[p.y][p.x].pos - Vec2(Cursor::Pos().x, Cursor::Pos().y);
                }
            }
        }

        if (moving) {
            const auto& p = moving.value();
            if (KeyShift.pressed()) {
                Vec2 cp = Cursor::Pos() - TL;
                int y = cp.y / frag_length, x = cp.x / frag_length;
                pos[p.y][p.x].pos = Vec2(x+0.5,y+0.5)*frag_length + TL;
            } else  pos[p.y][p.x].pos = Cursor::Pos() + offset;
            const auto size = Scene::Size();
            chmax(pos[p.y][p.x].pos.x, 0.);
            chmin(pos[p.y][p.x].pos.x, (double)size.x);
            chmax(pos[p.y][p.x].pos.y, 0.);
            chmin(pos[p.y][p.x].pos.y, (double)size.y);
        }
    }

    void align_pos() {
        const auto& div_num = getData().settings.div_num;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);
        optional<State> moving_state = nullopt;
        if (moving) moving_state = pos[moving.value().y][moving.value().x];
        vector<pair<double, pair<Pos, Pos>>> v;
        rep(i,div_num.y) rep(j,div_num.x) {
            const auto& p1 = pos[i][j].pos;
            rep(y,div_num.y) rep(x,div_num.x) {
                const auto& p2 = TL + Vec2(x+0.5, y+0.5)*frag_length;
                const double mh_dist = abs(p1.x-p2.x) + abs(p1.y-p2.y);
                const double penalty = mh_dist * mh_dist;
                v.emplace_back(penalty, make_pair(Pos(j,i),Pos(x,y)));
            }
        }
        sort(all(v));
        rep(i,div_num.y) rep(j,div_num.x) pos[i][j].pos = Vec2(-1,-1);
        set<Pos> s;
        rep(i,v.size()) {
            const Pos p1 = v[i].second.first;
            const Pos p2 = v[i].second.second;

            if (pos[p1.y][p1.x].pos.x < 0 && s.count(p2) == 0) {
                pos[p1.y][p1.x].pos = TL + Vec2(p2.x+0.5,p2.y+0.5) * frag_length;
                s.insert(p2);
                PRINT(p1,p2);
            }
        }
        if (moving_state) pos[moving.value().y][moving.value().x] = moving_state.value();
    }

public:

    ImageRestorer(const InitData& init) : IScene(init), pos(Constant::MAX_DIV_NUM), moving(nullopt) {
        const auto& progress = getData().progress;
        const auto& div_num = getData().settings.div_num;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);

        if (progress == Progress::initial) {
            rep(i,div_num.y) rep(j,div_num.x) {
                const int y = i, x = j;
                pos[i][j].pos = TL + (Vec2(x,y) + Vec2(0.5,0.5))*frag_length;
                pos[i][j].z_idx = State::cum_z_idx++;
                pos[i][j].rotation = 0;
            }
        } else {
            const auto& orig_state = getData().orig_state;
            rep(i,div_num.y) rep(j,div_num.x) {
                const int x = orig_state.orig_pos[i][j].x, y = orig_state.orig_pos[i][j].y;
                pos[i][j].pos = TL + (Vec2(x,y) + Vec2(0.5,0.5))*frag_length;
                pos[i][j].z_idx = State::cum_z_idx++;
                pos[i][j].rotation = orig_state.rotations[i][j];
            }
        }
    }

    void update() override {
        if (KeyE.down()) changeScene(U"RestorationVisualizer");
        if (KeyTab.down()) changeScene(U"ProcedureVisualizer");
        if (KeyA.down()) align_pos();
        move_and_rotate_frag();
    }
    void draw() const override {
        draw_info();
        draw_frag();
    }
};

class RestorationVisualizer : public App::Scene {
    void draw_frag() const {
        const auto& div_num = getData().settings.div_num;
        const auto& frags = getData().frags.textures;
        const auto& orig_state = getData().orig_state;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);

        const Size size(frag_length, frag_length);
        rep(i,div_num.y) rep(j,div_num.x) {
            const int J = orig_state.orig_pos[i][j].x, I = orig_state.orig_pos[i][j].y;
            const Vec2 pos = TL + Vec2(J+0.5,I+0.5)*frag_length;
            frags[i][j].resized(size).rotated(orig_state.rotations[i][j]).drawAt(pos);
        }
    }
    void draw_info() const {
        const auto& settings = getData().settings;
        const auto& div_num = settings.div_num;

        const Array<String> info_arr {
            U"hash         : {}"_fmt(Unicode::Widen(getData().prob_hash)),
            U"mode         : Restoration(program)",
            U"div num      : {}x{}"_fmt(div_num.x, div_num.y),
        };

        const Array<int> h_offset {
            0,
            100,
            200,
        };

        const int w = 1100, h = 500;
        rep(i,info_arr.size()) {
            FontAsset(U"info")(info_arr[i]).draw(Arg::topLeft = Vec2(w,h+h_offset[i]),Palette::White);
        }
    }

public:

    RestorationVisualizer(const InitData& init) : IScene(init) {}

    void update() override {
        if (KeyE.down()) changeScene(U"ImageRestorer");
        if (KeyTab.down()) changeScene(U"ProcedureVisualizer");
    }
    void draw() const override {
        draw_frag();
        draw_info();
    }
};

namespace PlaybackMenu {
    namespace Impl {
        int now_step = 0;
        int max_step = 0;
        bool holding = false;
        bool loop_playback = false;
        double seek_bar_rate = 0;
        double playback_speed = 1.;
        double playback_speed_magnification = 1.;
        double cum_diff_step = 0;
        enum { Play, Reverse, Pause } prev_type = Play, play_type = Pause;

        constexpr Size TL = Constant::RIGHT_BAR_AREA_TL + Size(0, Constant::RIGHT_BAR_AREA_SIZE.y * 6/7);
        constexpr Size BR = Constant::RIGHT_BAR_AREA_BR;

        void draw_seek_bar() {
            const int h = (BR.y-TL.y);
            const int w = (BR.x-TL.x);
            const Vec2 s(TL.x + w/8.4, TL.y+h/2), t(BR.x-w/30, TL.y+h/2);
            string id = "001";

            seek_bar_rate = (double)now_step / max_step;
            holding = krGUI::seek_bar(seek_bar_rate, 0, 1., s, t, id);
        }
        void draw_playback_button() {
            const int h = (BR.y-TL.y);
            const int w = (BR.x-TL.x);

            // draw playback state
            if (play_type == Pause) {
                const Size size(30,30);
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);

                if (prev_type == Play) TextureAsset(U"play").resized(size).drawAt(pos, Constant::PLAY_BUTTON_COLOR);
                else TextureAsset(U"play").resized(size).rotated(180_deg).drawAt(pos, Constant::PLAY_BUTTON_COLOR);
            } else {
                constexpr Size size(30,30);
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);

                TextureAsset(U"pause").resized(size).drawAt(pos, Constant::PAUSE_BUTTON_COLOR);
            }

            // repeat
            {
                const Point pos(TL.x+w/56, TL.y + h/2 + 3);

                if (loop_playback) {
                    FontAsset(U"repeat")(U"🔁").drawAt(pos, Constant::REPEAT_BUTTON_COLOR);
                    Circle(pos.x, pos.y+27, 4).draw(Constant::REPEAT_BUTTON_COLOR);
                } else FontAsset(U"repeat")(U"🔁").drawAt(pos, Constant::DO_NOT_REPEAT_BUTTON_COLOR);
            }
        }
        void update_playback_button() {
            const int h = (BR.y-TL.y);
            const int w = (BR.x-TL.x);

            if (holding) play_type = Pause;

            if (!loop_playback && (now_step == 0 || now_step == max_step) && play_type != Pause) prev_type = play_type, play_type = Pause;

            // change playback state by key
            if (KeySpace.down()) {
                if (play_type == Pause) play_type = prev_type;
                else prev_type = play_type, play_type = Pause;
            } else if (KeyR.down() && !KeyCommand.pressed() && !KeyControl.pressed()) {
                prev_type = play_type = (prev_type == Play ? Reverse : Play);
            }

            // change playback state by mouse
            if (play_type == Pause) {
                constexpr Size size(30,30);
                constexpr Size h_size = size/2;
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);

                const Rect rect(pos-h_size, size);
                if (rect.leftClicked()) play_type = prev_type;
                else if (rect.rightClicked()) prev_type = play_type = (prev_type == Play ? Reverse : Play);
            } else {
                constexpr Size size(30,30);
                constexpr Size h_size = size/2;
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);
                const Point trp1 = pos - h_size;
                const Point trp2(pos.x-h_size.x, pos.y+h_size.y);
                const Point trp3(pos.x+h_size.x, pos.y);

                Triangle tr;
                if (play_type == Play) tr = Triangle(trp1, trp2, trp3);
                else tr = Triangle(trp1, trp2, trp3).rotatedAt(pos.x, pos.y, 180_deg);

                if (tr.leftClicked()) prev_type = play_type, play_type = Pause;
                else if (tr.rightClicked()) prev_type = play_type = (prev_type == Play ? Reverse : Play);
            }

            // repeat
            {
                const Point pos(TL.x+w/56, TL.y + h/2 + 3);
                Rect rect(pos.x - 20, pos.y - 15, 40, 28);
                if (rect.leftClicked() || KeyC.down()) loop_playback = !loop_playback;
            }
        }

        void update_speed() {
            if (Key1.down()) Impl::playback_speed = 0.2;
            else if (Key2.down()) Impl::playback_speed = 0.4;
            else if (Key3.down()) Impl::playback_speed = 0.6;
            else if (Key4.down()) Impl::playback_speed = 0.8;
            else if (Key5.down()) Impl::playback_speed = 1.;
            else if (Key6.down()) Impl::playback_speed = 1.2;
            else if (Key7.down()) Impl::playback_speed = 1.4;
            else if (Key8.down()) Impl::playback_speed = 1.6;
            else if (Key9.down()) Impl::playback_speed = 1.8;
            else if (Key0.down()) Impl::playback_speed = 2.;

            if (KeyLShift.down()) {
                auto& mag = Impl::playback_speed_magnification;
                mag *= 10.;
                if (mag > 11.) mag = 0.1;
            }
        }

        void cumulate_diff_step_by_key() {
            if (KeyL.down()) cum_diff_step = (int)cum_diff_step+1*max(1.,playback_speed_magnification);
            if (KeyH.down()) cum_diff_step = (int)cum_diff_step-1*max(1.,playback_speed_magnification);
            if (KeyK.pressed()) cum_diff_step += playback_speed * playback_speed_magnification;
            if (KeyJ.pressed()) cum_diff_step -= playback_speed * playback_speed_magnification;

            if (play_type != Pause && (KeyL.down() || KeyH.down() || KeyK.pressed() || KeyJ.pressed())) {
                prev_type = play_type;
                play_type = Pause;
            }
        }
        void cumulate_diff_step_by_playback_button() {
            int play_dir = (play_type == Pause ? 0 : (play_type == Play ? 1 : -1));
            cum_diff_step += play_dir * playback_speed * playback_speed_magnification;
        }
        void cumulate_diff_step_by_seek_bar() {
            cum_diff_step += seek_bar_rate * max_step - now_step;
        }

        void update_step() {
            int diff_step = static_cast<int>(cum_diff_step);
            now_step += diff_step;
            cum_diff_step -= diff_step;
            if (loop_playback) {
                if (now_step > max_step) now_step %= (max_step+1);
                if (now_step < 0) now_step += (-now_step + max_step) / (max_step+1) * (max_step+1);
            } else {
                chmin(now_step, max_step);
                chmax(now_step, 0);
            }
        }
    }

    void init(const Context& ctx) {
        Impl::now_step = 0;
        Impl::max_step = ctx.procedures.max_istep;
        Impl::holding = false;
        Impl::loop_playback = false;
        Impl::seek_bar_rate = 0;
        Impl::playback_speed = 1.;
        Impl::playback_speed_magnification = 1.;
        Impl::cum_diff_step = 0;
        Impl::prev_type = Impl::Play;
        Impl::play_type = Impl::Pause;
    }

    int get_now_step() { return Impl::now_step; }

    void update_step() {
        Impl::update_speed();
        Impl::update_playback_button();

        Impl::cumulate_diff_step_by_key();
        Impl::cumulate_diff_step_by_playback_button();
        Impl::cumulate_diff_step_by_seek_bar();

        Impl::update_step();
    }

    void draw() {
        Impl::draw_seek_bar();
        Impl::draw_playback_button();
    }
}
class ProcedureVisualizer : public App::Scene {
    struct DrawType {
        enum FragType { Image, Color } frag_type;
        enum InfoType { None, Dist, Pos } info_type;
        DrawType() : frag_type(Image), info_type(None) {}
        DrawType(FragType frag_type, InfoType info_type) : frag_type(frag_type), info_type(info_type) {}
    };
    int now_step;
    Pos now_selected_pos;
    Grid<Pos> now_orig_pos, rev_now_orig_pos;
    DrawType draw_type;

    double calc_accuracy(const Pos& p) const {
        const auto& div_num = getData().settings.div_num;
        constexpr double coef = 0.2;
        int penalty = p.calc_mh_dist_toraly(now_orig_pos[p.y][p.x], div_num);
        return 1.-exp(-(double)penalty*coef);
    }
    Color get_color(const Pos& p) const {
        constexpr Vec3 worst_color = Constant::WORST_VCOLOR;
        constexpr Vec3 best_color = Constant::BEST_VCOLOR;
        constexpr Vec3 d_color = worst_color - best_color;
        const double accuracy = calc_accuracy(p);
        const Vec3 v_color = best_color + d_color*accuracy;
        return Color(v_color.x, v_color.y, v_color.z);
    }

    void update_draw_type() {
        if (KeyZ.down()) {
            if (draw_type.frag_type == DrawType::Image)
                draw_type.frag_type = DrawType::Color;
            else draw_type.frag_type = DrawType::Image;
        }
        if (KeyX.down()) {
            if (draw_type.info_type == DrawType::None)
                draw_type.info_type = DrawType::Dist;
            else if (draw_type.info_type == DrawType::Dist)
                draw_type.info_type = DrawType::Pos;
            else draw_type.info_type = DrawType::None;
        }
    }
    void transition() {
        const auto& div_num = getData().settings.div_num;
        const auto& procs = getData().procedures;
        const int next_step = PlaybackMenu::get_now_step();
        const int diff_step = next_step - now_step;
        auto [iselect, n, iswap, m] = procs.get_progress(now_step);
        now_step += diff_step;
        if (diff_step < 0) {
            for(int th=-1; th>=diff_step; --th) {
                iswap--;

                // select
                if (iswap == -1) {
                    iselect--;
                    if (iselect == -1) { now_selected_pos = Pos(-1,-1); return; }
                    iswap = procs.procs[iselect].size();
                    now_selected_pos = procs.procs[iselect].back().selected_pos;
                    now_selected_pos.move_toraly(procs.procs[iselect].back().dir, div_num);
                    continue;
                }

                // swap
                const auto& sp = procs.procs[iselect][iswap];
                const Pos prev = sp.selected_pos;
                const Pos now = prev.get_moved_toraly_pos(sp.dir, div_num);
                swap(now_orig_pos[now.y][now.x], now_orig_pos[prev.y][prev.x]);
                swap(rev_now_orig_pos[now.y][now.x], rev_now_orig_pos[prev.y][prev.x]);
                now_selected_pos = prev;
            }
        } else if (diff_step > 0) {
            for (int th=1; th<=diff_step; ++th) {
                // select
                if (iselect == -1 || iswap == (int)procs.procs[iselect].size()) {
                    iselect++;
                    iswap = 0;
                    if (iselect == n) { now_selected_pos = Pos(-1,-1); return; }
                    now_selected_pos = procs.procs[iselect][iswap].selected_pos;
                    continue;
                }

                // swap
                const auto& sp = procs.procs[iselect][iswap];
                const auto& now = sp.selected_pos;
                const Pos next = now.get_moved_toraly_pos(sp.dir, div_num);
                swap(now_orig_pos[now.y][now.x], now_orig_pos[next.y][next.x]);
                swap(rev_now_orig_pos[now.y][now.x], rev_now_orig_pos[next.y][next.x]);
                now_selected_pos = next;
                iswap++;
            }
        }
    }

    void draw_progress() const {
        constexpr Size TL = Constant::RIGHT_BAR_AREA_TL + Size(0, Constant::RIGHT_BAR_AREA_SIZE.y * 6/7);
        constexpr Size BR = Constant::RIGHT_BAR_AREA_BR;
        const auto& procs = getData().procedures;
        int N = procs.max_istep;
        auto [a,n,b,m] = procs.get_progress(now_step);
        FontAsset(U"progress")(U"progress: {:>3}/{:<3}, {:>5}/{:<5}, {:>5}/{:<5}({:>3}.{}[%])"_fmt(a,n,b,m,now_step,N,now_step*100/N,(now_step*1000/N)%10))
        .drawAt(TL.x+475, BR.y-35, Palette::White);
    }
    void draw_frags() const {
        const auto& div_num = getData().settings.div_num;
        const auto& frags = getData().frags.textures;
        const auto& orig_state = getData().orig_state;
        const auto& [TL, BR, frag_length] = get_TL_BR_length_for_frags(div_num);

        const Size size(frag_length, frag_length);
        const Size h_size = size / 2;

        rep(i,div_num.y) rep(j,div_num.x) {
            double offset = 0;
            if (now_selected_pos == Pos(j,i)) offset = Periodic::Sine0_1(1.7s) * frag_length/8. + frag_length/10.;
            const int x = rev_now_orig_pos[i][j].x, y = rev_now_orig_pos[i][j].y;
            const Vec2 tmpTL = TL + Vec2(j,i)*size - Vec2(offset, offset);
            const Vec2 center = tmpTL + h_size;

            if (draw_type.frag_type == DrawType::Image) frags[y][x].resized(size).rotated(orig_state.rotations[y][x]).drawAt(center);
            else Rect(tmpTL.x, tmpTL.y, size-Size(2,2)).rounded(3).draw(get_color(Pos(j,i)));

            if (draw_type.info_type != DrawType::None) {
                const Pos& p = now_orig_pos[i][j];
                stringstream ss;
                if (draw_type.info_type == DrawType::Dist) {
                    int dist = Pos(j,i).calc_mh_dist_toraly(p, div_num);
                    if (dist != 0) ss << dist;
                }
                else ss << uppercase << hex << p.x << p.y;

                FontAsset(U"TextOnFrag")(Unicode::Widen(ss.str())).drawAt(center-Vec2(1,1), Constant::TEXT_ON_FRAG_COLOR);
            }
        }
    }
    void draw_info() const {
        const auto& settings = getData().settings;
        const auto& procs = getData().procedures;
        const auto& div_num = settings.div_num;

        const int select_times = procs.procs.size();
        const int select_cost = select_times * settings.select_cost;
        const int swap_times = procs.swap_times;
        const int swap_cost = swap_times * settings.swap_cost;
        const int total_cost = select_cost + swap_cost;

        const Array<String> info_arr {
            U"hash         : {}"_fmt(Unicode::Widen(getData().prob_hash)),
            U"mode         : Procedure(program)",
            U"div num      : {}x{}"_fmt(div_num.x, div_num.y),
            U"select times : {} / {}"_fmt(select_times, settings.selectable_times),
            U"select costs :    x{} = {}"_fmt(settings.select_cost, select_cost),
            U"swap   times : {}"_fmt(swap_times),
            U"swap   costs :    x{} = {}"_fmt(settings.swap_cost, swap_cost),
            U"total  costs : {}"_fmt(total_cost),
            };

        const Array<int> h_offset {
            0,
            100,
            200,
            300,
            350,
            450,
            500,
            600
        };

        const int w = 1100, h = 200;
        rep(i,info_arr.size()) {
            FontAsset(U"info")(info_arr[i]).draw(Arg::topLeft = Vec2(w,h+h_offset[i]),Palette::White);
        }
    }

public:

    ProcedureVisualizer(const InitData& init) : IScene(init), now_step(0), now_selected_pos(-1,-1), now_orig_pos(Constant::MAX_DIV_NUM), rev_now_orig_pos(Constant::MAX_DIV_NUM), draw_type() {
        PlaybackMenu::init(getData());
        const auto& div_num = getData().settings.div_num;
        const auto& orig_pos = getData().orig_state.orig_pos;
        rep(i,div_num.y) rep(j,div_num.x) {
            const int y = orig_pos[i][j].y, x = orig_pos[i][j].x;
            now_orig_pos[i][j] = Pos(x,y);
            rev_now_orig_pos[i][j] = Pos(j,i);
        }
    }

    void update() override {

        if (KeyTab.down()) changeScene(U"RestorationVisualizer");

        update_draw_type();
        PlaybackMenu::update_step();
        transition();
    }
    void draw() const override {
        PlaybackMenu::draw();
        draw_progress();
        draw_frags();
        draw_info();
    }
};

void drawFPS() {
    static queue<double> elapseds;

    const int width = Scene::Width();
    const Vec2 pos(width-40, 15);
    const double elapsed = Scene::Time();
    elapseds.push(elapsed);
    const double fps = (double)elapseds.size() / (elapsed - elapseds.front());
    FontAsset(U"fps")(U"{:.3f}"_fmt(fps)).drawAt(pos, Palette::White);
    if (elapseds.size() >= 60) elapseds.pop();
}

void adjust_window_size_if_necessary() {
    const int h = Window::ClientHeight(), w = Window::ClientWidth();
    if (abs(h*Constant::WINDOW_RATIO.x - w*Constant::WINDOW_RATIO.y) > 10) {
        Size size((int)(h*Constant::WINDOW_RATIO.x / Constant::WINDOW_RATIO.y), h);
        Window::Resize(size, WindowResizeOption::KeepSceneSize, false);
    }
}

void Main() {
    Scene::SetBackground(Constant::BACK_GROUND_COLOR);

    // ウィンドウを手動リサイズ可能にする
    Window::SetStyle(WindowStyle::Sizable);

    Scene::Resize(1920, 1080);
    Window::Resize(1760, 990, WindowResizeOption::KeepSceneSize, false);
    Window::SetPos(-1900, -40);

    FontAsset::Register(U"TitleFont", 60, Typeface::Heavy);
    FontAsset::Register(U"ScoreFont", 30, Typeface::Bold);

    // シーンマネージャーを作成
    // ここで Context も初期化される
    App manager;

    manager.add<RestorationVisualizer>(U"RestorationVisualizer");
    manager.add<ProcedureVisualizer>(U"ProcedureVisualizer");
    manager.add<ImageRestorer>(U"ImageRestorer");


    manager.get()->score = 100000;
//    manager.get()->init("/Users/rk/Projects/procon32/.tmp_data/tmp");
//    manager.get()->init("/Users/rk/Projects/procon32/.tmp_data/tmp");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/40211b699f29d8b5077e1cc86a7f4f9d");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/4b119be5dbf5a0ececf00f506f5459b7");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/4eb0b7e9750cf7aad51a8cf11cc45131");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/5f58a788475f3c05f7295ebe82a3dc1e");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/66b9d56c5ccb3e886de0fbc1ae6ba2b3");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/9359558aafa91b63d7575b0aad245780");
    manager.get()->init("/Users/rk/Projects/procon32/.data/a89ea53bb8969322a6653a2a79b83a3b");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/a9df29717a478f25cec69f626bdd9bff");
//    manager.get()->init("/Users/rk/Projects/procon32/.data/ba8d67aa3b80cfc934a9ee9b114cbd61");


    // set font
    if (ifstream ifs("../Menlo.ttc"); ifs) {
        FontAsset::Register(U"fps",        20, U"../Menlo.ttc");
        FontAsset::Register(U"repeat",     40, U"../Menlo.ttc");
        FontAsset::Register(U"progress",   22, U"../Menlo.ttc");
        FontAsset::Register(U"info",       28, U"../Menlo.ttc");
        if (ifstream ifs_("../Klee.ttc"); ifs_) FontAsset::Register(U"TextOnFrag", 30, U"../Klee.ttc");
        else FontAsset::Register(U"TextOnFrag", 28);
    } else {
        FontAsset::Register(U"fps",        20);
        FontAsset::Register(U"info",       30);
        FontAsset::Register(U"repeat",     40);
        FontAsset::Register(U"progress",   22);
        FontAsset::Register(U"TextOnFrag", 28);
    }

    TextureAsset::Register(U"redo", Icon(0xf2f9, 100));
    TextureAsset::Register(U"play", Icon(0xf04b, 100));
    TextureAsset::Register(U"pause", Icon(0xf04c, 100));

    while (System::Update()) {
        // window resize if necessary
        adjust_window_size_if_necessary();

        // 現在のシーンを実行
        if (!manager.update()) break;

        // 右上にfpsを描画
        drawFPS();
    }
}
