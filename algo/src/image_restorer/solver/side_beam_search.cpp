
#include "side_beam_search.hpp"
#include <bits/stdc++.h>
#include <direction.hpp>
#include <utility.hpp>
#include <macro.hpp>
using namespace std;


int get_dir_offset(Direction reference, Direction another) {
    int count = 0;
    while(reference != another) { reference.rotate_cw(); count++; }
    return count;
}

constexpr unsigned int MAX_DIV_NUM = 16;

struct ImageFragmentState {
    unsigned char orig_idx, rotation_times;

    ImageFragmentState() : orig_idx(), rotation_times() {}
    ImageFragmentState(unsigned char orig_idx, unsigned char rotation_times) : orig_idx(orig_idx), rotation_times(rotation_times) {}
    void rotate_90deg(int times) {
        times = 4 + (times%4);
        rotation_times = static_cast<unsigned char>((static_cast<int>(rotation_times)+times)%4);
    }
    [[nodiscard]] unsigned int get_idx() const { return static_cast<unsigned int>(orig_idx)*4 + static_cast<unsigned int>(rotation_times); }
};

struct ImageState {
    Vec2<unsigned short> now_size;
    float adjacency_sum;
    array<array<ImageFragmentState, MAX_DIV_NUM>, MAX_DIV_NUM> states;
    array<bool, MAX_DIV_NUM*MAX_DIV_NUM> placed;

    [[nodiscard]] float adjacency_ave() const {
        return adjacency_sum / (float)(now_size.y*(now_size.x-1) + (now_size.y-1)*now_size.x);
    }
    void dump_for_human(const Vec2<int> div_num) const {
        PRINT(adjacency_sum, adjacency_ave());
        PRINT(now_size);
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                cout << hex << uppercase << states[i][j].orig_idx%div_num.x << states[i][j].orig_idx/div_num.x << "(" << (unsigned int)states[i][j].rotation_times << ") ";
            }
            cout << endl;
        }
    }
    void dump(ostream& os, Vec2<int> div_num) const {
        vector<int> v(div_num.y*div_num.x);
        rep(i,now_size.y) rep(j,now_size.x) v[i*now_size.x+j] = states[i][j].rotation_times;
        for(const auto& i : v) os << i;
        os << endl;
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                os << hex << uppercase << states[i][j].orig_idx%div_num.x << states[i][j].orig_idx/div_num.x;
                if (j != static_cast<int>(div_num.x)-1) os << " ";
            }
            os << endl;
        }
    }
    void dump(ostream& os) const {
        dump(os, Vec2<int>(now_size.x, now_size.y));
    }
    void dump(const Vec2<int> div_num) const {
        dump(cout, div_num);
    }
    void dump() const {
        dump(cout, Vec2<int>(now_size.x, now_size.y));
    }

    [[nodiscard]] unsigned char get_00_rotation_times() const {
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
    void place(Pos pos, ImageFragmentState frag_state, float d_adjacency) {
        states[pos.y][pos.x] = frag_state;
        placed[frag_state.orig_idx] = true;
        now_size.x = max(now_size.x+0, pos.x+1);
        now_size.y = max(now_size.y+0, pos.y+1);
        adjacency_sum += d_adjacency;
    }

    Answer convert_answer() {
        Answer ans;
        ans.rotations.resize(now_size.y*now_size.x);
        rep(i,now_size.y) rep(j,now_size.x) ans.rotations[i*now_size.x+j] = states[i][j].rotation_times;
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
    FragmentPair(float adjacency, ImageFragmentState l, ImageFragmentState r) : adjacency(adjacency), l(l), r(r) {}
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
    auto div_num = settings.div_num;
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
void dump_sorted_fp_info(const v2fp& sorted_fp, const Vec2<int>& div_num) {
    rep(i,div_num.y) rep(j,div_num.x) rep(k,4) {
        unsigned int idx = i*div_num.x*4 + j*4 + k;
        auto tmp = sorted_fp[idx].front().r;
        cout << "(" << j << ", " << i << ", " << k << ") <=> (" << tmp.orig_idx % div_num.x << ", " << tmp.orig_idx / div_num.x << ", " << 3 - tmp.rotation_times << ") : " << sorted_fp[idx].front().adjacency << endl;
    }
}
void dump_states(const vector<ImageState>& states, const Vec2<int>& div_num, const unsigned int maximum = 1000) {
    unsigned int th = 0;
    for(const auto& img_state : states) {
        if (maximum <= th) break;
        img_state.dump_for_human(div_num);
        th++;
    }
}

v2fp get_sorted_fragment_pair(const double *adjacency, const Vec2<int>& div_num) {
    const unsigned int N = div_num.y*div_num.x*4;
    v2fp sorted_fp(N, vector<FragmentPair>(N));
    rep(i, N) {
        repr(j, i, N) {
            const auto adj = static_cast<float>(*(adjacency + i*N + j));
            ImageFragmentState frag_state0(i / 4, (5-i%4)%4);
            ImageFragmentState frag_state1(j / 4, (3-j%4));
            sorted_fp[i][j] = FragmentPair(adj, frag_state0, frag_state1);
            frag_state0.rotation_times = (frag_state0.rotation_times+2)%4;
            frag_state1.rotation_times = (frag_state1.rotation_times+2)%4;
            sorted_fp[j][i] = FragmentPair(adj, frag_state1, frag_state0);
        }
        sort(sorted_fp[i].begin(), sorted_fp[i].end());
    }
    return sorted_fp;
}
vector<ImageState> get_initial_states(const v2fp& sorted_fp, const Vec2<int>& div_num, const unsigned int MAX_STATE_NUM) {
    const unsigned int N = div_num.y*div_num.x*4;
    vector<ImageState> current_states;
    Utility::reverse_priority_queue<FragmentPair> pq;
    vector<vector<bool>> pushed(N, vector<bool>(N, false));
    rep(i,N) rep(j,N) if (sorted_fp[i][j].adjacency != INFINITY) {
        unsigned int l_idx = sorted_fp[i][j].l.get_idx();
        unsigned int r_idx = sorted_fp[i][j].r.get_idx();
        if (!pushed[l_idx][r_idx]) {
            pq.push(sorted_fp[i][j]);
            pushed[l_idx][r_idx] = pushed[r_idx][l_idx] = true;
        }
    }
    rep(i, MAX_STATE_NUM) {
        FragmentPair tmp = pq.top(); pq.pop();
        ImageState state;
        state.place(Pos(0,0), tmp.l, 0);
        state.place(Pos(1,0), tmp.r, tmp.adjacency);
        current_states.push_back(state);
        if (pq.empty()) break;
    }
    return current_states;
}
pair<float, ImageFragmentState> get_best_adjacent_fragment(ImageFragmentState frag_state, Direction dir, const array<bool, MAX_DIV_NUM*MAX_DIV_NUM>& placed, const v2fp& sorted_fp, const Vec2<int>& div_num) {
    const unsigned int idx = frag_state.orig_idx*4 + get_side_idx(frag_state, dir);
    for (const auto& fp : sorted_fp[idx]) if (!placed[fp.r.orig_idx]) {
        ImageFragmentState tmp = fp.r;
        tmp.rotate_90deg(get_dir_offset(Direction::R, dir));
        return make_pair(fp.adjacency, tmp);
    }
    cerr << "get best adjacent fragment1" << endl;
    exit(-1);
}
pair<float, ImageFragmentState> get_best_adjacent_fragment(ImageFragmentState frag_state0, Direction dir0, ImageFragmentState frag_state1, Direction dir1, const array<bool, MAX_DIV_NUM*MAX_DIV_NUM>& placed, const double* adjacency, const v2fp& sorted_fp, const Vec2<int>& div_num) {
    if (dir0 == dir1) {
        cerr << "get_best_adjacent_fragment2" << endl;
        exit(-1);
    }
    const unsigned int N = div_num.y * div_num.x * 4;
    float min_adjacency = 1e20;
    ImageFragmentState best_frag_state;
    const unsigned int idx0 = frag_state0.orig_idx*4 + get_side_idx(frag_state0, dir0);
    const unsigned int idx1 = frag_state1.orig_idx*4 + get_side_idx(frag_state1, dir1);
    {
        const unsigned int dir_offset = get_dir_offset(dir0, dir1);
        for (const auto& fp : sorted_fp[idx0]) {
            if (fp.adjacency >= min_adjacency) break;
            const unsigned int idx2_adjacent_idx1 = fp.r.orig_idx*4 + (get_side_idx(fp.r, Direction::L) + dir_offset)%4;
            float tmp_adjacency = fp.adjacency + static_cast<float>(*(adjacency + idx1*N + idx2_adjacent_idx1));
            if (!placed[fp.r.orig_idx] && min_adjacency > tmp_adjacency) {
                min_adjacency = tmp_adjacency;
                ImageFragmentState tmp = fp.r;
                tmp.rotate_90deg(get_dir_offset(Direction::R, dir0));
                best_frag_state = tmp;
            }
        }
    }
    if (abs(min_adjacency - 1e20) < 1) {
        cerr << "get best adjacent fragment3" << endl;
        exit(-1);
    }
    return make_pair(min_adjacency, best_frag_state);
}


ImageState expand_image(ImageState img_state, Direction dir, unsigned int base_pos, const double *adjacency, const v2fp& sorted_fp, const Vec2<int>& div_num) {
    if (dir != Direction::D && dir != Direction::R) {
        cerr << "expand image" << endl;
        exit(-1);
    }
    const Vec2<unsigned short> now_size = img_state.now_size;
    if (dir == Direction::D) {
        pair<float, ImageFragmentState> tmp;
        tmp = get_best_adjacent_fragment(img_state.states[now_size.y-1][base_pos], Direction::D, img_state.placed, sorted_fp, div_num);
        img_state.place(Pos(base_pos, now_size.y), tmp.second, tmp.first);
        revrep(i,base_pos) {
            tmp = get_best_adjacent_fragment(img_state.states[now_size.y-1][i], Direction::D, img_state.states[now_size.y][i+1], Direction::L, img_state.placed, adjacency, sorted_fp, div_num);
            img_state.place(Pos(i, now_size.y), tmp.second, tmp.first);
        }
        repr(i,base_pos+1,now_size.x) {
            tmp = get_best_adjacent_fragment(img_state.states[now_size.y-1][i], Direction::D, img_state.states[now_size.y][i-1], Direction::R, img_state.placed, adjacency, sorted_fp, div_num);
            img_state.place(Pos(i, now_size.y), tmp.second, tmp.first);
        }
    } else /* if (dir == Direction::R) */ {
        pair<float, ImageFragmentState> tmp;
        tmp = get_best_adjacent_fragment(img_state.states[base_pos][now_size.x-1], Direction::R, img_state.placed, sorted_fp, div_num);
        img_state.place(Pos(now_size.x, base_pos), tmp.second, tmp.first);
        revrep(i,base_pos) {
            tmp = get_best_adjacent_fragment(img_state.states[i][now_size.x-1], Direction::R, img_state.states[i+1][now_size.x], Direction::U, img_state.placed, adjacency, sorted_fp, div_num);
            img_state.place(Pos(now_size.x, i), tmp.second, tmp.first);
        }
        repr(i,base_pos+1,now_size.y) {
            tmp = get_best_adjacent_fragment(img_state.states[i][now_size.x-1], Direction::R, img_state.states[i-1][now_size.x], Direction::D, img_state.placed, adjacency, sorted_fp, div_num);
            img_state.place(Pos(now_size.x, i), tmp.second, tmp.first);
        }
    }
    return img_state;
}

const unsigned short TIMES = 1000;
vector<ImageState> generate_next_states(const vector<ImageState>& current_states, const double *adjacency, const v2fp& sorted_fp, const Vec2<int>& div_num) {
    vector<ImageState> next_states;
    next_states.reserve(current_states.size()*4);
    for(const auto& state : current_states) {
        ImageState rotated_state = state; rotated_state.rotate_180deg();
        const Vec2<unsigned short>& now_size = state.now_size;
        const bool x_expandable = !(now_size.x < now_size.y && now_size.x == min(div_num.x, div_num.y)) && !(now_size.x > now_size.y && now_size.x == max(div_num.x, div_num.y));
        const bool y_expandable = !(now_size.y < now_size.x && now_size.y == min(div_num.x, div_num.y)) && !(now_size.y > now_size.x && now_size.y == max(div_num.x, div_num.y));
        if (x_expandable) {
            const unsigned int offset = max(1,state.now_size.y / TIMES);
            rep(i,min(TIMES, state.now_size.y)) {
                const unsigned int base_point = i*offset;
                next_states.push_back(expand_image(state, Direction::R, base_point, adjacency, sorted_fp, div_num));
                next_states.push_back(expand_image(rotated_state, Direction::R, base_point, adjacency, sorted_fp, div_num));
            }
        }
        if (y_expandable) {
          const unsigned int offset = max(1,state.now_size.x / TIMES);
          rep(i,min(TIMES, state.now_size.x)) {
              const unsigned int base_point = i*offset;
              next_states.push_back(expand_image(state, Direction::D, base_point, adjacency, sorted_fp, div_num));
              next_states.push_back(expand_image(rotated_state, Direction::D, base_point, adjacency, sorted_fp, div_num));
          }
      }
    }
    return next_states;
}
void remove_duplicate_state(vector<ImageState>& states) {
    set<size_t> hash_map;
    vector<ImageState> tmp;
    tmp.reserve(states.size());

    for(const auto& img_state : states) {
        // hash値計算
        string s;
        rep(i,MAX_DIV_NUM*MAX_DIV_NUM) s.push_back(img_state.placed[i]);
        size_t a = hash<string>()(s);
        size_t b = hash<string>()(to_string(img_state.adjacency_sum).substr(0, 6));
        size_t c = a^b;

        // 今までに存在しなければ保存
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
Answer get_answer(const vector<ImageState>& sorted_states, const Settings& settings) {
    const auto& div_num = static_cast<Vec2<short unsigned int>>(settings.div_num);
    for(const auto& img_state : sorted_states) {
        unsigned int rotation_times_00 = img_state.get_00_rotation_times();
        const auto& now_size = img_state.now_size;
        if ((rotation_times_00%2 == 0 && now_size == div_num) || (rotation_times_00%2 == 1 && now_size.x == div_num.y && now_size.y == div_num.x)) {
            ImageState tmp = img_state;
            tmp.rotate_90deg(4 - rotation_times_00);
            return tmp.convert_answer();
        }
    }
    cerr << "get_answer" << endl;
    exit(-1);
}

Answer SideBeamSearchSolver::operator()(double *adjacency, const Settings& settings, int argc, char *argv[]) {

//    constexpr unsigned int MAX_STATE_NUM = 64;
    constexpr unsigned int MAX_STATE_NUM = 128;
    const auto& div_num = settings.div_num;

    const auto& sorted_fp = get_sorted_fragment_pair(adjacency, div_num);


    // 横に二つ並べたStateを全列挙し、ソートしMAX_STATE_NUMまで取る
    auto current_states = get_initial_states(sorted_fp, div_num, MAX_STATE_NUM);

    const unsigned int STEP_NUM = div_num.y + div_num.x - 3;

    rep(step, STEP_NUM) {
        // 現在の城代から次の状態を列挙し重複を削除しソートする
        auto next_states = generate_next_states(current_states, adjacency, sorted_fp, div_num);
        remove_duplicate_state(next_states);
        sort_and_resize_states(next_states, MAX_STATE_NUM);
        current_states = move(next_states);
    }
    if (current_states.empty()) {
        cerr << "solve" << endl;
        exit(-1);
    }
    return get_answer(current_states, settings);
}
