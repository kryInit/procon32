#include "side_beam_search.hpp"
#include <bits/stdc++.h>
#include <direction.hpp>
using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

template<class T>
using rev_priority_queue = priority_queue<T, vector<T>, greater<T>>;

unsigned int get_dir_offset(Direction reference, Direction another) {
    return (static_cast<int>(another) - static_cast<int>(reference) + 4) % 4;
}

constexpr unsigned int MAX_DIV_NUM = 16;

struct ImageFragmentState {
    unsigned char orig_idx, rotation_times;

    ImageFragmentState() : orig_idx(), rotation_times() {}
    ImageFragmentState(unsigned char _orig_idx, unsigned char _rotation_times) : orig_idx(_orig_idx), rotation_times(_rotation_times) {}
    void rotate_90deg(int times) {
        times = 4 + (times%4);
        rotation_times = static_cast<unsigned char>((static_cast<int>(rotation_times)+times)%4);
    }
    unsigned int get_idx() const { return static_cast<unsigned int>(orig_idx)*4 + static_cast<unsigned int>(rotation_times); }
};

struct ImageState {
    Vec2<unsigned short> now_size;
    float adjacency_sum;
    array<array<ImageFragmentState, MAX_DIV_NUM>, MAX_DIV_NUM> states;
    array<bool, MAX_DIV_NUM*MAX_DIV_NUM> placed;

    float adjacency_ave() const {
        return adjacency_sum / (float)(now_size.y*(now_size.x-1) + (now_size.y-1)*now_size.x);
    }
    void dump_for_human(const Vec2<unsigned int> DIV_NUM) const {
        cout << "adjacency (sum, ave): (" << adjacency_sum << ", " << adjacency_ave() << ")" << endl;
        cout << "size: " << now_size << endl;
        rep(i,DIV_NUM.y) {
            rep(j,DIV_NUM.x) {
                cout << hex << uppercase << states[i][j].orig_idx%DIV_NUM.x << states[i][j].orig_idx/DIV_NUM.x << "(" << (unsigned int)states[i][j].rotation_times << ") ";
            }
            cout << endl;
        }
    }
    void dump(ostream& os, Vec2<unsigned int> DIV_NUM) const {
        vector<int> v(DIV_NUM.y*DIV_NUM.x);
        rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) v[states[i][j].orig_idx] = states[i][j].rotation_times;
        for(const auto& i : v) os << i;
        os << endl;
        rep(i,DIV_NUM.y) {
            rep(j,DIV_NUM.x) {
                os << hex << uppercase << states[i][j].orig_idx%DIV_NUM.x << states[i][j].orig_idx/DIV_NUM.x;
                if (j != static_cast<int>(DIV_NUM.x)-1) os << " ";
            }
            os << endl;
        }
    }
    void dump(ostream& os) const {
        dump(os, Vec2<unsigned int>(now_size.x, now_size.y));
    }
    void dump(const Vec2<unsigned int> DIV_NUM) const {
        dump(cout, DIV_NUM);
    }
    void dump() const {
        dump(cout, Vec2<unsigned int>(now_size.x, now_size.y));
    }

    unsigned char get_00_rotation_times() const {
        rep(i,now_size.y) rep(j,now_size.x) if (states[i][j].orig_idx == 0) return states[i][j].rotation_times;
        return 255;
    }

    void rotate_90deg(int times) {
        times = (times%4+4)%4;
        if (times == 1) rotate_90deg();
        else if (times == 2) rotate_180deg();
        else if (times == 3) rotate_270deg();
    }
    void rotate_90deg() {
        array<array<ImageFragmentState, MAX_DIV_NUM>, MAX_DIV_NUM> tmp_states;
        rep(i,now_size.y) rep(j,now_size.x) tmp_states[j][now_size.y-i-1] = states[i][j];
        swap(now_size.x, now_size.y);
        rep(i,now_size.y) rep(j,now_size.x) tmp_states[i][j].rotate_90deg(1);
        states = tmp_states;
    }
    void rotate_180deg() {
        rep(i,now_size.y) rep(j,now_size.x) {
                int I = now_size.y-i-1;
                int J = now_size.x-j-1;
                if (i*now_size.x+j >= I*now_size.x+J) break;
                swap(states[i][j], states[I][J]);
            }
        rep(i,now_size.y) rep(j,now_size.x) states[i][j].rotate_90deg(2);
    }
    void rotate_270deg() {
        array<array<ImageFragmentState, MAX_DIV_NUM>, MAX_DIV_NUM> tmp_states;
        rep(i,now_size.y) rep(j,now_size.x) tmp_states[now_size.x-j-1][i] = states[i][j];
        swap(now_size.x, now_size.y);
        rep(i,now_size.y) rep(j,now_size.x) tmp_states[i][j].rotate_90deg(3);
        states = tmp_states;
    }
    void place(Vec2<unsigned short> pos, ImageFragmentState frag_state, float d_adjacency) {
        states[pos.y][pos.x] = frag_state;
        placed[frag_state.orig_idx] = true;
        now_size.x = max(now_size.x+0, pos.x+1);
        now_size.y = max(now_size.y+0, pos.y+1);
        adjacency_sum += d_adjacency;
    }

    [[nodiscard]] vector<ImageFragmentState> get_side(Direction dir) const {

        return vector<ImageFragmentState>();
    }

    Answer convert_answer() {
        Answer ans;
        ans.rotations.resize(now_size.y*now_size.x);
        rep(i,now_size.y) rep(j,now_size.x) ans.rotations[states[i][j].orig_idx] = states[i][j].rotation_times;
        rep(i,now_size.y) {
            vector<Vec2<unsigned int>> tmp;
            rep(j,now_size.x) tmp.emplace_back(states[i][j].orig_idx%now_size.x, states[i][j].orig_idx/now_size.x);
            ans.orig_idx.push_back(tmp);
        }
        return ans;
    }

    ImageState() : now_size{}, adjacency_sum{}, states{}, placed{} {}
    bool operator<(const ImageState& another) const {
        return adjacency_sum < another.adjacency_sum;
    }
    bool operator>(const ImageState& another) const {
        return another.adjacency_sum < adjacency_sum;
    }
};

struct FragmentPair {
    float adjacency;
    ImageFragmentState l,r;

    FragmentPair() : adjacency(), l(), r() {}
    FragmentPair(float _adjacency, ImageFragmentState _l, ImageFragmentState _r) : adjacency(_adjacency), l(_l), r(_r) {}
    bool operator<(const FragmentPair& another) const {
                                                  return adjacency < another.adjacency;
                                                  }
    bool operator>(const FragmentPair& another) const {
                                                  return another.adjacency < adjacency;
                                                  }
};
using v2fp = vector<vector<FragmentPair>>;

unsigned int get_side_idx(const ImageFragmentState frag_state, const Direction dir) {
    if (dir == Direction::U) return (4-frag_state.rotation_times)%4;
    else if (dir == Direction::R) return (5-frag_state.rotation_times)%4;
    else if (dir == Direction::D) return (6-frag_state.rotation_times)%4;
    else /* if (dir == Direction::L) */ return 3 - frag_state.rotation_times;
}

void dump_adjacency_info(const double *adjacency, const Settings& settings) {
    auto div_num = settings.DIV_NUM();
    auto frag_num = div_num.y*div_num.x;
    rep(i,div_num.y) {
        rep(j,div_num.x) {
            rep(k,4) {
                const unsigned int offset = (i*div_num.x*4+j*4+k)*frag_num*4;
                double best_adjacency = 1e10;
                int best_i=-1, best_j=-1, best_k=-1;
                rep(I,div_num.y) {
                    rep(J,div_num.x) {
                        if (I == i && J == j) continue;
                        rep(K,4) {
                            double tmp = *(adjacency+offset + I*div_num.x*4 + J*4 + K);
                            if (best_adjacency > tmp) {
                                best_adjacency = tmp;
                                best_i = I;
                                best_j = J;
                                best_k = K;
                            }
                        }
                    }
                }
                cout << "(" << j << ", " << i << ", " << k <<  ") <=> (" << best_j << ", " << best_i << ", " << best_k << ") : " << best_adjacency << endl;
            }
        }
    }
    const unsigned int offset = (2*div_num.x*4 + 1*4 + 3)*frag_num*4;
    //        const unsigned int offset = (13*div_num.x*4 + 3*4 + 1)*frag_num*4;
    //        const unsigned int offset = 0;
    rep(i,div_num.y) {
        rep(j,div_num.x) {
            cout << "(";
            rep(k,4) cout << setw(2) << *(adjacency + offset + i*div_num.x*4 + j*4+k) << ", ";
            cout << "), ";
        }
        cout << endl;
    }
    cout << endl;
}
void dump_ordered_adjacency_info(const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM) {
    rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) rep(k,4) {
        unsigned int idx = i*DIV_NUM.x*4 + j*4 + k;
        auto tmp = ordered_adjacency[idx].front().r;
        cout << "(" << j << ", " << i << ", " << k << ") <=> (" << tmp.orig_idx % DIV_NUM.x << ", " << tmp.orig_idx / DIV_NUM.x << ", " << 3 - tmp.rotation_times << ") : " << ordered_adjacency[idx].front().adjacency << endl;
    }
}
void dump_states(const vector<ImageState>& states, const Vec2<unsigned int>& DIV_NUM, const unsigned int maximum = 1000) {
    unsigned int th = 0;
    for(const auto& img_state : states) {
        if (maximum <= th) break;
        img_state.dump_for_human(DIV_NUM);
        th++;
    }
}

v2fp get_ordered_adjacency(const double *adjacency, const Vec2<unsigned int>& DIV_NUM) {
    const unsigned int N = DIV_NUM.y*DIV_NUM.x*4;
    v2fp ordered_adjacency(N, vector<FragmentPair>(N));
    rep(idx0, N) {
        repo(idx1, idx0, N) {
            const auto adj = static_cast<float>(*(adjacency + idx0*N + idx1));
            ImageFragmentState frag_state0(idx0 / 4, (5-idx0%4)%4);
            ImageFragmentState frag_state1(idx1 / 4, (3-idx1%4));
            ordered_adjacency[idx0][idx1] = FragmentPair(adj, frag_state0, frag_state1);
            frag_state0.rotation_times = (frag_state0.rotation_times+2)%4;
            frag_state1.rotation_times = (frag_state1.rotation_times+2)%4;
            ordered_adjacency[idx1][idx0] = FragmentPair(adj, frag_state1, frag_state0);
        }
        sort(ordered_adjacency[idx0].begin(), ordered_adjacency[idx0].end());
    }
    return ordered_adjacency;
}
pair<float, ImageFragmentState> get_best_adjacent_fragment(ImageFragmentState frag_state, Direction dir, const array<bool, MAX_DIV_NUM*MAX_DIV_NUM>& placed, const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM) {
    const unsigned int idx = frag_state.orig_idx*4 + get_side_idx(frag_state, dir);
    for (const auto& fp : ordered_adjacency[idx]) if (!placed[fp.r.orig_idx]) {
        ImageFragmentState tmp = fp.r;
        tmp.rotate_90deg(static_cast<int>(get_dir_offset(Direction::R, dir)));
        return make_pair(fp.adjacency, tmp);
    }
    cerr << "get best adjacent fragment" << endl;
    exit(-1);
}
pair<float, ImageFragmentState> get_best_adjacent_fragment(ImageFragmentState frag_state0, Direction dir0, ImageFragmentState frag_state1, Direction dir1, const array<bool, MAX_DIV_NUM*MAX_DIV_NUM>& placed, const double* adjacency, const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM) {
    if (dir0 == dir1) {
        cerr << "get_best_adjacent_fragment" << endl;
        exit(-1);
    }
    const unsigned int N = DIV_NUM.y * DIV_NUM.x * 4;
    float min_adjacency = 1e20;
    ImageFragmentState best_frag_state;
    const unsigned int idx0 = frag_state0.orig_idx*4 + get_side_idx(frag_state0, dir0);
    const unsigned int idx1 = frag_state1.orig_idx*4 + get_side_idx(frag_state1, dir1);
    {
        const unsigned int dir_offset = get_dir_offset(dir0, dir1);
        for (const auto& fp : ordered_adjacency[idx0]) {
            if (fp.adjacency >= min_adjacency) break;
            const unsigned int idx2_adjacent_idx1 = fp.r.orig_idx*4 + (get_side_idx(fp.r, Direction::L) + dir_offset)%4;
            float tmp_adjacency = fp.adjacency + static_cast<float>(*(adjacency + idx1*N + idx2_adjacent_idx1));
            if (!placed[fp.r.orig_idx] && min_adjacency > tmp_adjacency) {
                min_adjacency = tmp_adjacency;
                ImageFragmentState tmp = fp.r;
                tmp.rotate_90deg(static_cast<int>(get_dir_offset(Direction::R, dir0)));
                best_frag_state = tmp;
            }
        }
    }
    /*
    {
        const unsigned int dir_offset = get_dir_offset(dir1, dir0);
        for (const auto& fp : ordered_adjacency[idx1]) {
            if (fp.adjacency >= min_adjacency) break;
            const unsigned int idx2_adjacent_idx0 = fp.r.orig_idx*4 + (get_side_idx(fp.r, Direction::L) + dir_offset)%4;
            float tmp_adjacency = fp.adjacency + static_cast<float>(*(adjacency + idx0*N + idx2_adjacent_idx0));
            if (!placed[fp.r.orig_idx] && min_adjacency > tmp_adjacency) {
                min_adjacency = tmp_adjacency;
                ImageFragmentState tmp = fp.r;
                tmp.rotate_90deg(static_cast<int>(get_dir_offset(Direction::R, dir1)));
                best_frag_state = tmp;
            }
        }
    }
    */
    if (abs(min_adjacency - 1e20) < 1) {
        cerr << "get best adjacent fragment" << endl;
        exit(-1);
    }
    return make_pair(min_adjacency, best_frag_state);
}
vector<ImageState> get_first_sorted_states(const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM, const unsigned int MAX_STATE_NUM) {
    const unsigned int N = DIV_NUM.y*DIV_NUM.x*4;
    vector<ImageState> now_states;
    rev_priority_queue<FragmentPair> pq;
    vector<vector<bool>> pushed(N, vector<bool>(N, false));
    rep(i,N) rep(j,N) if (ordered_adjacency[i][j].adjacency != INFINITY) {
        unsigned int l_idx = ordered_adjacency[i][j].l.get_idx();
        unsigned int r_idx = ordered_adjacency[i][j].r.get_idx();
        if (!pushed[l_idx][r_idx]) {
            pq.push(ordered_adjacency[i][j]);
            pushed[l_idx][r_idx] = pushed[r_idx][l_idx] = true;
        }
    }
    cout << "min_adjacency: " << pq.top().adjacency << endl;
    rep(i, MAX_STATE_NUM) {
        FragmentPair tmp = pq.top(); pq.pop();
        ImageState state;
        state.place(Vec2<unsigned short>(0,0), tmp.l, 0);
        state.place(Vec2<unsigned short>(1,0), tmp.r, tmp.adjacency);
        now_states.push_back(state);
        if (pq.empty()) break;
    }
    return now_states;
}

ImageState expand_image(ImageState img_state, Direction dir, const double *adjacency, const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM) {
    if (dir != Direction::D && dir != Direction::R) {
        cerr << "expand image" << endl;
        exit(-1);
    }
    const Vec2<unsigned short> now_size = img_state.now_size;
    if (dir == Direction::D) {
        rep(i, now_size.x) {
            pair<float, ImageFragmentState> tmp;
            if (i == 0) tmp = get_best_adjacent_fragment(img_state.states[now_size.y-1][i], Direction::D, img_state.placed, ordered_adjacency, DIV_NUM);
            else tmp = get_best_adjacent_fragment(img_state.states[now_size.y-1][i], Direction::D, img_state.states[now_size.y][i-1], Direction::R, img_state.placed, adjacency, ordered_adjacency, DIV_NUM);
            img_state.place(Vec2<unsigned short>(i, now_size.y), tmp.second, tmp.first);
        }
    } else /* if (dir == Direction::R) */ {
        rep(i,now_size.y) {
            pair<float, ImageFragmentState> tmp;
            if (i == 0) tmp = get_best_adjacent_fragment(img_state.states[i][now_size.x-1], Direction::R, img_state.placed, ordered_adjacency, DIV_NUM);
            else tmp = get_best_adjacent_fragment(img_state.states[i][now_size.x-1], Direction::R, img_state.states[i-1][now_size.x], Direction::D, img_state.placed, adjacency, ordered_adjacency, DIV_NUM);
            img_state.place(Vec2<unsigned short>(now_size.x, i), tmp.second, tmp.first);
        }
    }
    return img_state;
}
vector<ImageState> generate_next_states(const vector<ImageState>& now_states, const double *adjacency, const v2fp& ordered_adjacency, const Vec2<unsigned int>& DIV_NUM) {
    vector<ImageState> next_states;
    next_states.reserve(now_states.size()*4);
    for(const auto& img_state : now_states) {
        const Vec2<unsigned short>& now_size = img_state.now_size;
        ImageState tmp_img_state = img_state;
        tmp_img_state.rotate_180deg();
        const bool x_expandable = !(now_size.x < now_size.y && now_size.x == min(DIV_NUM.x, DIV_NUM.y)) && !(now_size.x > now_size.y && now_size.x == max(DIV_NUM.x, DIV_NUM.y));
        const bool y_expandable = !(now_size.y < now_size.x && now_size.y == min(DIV_NUM.x, DIV_NUM.y)) && !(now_size.y > now_size.x && now_size.y == max(DIV_NUM.x, DIV_NUM.y));
        if (x_expandable) {
            next_states.push_back(expand_image(img_state, Direction::R, adjacency, ordered_adjacency, DIV_NUM));
            next_states.push_back(expand_image(tmp_img_state, Direction::R, adjacency, ordered_adjacency, DIV_NUM));
        }
        if (y_expandable) {
            next_states.push_back(expand_image(img_state, Direction::D, adjacency, ordered_adjacency, DIV_NUM));
            next_states.push_back(expand_image(tmp_img_state, Direction::D, adjacency, ordered_adjacency, DIV_NUM));
        }
    }
    return next_states;
}
void remove_duplicate_state(vector<ImageState>& states) {
    set<size_t> hash_map;
    vector<ImageState> tmp;
    tmp.reserve(states.size());
    for(const auto& img_state : states) {
        string s;
        rep(i,MAX_DIV_NUM*MAX_DIV_NUM) s.push_back(img_state.placed[i]);
        size_t a = hash<string>()(s);
        size_t b = hash<string>()(to_string(img_state.adjacency_sum).substr(0, 6));
        size_t c = a^b;
        if (hash_map.count(c) == 0) {
            tmp.push_back(img_state);
            hash_map.insert(c);
        }
    }
    states = move(tmp);
}
void sort_and_resize_states(vector<ImageState>& states, const unsigned int MAX_STATE_NUM) {
    vector<pair<float, int>> v;
    v.reserve(states.size());
    rep(i,states.size()) v.emplace_back(states[i].adjacency_ave(), i);
    sort(v.begin(), v.end());
    const unsigned int n = min((unsigned int)v.size(), MAX_STATE_NUM);
    vector<ImageState> tmp(n);
    rep(i,n) tmp[i] = states[v[i].second];
    states = move(tmp);
}
Answer get_answer(vector<ImageState>& sorted_states) {
    for(const auto& img_state : sorted_states) {
        unsigned int rotation_times_00 = img_state.get_00_rotation_times();
        if (rotation_times_00 % 2 == 0 || img_state.now_size.x == img_state.now_size.y) {
            ImageState tmp = img_state;
            tmp.rotate_90deg(4 - rotation_times_00);
            return tmp.convert_answer();
        }
    }
    cerr << "get_answer" << endl;
    exit(-1);
}

Answer SideBeamSearchSolver::solve(double *adjacency, const Settings& settings) {
//    dump_adjacency_info(adjacency, settings);
    cout << "solve" << endl;
    cout << "fragment pair size: " << sizeof(FragmentPair) << endl;
    cout << "image fragment state size: " << sizeof(ImageFragmentState) << endl;
    cout << "image state size: " << sizeof(ImageState) << endl;

    constexpr unsigned int MAX_STATE_NUM = 128;
//    constexpr unsigned int MAX_STATE_NUM = 1024;
//    constexpr unsigned int MAX_STATE_NUM = 8192;
//    constexpr unsigned int MAX_STATE_NUM = 65536;
    const auto& DIV_NUM = settings.DIV_NUM();
    const auto& ordered_adjacency = get_ordered_adjacency(adjacency, DIV_NUM);

//    dump_ordered_adjacency_info(ordered_adjacency, DIV_NUM);

    auto now_states = get_first_sorted_states(ordered_adjacency, DIV_NUM, MAX_STATE_NUM);
//    dump_states(now_states, DIV_NUM);

    const unsigned int STEP_NUM = DIV_NUM.y + DIV_NUM.x - 3;
    rep(step, STEP_NUM) {
        cout << step << " th" << endl;
/*
        cout << "\n\n" << "=============================" << endl;
        dump_states(now_states, DIV_NUM, 10);
        map<Vec2<unsigned short>, int> m;
        for(const auto& i : now_states) m[i.now_size]++;
        for(const auto& i : m) cout << dec << i.first << " " << i.second << endl;
        cout << "=============================" << "\n\n" << endl;
*/
        auto next_states = generate_next_states(now_states, adjacency, ordered_adjacency, DIV_NUM);
        cout << "size before remove: " << next_states.size() << endl;
        remove_duplicate_state(next_states);
        cout << "size after remove: " << next_states.size() << endl;
        sort_and_resize_states(next_states, MAX_STATE_NUM);
//        dump_states(next_states, DIV_NUM, 100);
        now_states = move(next_states);

    }
    if (now_states.empty()) {
        cerr << "solve" << endl;
        exit(-1);
    }
    return get_answer(now_states);
}
