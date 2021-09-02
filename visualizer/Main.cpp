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
    constexpr Pos operator*(int s) const { return { x*s, y*s }; }
    constexpr Pos operator/(int s) const { return { x/s, y/s }; }

    constexpr Pos& operator+=(const Pos& another) { *this = *this+another; return *this; }
    constexpr Pos& operator-=(const Pos& another) { *this = *this-another; return *this; }
    constexpr Pos& operator*=(const Pos& another) { *this = *this*another; return *this; }
    constexpr Pos& operator*=(int s) { *this = *this*s; return *this; }
    constexpr Pos& operator/=(int s) { *this = *this/s; return *this; }

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
    void move_toraly(const Path& path, const Pos& area) { for(const auto& dir : path) move_toraly(dir, area); }


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
    constexpr Size RIGHT_BAR_RATIO(6, 1);

    constexpr Size SCENE_SIZE(1920, 1080);
    constexpr Size MARGIN(10,10);
    constexpr Size ACTUAL_FRAGS_AREA_SIZE(SCENE_SIZE.y, SCENE_SIZE.y);

    constexpr Size FRAGS_AREA_SIZE = ACTUAL_FRAGS_AREA_SIZE - MARGIN*2;
    constexpr Size FRAGS_AREA_TL = MARGIN;
    constexpr Size FRAGS_AREA_BR = FRAGS_AREA_TL + FRAGS_AREA_SIZE;

    constexpr Size ACTUAL_RIGHT_BAR_AREA_SIZE(SCENE_SIZE.x - ACTUAL_FRAGS_AREA_SIZE.x, SCENE_SIZE.y);

    constexpr Size RIGHT_BAR_AREA_SIZE = ACTUAL_RIGHT_BAR_AREA_SIZE - MARGIN*2;
    constexpr Size RIGHT_BAR_AREA_TL = MARGIN + Size(ACTUAL_FRAGS_AREA_SIZE.x, 0);
    constexpr Size RIGHT_BAR_AREA_BR = RIGHT_BAR_AREA_TL + RIGHT_BAR_AREA_SIZE;

    constexpr int MAX_DIV_NUM_Y = 16;
    constexpr int MAX_DIV_NUM_X = 16;
    constexpr Pos DIV_NUM(MAX_DIV_NUM_X, MAX_DIV_NUM_Y);


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

void move_toraly(Pos& p, const Pos& dp, const Pos& div_num) {
    p += dp;
    p.x = (p.x + div_num.x) % div_num.x;
    p.y = (p.y + div_num.y) % div_num.y;
}
void move_toraly(Pos& p, const Direction& dir, const Pos& div_num) {
    move_toraly(p, Pos(dir), div_num);
}
Pos get_moved_toraly_pos(Pos p, const Pos& dp, const Pos& div_num) {
    move_toraly(p, dp, div_num);
    return p;
}
Pos get_moved_toraly_pos(Pos p, const Direction& dir, const Pos& div_num) {
    move_toraly(p, dir, div_num);
    return p;
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
    int frag_size;
    int selectable_times;
    int select_cost, swap_cost;
    ProblemSettings() : div_num(1,1), frag_size(0), selectable_times(0), select_cost(INT_MAX), swap_cost(INT_MAX) {}
    bool load(const string& prob_dir_path) {
        Pos pix_num;
        ifstream ifs(prob_dir_path + "/prob.txt");
        if (!ifs) { *this = ProblemSettings(); return false; }
        Utility::input_from(ifs, div_num, selectable_times, select_cost, swap_cost, pix_num);
        if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
            cerr << "h_pix / h_div != w_pix / w_div" << endl;
            exit(-1);
        }
        frag_size = pix_num.y / div_num.y;
        return true;
    }
};

class FragmentImages {
    Grid<double> rotations;
    Grid<Texture> frags;

public:
    FragmentImages() : rotations(Constant::MAX_DIV_NUM_X, Constant::MAX_DIV_NUM_Y), frags(Constant::MAX_DIV_NUM_X, Constant::MAX_DIV_NUM_Y) {}
    bool load(const string& prob_dir_path, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        ifstream ifs(prob_dir_path + "/original_state.txt");
        if (!ifs) {
            *this = FragmentImages();
            rep(i, div_num.y) rep(j,div_num.x) {
                stringstream ss;
                ss << hex << uppercase << j << i << ".jpg";
                frags[i][j] = Texture(Unicode::Widen(prob_dir_path) + U"/frags/" + Unicode::Widen(ss.str()));
            }
            return false;
        }
        string srotations;
        ifs >> srotations;
        rep(i, div_num.y) rep(j,div_num.x) {
            string s;
            ifs >> s;
            int idx = stoi(s, nullptr, 16);
            int x = idx / 16, y = idx % 16;

            stringstream ss;
            ss << hex << uppercase << x << y << ".jpg";
            frags[i][j] = Texture(Unicode::Widen(prob_dir_path) + U"/frags/" + Unicode::Widen(ss.str()));

            rotations[i][j] = (srotations[i*div_num.x+j]-'0')*90_deg;
        }
        return true;
    }
    void draw(const Pos& idx, const Vec2& size, const Vec2& TL) const {
        frags[idx.y][idx.x].resized(size).rotated(rotations[idx.y][idx.x]).draw(TL);
    }
};

struct SingleProcedure {
    Pos selected_pos;
    Direction dir;

    SingleProcedure(Pos selected_pos) : selected_pos(selected_pos), dir() {}
    SingleProcedure(Pos selected_pos, Direction dir) : selected_pos(selected_pos), dir(dir) {}
};

class Procedures {
    int swap_times;
    int max_istep;
public:
    vector<vector<SingleProcedure>> procs;

    Procedures() : procs() {}
    bool load(const string& prob_dir_path, const ProblemSettings& settings) {
        ifstream ifs(prob_dir_path + "/procedure.txt");
        if (!ifs) { *this = Procedures(); return false; }

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
            vector<SingleProcedure> tmp_procs;
            for(const auto& dir : path) {
                tmp_procs.emplace_back(selected_pos, dir);
                move_toraly(selected_pos, dir, settings.div_num);
            }
            procs.push_back(tmp_procs);
            swap_times += path.size();
        }
        max_istep = get_istep(make_pair(procs.size(), 0));
        return true;
    }

    int get_istep(pair<int,int> now_step) const {
        if (now_step.first == -1) return 0;
        int istep = 0;
        rep(i,now_step.first) {
            istep++;
            istep += procs[i].size();
        }
        istep++;
        istep += now_step.second;
        return istep;
    }
    int get_max_istep() const { return max_istep; }
    int get_swap_times() const { return swap_times; }
};

class BoardState {
    pair<int,int> now_step;
    Pos now_selected_pos;
    Grid<Pos> now_orig_pos;

    int calc_mh_dist(const Pos& p, const Pos& div_num) const {
        const auto& orig_pos = now_orig_pos[p.y][p.x];
        const int x_dist = min(div_num.x - abs(p.x-orig_pos.x), abs(p.x-orig_pos.x));
        const int y_dist = min(div_num.y - abs(p.y-orig_pos.y), abs(p.y-orig_pos.y));
        const int mh_dist = x_dist + y_dist;
        return mh_dist;
    }
    double calc_accuracy(const Pos& p, const Pos& div_num) const {
        constexpr double coef = 0.2;
        int penalty = calc_mh_dist(p, div_num);
        return 1.-exp(-(double)penalty*coef);
    }
    Color get_color(const Pos& p, const Pos& div_num) const {
        constexpr Vec3 worst_color = Constant::WORST_VCOLOR;
        constexpr Vec3 best_color = Constant::BEST_VCOLOR;
        constexpr Vec3 d_color = worst_color - best_color;
        const double accuracy = calc_accuracy(p, div_num);
        const Vec3 v_color = best_color + d_color*accuracy;
        return Color(v_color.x, v_color.y, v_color.z);
    }

public:

    struct DrawType {
        enum FragType { Image, Color } frag_type;
        enum InfoType { None, Dist, Pos } info_type;
        DrawType() : frag_type(Color), info_type(None) {}
        DrawType(FragType frag_type, InfoType info_type) : frag_type(frag_type), info_type(info_type) {}
    };

    BoardState() : now_step(-1,0), now_selected_pos(-1,-1), now_orig_pos(Constant::MAX_DIV_NUM_X, Constant::MAX_DIV_NUM_Y) {
        rep(i,Constant::MAX_DIV_NUM_Y) {
            rep(j,Constant::MAX_DIV_NUM_X) {
                now_orig_pos[i][j] = Pos(j,i);
            }
        }
    }
    bool load(const string& prob_dir_path, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        ifstream ifs(prob_dir_path + "/original_state.txt");
        if (!ifs) { *this = BoardState(); return false; }
        string rotations;
        ifs >> rotations;
        rep(i, div_num.y) rep(j,div_num.x) {
            string s;
            ifs >> s;
            int idx = stoi(s, nullptr, 16);
            int x = idx / 16, y = idx % 16;
            now_orig_pos[y][x] = Pos(j,i);
        }
        return true;
    }

    int transition_to_end(const Procedures& procs, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        int n = procs.procs.size();
        if (now_step.first == n) return 0;
        for (int th=1;;++th) {
            // select
            if (now_step.first == -1 || now_step.second == (int)procs.procs[now_step.first].size()) {
                now_step.first++;
                now_step.second = 0;
                if (now_step.first == n) { now_selected_pos = Pos(-1,-1); return th; }
                now_selected_pos = procs.procs[now_step.first][now_step.second].selected_pos;
                continue;
            }

            // swap
            const auto& sp = procs.procs[now_step.first][now_step.second];
            const auto& now = sp.selected_pos;
            const Pos next = get_moved_toraly_pos(now, sp.dir, div_num);
            swap(now_orig_pos[now.y][now.x], now_orig_pos[next.y][next.x]);
            now_selected_pos = next;
            now_step.second++;
        }
    }
    int transition_to_begin(const Procedures& procs, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        if (now_step.first == -1) return 0;

        for(int th=-1;; --th) {
            now_step.second--;

            // select
            if (now_step.second == -1) {
                now_step.first--;
                if (now_step.first == -1) { now_selected_pos = Pos(-1,-1); return th; }
                now_step.second = procs.procs[now_step.first].size();
                now_selected_pos = procs.procs[now_step.first].back().selected_pos;
                move_toraly(now_selected_pos, procs.procs[now_step.first].back().dir, div_num);
                continue;
            }

            // swap
            const auto& sp = procs.procs[now_step.first][now_step.second];
            const auto& prev = sp.selected_pos;
            const Pos now = get_moved_toraly_pos(prev, sp.dir, div_num);
            swap(now_orig_pos[now.y][now.x], now_orig_pos[prev.y][prev.x]);
            now_selected_pos = prev;
        }
    }

    int transition(int step, const Procedures& procs, const ProblemSettings& settings) {
        const auto& div_num = settings.div_num;
        if (step < 0) {
            if (now_step.first == -1) return 0;

            for(int th=-1; th>=step; --th) {
                now_step.second--;

                // select
                if (now_step.second == -1) {
                    now_step.first--;
                    if (now_step.first == -1) { now_selected_pos = Pos(-1,-1); return th; }
                    now_step.second = procs.procs[now_step.first].size();
                    now_selected_pos = procs.procs[now_step.first].back().selected_pos;
                    move_toraly(now_selected_pos, procs.procs[now_step.first].back().dir, div_num);
                    continue;
                }

                // swap
                const auto& sp = procs.procs[now_step.first][now_step.second];
                const auto& prev = sp.selected_pos;
                const Pos now = get_moved_toraly_pos(prev, sp.dir, div_num);
                swap(now_orig_pos[now.y][now.x], now_orig_pos[prev.y][prev.x]);
                now_selected_pos = prev;
            }
        } else {
            int n = procs.procs.size();
            if (now_step.first == n) return 0;

            for (int th=1; th<=step; ++th) {
                // select
                if (now_step.first == -1 || now_step.second == (int)procs.procs[now_step.first].size()) {
                    now_step.first++;
                    now_step.second = 0;
                    if (now_step.first == n) { now_selected_pos = Pos(-1,-1); return th; }
                    now_selected_pos = procs.procs[now_step.first][now_step.second].selected_pos;
                    continue;
                }

                // swap
                const auto& sp = procs.procs[now_step.first][now_step.second];
                const auto& now = sp.selected_pos;
                const Pos next = get_moved_toraly_pos(now, sp.dir, div_num);
                swap(now_orig_pos[now.y][now.x], now_orig_pos[next.y][next.x]);
                now_selected_pos = next;
                now_step.second++;
            }
        }
        return step;
    }

    void draw_frags(const Size& TL, const Size& BR, const FragmentImages& frags, const ProblemSettings& settings, DrawType type) const {
        const auto& div_num = settings.div_num;
        const int length = (BR-TL).x / div_num.x;
        const Vec2 size(length, length);
        const Vec2 h_size(length/2., length/2.);

        // draw frag and info except selected one
        optional<Pos> idx(nullopt);
        rep(i,div_num.y) rep(j,div_num.x) {
            const Pos& p = now_orig_pos[i][j];

            if (now_selected_pos == Pos(j,i)) { idx = Pos(j,i); continue; }

            const Vec2 tmpTL = Vec2(TL.x, TL.y) + Vec2(j,i)*length;
            const Vec2 center = tmpTL + h_size;

            if (type.frag_type == DrawType::Image) frags.draw(p, size, tmpTL);
            else Rect(tmpTL.x, tmpTL.y, size.x-2, size.y-2).rounded(3).draw(get_color(Pos(j,i), div_num));

            if (type.info_type != DrawType::None) {
                stringstream ss;
                if (type.info_type == DrawType::Dist) {
                    auto dist = calc_mh_dist(Pos(j,i), div_num);
                    if (dist != 0) ss << dist;
                }
                else ss << uppercase << hex << p.x << p.y;

                FontAsset(U"TextOnFrag")(Unicode::Widen(ss.str())).drawAt(center-Vec2(1,1), Constant::TEXT_ON_FRAG_COLOR);
            }
        }

        // draw selected frag and info
        if (idx) {
            const int i = idx.value().y, j = idx.value().x;
            const Pos& p = now_orig_pos[i][j];
            const double offset = Periodic::Sine0_1(1.7s) * length/8. + length/10.;
            const Vec2 tmpTL = Vec2(TL.x, TL.y) + Vec2(j,i)*length - Vec2(offset, offset);
            const Vec2 center = tmpTL + h_size;
            Rect(tmpTL.x, tmpTL.y, length).drawShadow(Vec2(offset, offset), 24, 6);

            if (type.frag_type == DrawType::Image) frags.draw(p, size, tmpTL);
            else Rect(tmpTL.x, tmpTL.y, size.x, size.y).rounded(3).draw(get_color(Pos(j,i), div_num));

            if (type.info_type != DrawType::None) {
                stringstream ss;
                if (type.info_type == DrawType::Dist) {
                    auto dist = calc_mh_dist(Pos(j,i), div_num);
                    if (dist != 0) ss << dist;
                }
                else ss << uppercase << hex << p.x << p.y;

                FontAsset(U"TextOnFrag")(Unicode::Widen(ss.str())).drawAt(center-Vec2(1,1), Constant::TEXT_ON_FRAG_COLOR);
            }
        }
    }
    void draw_frags_at_correct_pos(const Size& TL, const Size& BR, const FragmentImages& frags, const ProblemSettings& settings, DrawType type) const {
        const auto& div_num = settings.div_num;
        const int length = (BR-TL).x / div_num.x;
        const Vec2 size(length, length);
        const Vec2 h_size(length/2., length/2.);

        // draw frag and info
        rep(i,div_num.y) rep(j,div_num.x) {
            const Pos& p = now_orig_pos[i][j];

            const Vec2 tmpTL = Vec2(TL.x, TL.y) + Vec2(p.x,p.y)*length;
            const Vec2 center = tmpTL + h_size;

            if (type.frag_type == DrawType::Image) frags.draw(p, size, tmpTL);
            else Rect(tmpTL.x, tmpTL.y, size.x-2, size.y-2).rounded(3).draw(get_color(Pos(j,i), div_num));

            if (type.info_type != DrawType::None) {
                stringstream ss;
                if (type.info_type == DrawType::Dist) {
                    auto dist = calc_mh_dist(Pos(j,i), div_num);
                    if (dist != 0) ss << dist;
                }
                else ss << uppercase << hex << p.x << p.y;

                FontAsset(U"TextOnFrag")(Unicode::Widen(ss.str())).drawAt(center-Vec2(1,1), Constant::TEXT_ON_FRAG_COLOR);
            }
        }
    }
    tuple<int, int, int, int, int, int> get_progress(const Procedures& procs) const {
        const int N = procs.get_max_istep();
        const int now = procs.get_istep(now_step);
        const int n = procs.procs.size();
        if (now_step.first == -1) return {-1, n, 0, 0, now, N};
        else if (now_step.first == n) return {n, n, 0, 0, now, N};
        else return {now_step.first, n, now_step.second, procs.procs[now_step.first].size(), now, N};
    }
};


struct Context {
    int32 score = 0;
    ProblemSettings problem_settings;
    FragmentImages frags;
    BoardState state;
    Procedures procs;
    string prob_hash;

    void load(const string& prob_dir_path) {
        filesystem::path path(prob_dir_path);
        prob_hash = path.filename();
        problem_settings.load(prob_dir_path);
        frags.load(prob_dir_path, problem_settings);
        state.load(prob_dir_path, problem_settings);
        procs.load(prob_dir_path, problem_settings);
    }
};

using App = SceneManager<String, Context>;

class RestorationVisualizer : public App::Scene {
    void draw_frags() const {

        const auto& settings = getData().problem_settings;
        const auto& frags = getData().frags;
        const auto& state = getData().state;

        constexpr Size AREA_TL = Constant::FRAGS_AREA_TL;
        constexpr Size AREA_BR = Constant::FRAGS_AREA_BR;
        constexpr int w = Constant::FRAGS_AREA_BR.x - Constant::FRAGS_AREA_TL.x;
        constexpr int h = Constant::FRAGS_AREA_BR.y - Constant::FRAGS_AREA_TL.y;
        const auto& div_num = settings.div_num;
        const int frag_length = min(w/div_num.x, h/div_num.y);
        const Size space(w-frag_length*div_num.x, h-frag_length*div_num.y);

        const Size TL = AREA_TL + space/2;
        const Size BR = AREA_BR - space/2;
        state.draw_frags_at_correct_pos(TL, BR, frags, settings, {BoardState::DrawType::Image, BoardState::DrawType::None});
    }
    void draw_info() const {
        const auto& settings = getData().problem_settings;
        const auto& procs = getData().procs;
        const auto& div_num = settings.div_num;

        const int select_times = procs.procs.size();
        const int select_cost = select_times * settings.select_cost;
        const int swap_times = procs.get_swap_times();
        const int swap_cost = swap_times * settings.swap_cost;
        const int total_cost = select_cost + swap_cost;

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

        const int w = 1100, h = 800;
        rep(i,info_arr.size()) {
            FontAsset(U"info")(info_arr[i]).draw(Arg::topLeft = Vec2(w,h+h_offset[i]),Palette::White);
        }
    };
public:

    RestorationVisualizer(const InitData& init) : IScene(init) {}

    void update() override {}
    void draw() const override {
        draw_info();
        draw_frags();
    }
};

class RestorationEditor : public App::Scene {

};

namespace PlaybackMenu {
    namespace Impl {
        int now_step = 0;
        bool holding = false;
        bool loop_playback = false;
        double seek_bar_rate = 0;
        double playback_speed = 1.;
        double playback_speed_magnification = 1.;
        double cumulative_diff_step = 0;
        enum { Play, Reverse, Pause } prev_type = Play, play_type = Pause;

        void draw_seek_bar(const Size& TL, const Size& BR) {
            const int h = (BR.y-TL.y);
            const int w = (BR.x-TL.x);
            const Vec2 s(TL.x + w/8.4, TL.y+h/2), t(BR.x-w/30, TL.y+h/2);
            string id = "001";

            Impl::holding = krGUI::seek_bar(Impl::seek_bar_rate, 0, 1., s, t, id);
        }
        void draw_playback_button(const Size& TL, const Size& BR) {
            const int h = (BR.y-TL.y);
            const int w = (BR.x-TL.x);

            if (holding) play_type = Pause;

            // change state by key
            if (KeySpace.down()) {
                if (play_type == Pause) play_type = prev_type;
                else prev_type = play_type, play_type = Pause;
            } else if (KeyR.down() && !KeyCommand.pressed() && !KeyControl.pressed()) {
                prev_type = play_type = (prev_type == Play ? Reverse : Play);
            }

            // change state by mouse
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

            // draw state
            if (play_type == Pause) {
                const Size size(30,30);
                const Size h_size = size/2;
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);
                const Point trp1 = pos - h_size;
                const Point trp2(pos.x-h_size.x, pos.y+h_size.y);
                const Point trp3(pos.x+h_size.x, pos.y);

                if (prev_type == Play) TextureAsset(U"play").resized(size).drawAt(pos, Constant::PLAY_BUTTON_COLOR);
                else TextureAsset(U"play").resized(size).rotated(180_deg).drawAt(pos, Constant::PLAY_BUTTON_COLOR);
            } else {
                constexpr Size size(30,30);
                constexpr Size h_size = size/2;
                const Point pos(TL.x+w/13, TL.y + h/2 + 1);

                TextureAsset(U"pause").resized(size).drawAt(pos, Constant::PAUSE_BUTTON_COLOR);
            }

            // repeat button
            {
                const Point pos(TL.x+w/56, TL.y + h/2 + 3);

                Rect rect(pos.x - 20, pos.y - 15, 40, 28);

                if (rect.leftClicked() || KeyC.down()) loop_playback = !loop_playback;

                if (loop_playback) {
                    FontAsset(U"repeat")(U"🔁").drawAt(pos, Constant::REPEAT_BUTTON_COLOR);
                    Circle(pos.x, pos.y+27, 4).draw(Constant::REPEAT_BUTTON_COLOR);
                } else FontAsset(U"repeat")(U"🔁").drawAt(pos, Constant::DO_NOT_REPEAT_BUTTON_COLOR);
            }
        }
        void draw_progress(const Size& TL, const Size& BR, const Context& ctx) {
            const auto& procs = ctx.procs;
            const auto& state = ctx.state;

            auto [a,n,b,m,now,N] = state.get_progress(procs); N = (N == 0 ? -1 : N);
            FontAsset(U"progress")(U"progress: {:>3}/{:<3}, {:>5}/{:<5}, {:>5}/{:<5}({:>3}.{}[%])"_fmt(a,n,b,m,now,N,now*100/N,(now*1000/N)%10))
                .drawAt(TL.x+475, BR.y-35, Palette::White);
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
            if (KeyL.down()) cumulative_diff_step = (int)cumulative_diff_step+1*max(1.,playback_speed_magnification);
            if (KeyH.down()) cumulative_diff_step = (int)cumulative_diff_step-1*max(1.,playback_speed_magnification);
            if (KeyK.pressed()) cumulative_diff_step += playback_speed * playback_speed_magnification;
            if (KeyJ.pressed()) cumulative_diff_step -= playback_speed * playback_speed_magnification;

            if (play_type != Pause && (KeyL.down() || KeyH.down() || KeyK.pressed() || KeyJ.pressed())) {
                prev_type = play_type;
                play_type = Pause;
            }
        }
        void cumulate_diff_step_by_playback_button() {
            int play_dir = (play_type == Pause ? 0 : (play_type == Play ? 1 : -1));
            cumulative_diff_step += play_dir * playback_speed * playback_speed_magnification;
        }
        void cumulate_diff_step_by_seek_bar(const int max_step) {
            cumulative_diff_step += seek_bar_rate * max_step - now_step;
        }
        void transition(Context& ctx) {
            const auto& settings = ctx.problem_settings;
            const auto& procs = ctx.procs;
            auto& state = ctx.state;

            int diff_step = cumulative_diff_step;
            int actual_diff_step = state.transition(diff_step, procs, settings);
            now_step += actual_diff_step;

            if (loop_playback || KeySpace.pressed()) {
                cumulative_diff_step -= actual_diff_step;
                while(abs(cumulative_diff_step) >= 1) {
                    if (cumulative_diff_step > 0) {
                        state.transition_to_begin(procs, settings);
                        now_step = 0;
                        cumulative_diff_step -= 1;
                    } else {
                        state.transition_to_end(procs, settings);
                        now_step = procs.get_max_istep();
                        cumulative_diff_step += 1;
                    }

                    diff_step = cumulative_diff_step;
                    actual_diff_step = state.transition(diff_step, procs, settings);
                    now_step += actual_diff_step;
                    cumulative_diff_step -= actual_diff_step;
                }
            } else cumulative_diff_step -= diff_step;


            if (((now_step == 0 && play_type == Reverse) || (now_step == procs.get_max_istep() && play_type == Play)) && !loop_playback) play_type = Pause;
            seek_bar_rate = (double)now_step / max((double)procs.get_max_istep(), 1e-8);
        }
    }

    void update(Context& ctx) {
        Impl::update_speed();

        Impl::cumulate_diff_step_by_key();
        Impl::cumulate_diff_step_by_playback_button();
        Impl::cumulate_diff_step_by_seek_bar(ctx.procs.get_max_istep());

        Impl::transition(ctx);
    }

    void draw(const Context& ctx) {
        constexpr Size TL = Constant::RIGHT_BAR_AREA_TL + Size(0, Constant::RIGHT_BAR_AREA_SIZE.y * Constant::RIGHT_BAR_RATIO.x / (Constant::RIGHT_BAR_RATIO.x + Constant::RIGHT_BAR_RATIO.y));
        constexpr Size BR = Constant::RIGHT_BAR_AREA_BR;

        Impl::draw_seek_bar(TL, BR);
        Impl::draw_playback_button(TL, BR);
        Impl::draw_progress(TL,BR,ctx);
    }
}

class ProceduresVisualizer : public App::Scene {
    inline static BoardState::DrawType draw_frag_type;
    void update_draw_frag_type() {
        if (KeyZ.down()) {
            if (draw_frag_type.frag_type == BoardState::DrawType::Image)
                draw_frag_type.frag_type = BoardState::DrawType::Color;
            else draw_frag_type.frag_type = BoardState::DrawType::Image;
        }
        if (KeyX.down()) {
            if (draw_frag_type.info_type == BoardState::DrawType::None)
                draw_frag_type.info_type = BoardState::DrawType::Dist;
            else if (draw_frag_type.info_type == BoardState::DrawType::Dist)
                draw_frag_type.info_type = BoardState::DrawType::Pos;
            else draw_frag_type.info_type = BoardState::DrawType::None;
        }
    }
    void draw_frags() const {

        const auto& settings = getData().problem_settings;
        const auto& frags = getData().frags;
        const auto& state = getData().state;

        constexpr Size AREA_TL = Constant::FRAGS_AREA_TL;
        constexpr Size AREA_BR = Constant::FRAGS_AREA_BR;
        constexpr int w = Constant::FRAGS_AREA_BR.x - Constant::FRAGS_AREA_TL.x;
        constexpr int h = Constant::FRAGS_AREA_BR.y - Constant::FRAGS_AREA_TL.y;
        const auto& div_num = settings.div_num;
        const int frag_length = min(w/div_num.x, h/div_num.y);
        const Size space(w-frag_length*div_num.x, h-frag_length*div_num.y);

        const Size TL = AREA_TL + space/2;
        const Size BR = AREA_BR - space/2;
        state.draw_frags(TL, BR, frags, settings, draw_frag_type);
    }

    void draw_info() const {
        const auto& settings = getData().problem_settings;
        const auto& procs = getData().procs;
        const auto& div_num = settings.div_num;

        const int select_times = procs.procs.size();
        const int select_cost = select_times * settings.select_cost;
        const int swap_times = procs.get_swap_times();
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
    };

public:

    ProceduresVisualizer(const InitData& init) : IScene(init) {

    }

    void update() override {
/*
        if (MouseR.down()) {
            changeScene(U"Title");
        }
*/

        update_draw_frag_type();

        PlaybackMenu::update(getData());
    }

    void draw() const override {
        draw_info();
        draw_frags();
        PlaybackMenu::draw(getData());
    }
};


// タイトルシーン
class Title : public App::Scene {
public:

    // コンストラクタ（必ず実装）
    Title(const InitData& init)
        : IScene(init)
    {

    }

    // 更新関数
    void update() override
    {
        // 左クリックで
        if (MouseL.down())
        {
            // ゲームシーンに遷移
            changeScene(U"Game");
        }
        if (MouseR.down()) {
            changeScene(U"ProceduresVisualizer");
        }
    }

    // 描画関数 (const 修飾)
    void draw() const override
    {
        Scene::SetBackground(ColorF(0.3, 0.4, 0.5));

        FontAsset(U"TitleFont")(U"My Game").drawAt(400, 100);

        // 現在のスコアを表示
        FontAsset(U"ScoreFont")(U"Score: {}"_fmt(getData().score)).draw(520, 540);

        Circle(Cursor::Pos(), 50).draw(Palette::Orange);
    }
};

// ゲームシーン
class Game : public App::Scene {
private:

    Texture m_texture;

public:

    Game(const InitData& init)
        : IScene(init)
        , m_texture(Emoji(U"🐈"))
    {

    }

    void update() override {
        // 左クリックで
        if (MouseL.down())
        {
            // タイトルシーンに遷移
            changeScene(U"Title");
        }

        // マウスカーソルの移動でスコアが増加
        getData().score += static_cast<int32>(Cursor::Delta().length() * 10);
    }

    void draw() const override {
        Scene::SetBackground(ColorF(0.2, 0.8, 0.6));

        m_texture.drawAt(Cursor::Pos());

        // 現在のスコアを表示
        FontAsset(U"ScoreFont")(U"Score: {}"_fmt(getData().score)).draw(40, 40);
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

//    manager.add<RestorationVisualizer>(U"RestorationVisualizer");

    manager.add<ProceduresVisualizer>(U"ProceduresVisualizer");

    // タイトルシーン（名前は U"Title"）を登録
    manager.add<Title>(U"Title");

    // ゲームシーン（名前は U"Game"）を登録
    manager.add<Game>(U"Game");


    manager.get()->score = 100000;
    manager.get()->load("/Users/rk/Projects/procon32/.tmp_data/tmp");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/40211b699f29d8b5077e1cc86a7f4f9d");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/4b119be5dbf5a0ececf00f506f5459b7");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/4eb0b7e9750cf7aad51a8cf11cc45131");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/5f58a788475f3c05f7295ebe82a3dc1e");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/66b9d56c5ccb3e886de0fbc1ae6ba2b3");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/9359558aafa91b63d7575b0aad245780");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/a89ea53bb8969322a6653a2a79b83a3b");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/a9df29717a478f25cec69f626bdd9bff");
//    manager.get()->load("/Users/rk/Projects/procon32/.data/ba8d67aa3b80cfc934a9ee9b114cbd61");


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
        {
            const int h = Window::ClientHeight(), w = Window::ClientWidth();
            if (abs(h*Constant::WINDOW_RATIO.x - w*Constant::WINDOW_RATIO.y) > 10) {
                Size size((int)(h*Constant::WINDOW_RATIO.x / Constant::WINDOW_RATIO.y), h);
                Window::Resize(size, WindowResizeOption::KeepSceneSize, false);
            }
        }

        // 現在のシーンを実行
        if (!manager.update()) break;

        // 右上にfpsを描画
        drawFPS();
    }
}
