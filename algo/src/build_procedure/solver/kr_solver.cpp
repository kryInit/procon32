#include "kr_solver.hpp"
#include <bits/stdc++.h>
#include <comparable_data.hpp>
#include <time_manager.hpp>
#include <utility.hpp>
#include <macro.hpp>
#include <omp.h>

using namespace std;

constexpr int MIN_DIV_NUM_Y = 2;
constexpr int MIN_DIV_NUM_X = 2;
constexpr int MIN_DIV_NUM   = MIN_DIV_NUM_Y * MIN_DIV_NUM_X;
constexpr int MAX_DIV_NUM_Y = 16;
constexpr int MAX_DIV_NUM_X = 16;
constexpr int MAX_DIV_NUM   = MAX_DIV_NUM_Y * MAX_DIV_NUM_X;

constexpr int MIN_SELECTABLE_TIMES = 2;
constexpr int MAX_SELECTABLE_TIMES = 128;

constexpr int MIN_SWAP_COST   = 1;
constexpr int MIN_SELECT_COST = 1;
constexpr int MAX_SWAP_COST   = 100;
constexpr int MAX_SELECT_COST = 500;

/* * * * * * * * * * * * * * * *
 * row   , UR: RUULDRDLUURDLDRU *
 * row   , DR: RDDLURULDDRULURD *
 * row   , DL: LDDRULURDDLURULD *
 * row   , UL: LUURDLDRUULDRDLU *
 * column, UR: URRDLULDRRULDLUR *
 * column, DR: DRRULDLURRDLULDR *
 * column, DL: DLLURDRULLDRURDL *
 * column, UL: ULLDRURDLLURDRUL *
  * * * * * * * * * * * * * * * */

const Path row_UR = Path::from_string("RUULDRDLUURDLDRU");
const Path row_DR = Path::from_string("RDDLURULDDRULURD");
const Path row_DL = Path::from_string("LDDRULURDDLURULD");
const Path row_UL = Path::from_string("LUURDLDRUULDRDLU");
const Path column_UR = Path::from_string("URRDLULDRRULDLUR");
const Path column_DR = Path::from_string("DRRULDLURRDLULDR");
const Path column_DL = Path::from_string("DLLURDRULLDRURDL");
const Path column_UL = Path::from_string("ULLDRURDLLURDRUL");


void join_path(Path &p1, const Path& p2) { p1.insert(p1.end(), p2.begin(), p2.end()); }

void dump_original_positions(const OriginalPositions& original_positions) {
    for(const auto& tmp : original_positions) {
        for(const auto& p : tmp) {
            stringstream ss;
            ss << hex << uppercase << p.x << p.y;
            cout << ss.str() << " ";
        }
        cout << endl;
    }
}

void reduce_path(Path& path) {
    Path tmp_path; tmp_path.reserve(path.size());
    for(const auto& dir : path) {
        if (tmp_path.empty()) tmp_path.push_back(dir);
        else if (dir != tmp_path.back().get_dir_reversed()) tmp_path.push_back(dir);
        else tmp_path.pop_back();
    }
    path = tmp_path;
}

void optimize_procedures(Procedures& procs) {
    Procedures tmp_procs; tmp_procs.reserve(procs.size());
    for(auto& proc : procs) {
        reduce_path(proc.path);
        if (!proc.path.empty()) tmp_procs.push_back(proc);
    }
    procs = tmp_procs;
}

int calc_cost(const Procedures& procs, const Settings& settings) {
    int cost = 0;
    cost += procs.size() * settings.choice_cost;
    for(const auto& proc : procs) cost += proc.path.size() * settings.swap_cost;
    return cost;
}

/*/
 *
 * 2x2の場合は完全解を求めることが可能
 * 3x3もギリいけるかも
 * でも3x3は実用性薄そう、一応制限にもなってしまうし
 *
 * 最初に分ける点を決め打って探索した方が良いのかもしれない
 * 実は盤外移動するメリットはあんまりない？
 *
 * 最初に適当に揃える時には盤外移動を考慮するけどちゃんと揃えるときは移動しないようにしたほうが楽なのかもしれない
 * 偶奇性の調整は最初でやっておくと良いかも
 *
 * sort_roughlyを実行
 * グリッドサーチ的にやっていく
 *
 * 近傍として部分的な盤面を埋めることを用意するとか
 *
 *
/*/

struct State {
    static constexpr char ordinary = 0, sorted = 1, unmovable = 2;
    Pos selected_pos, first_sorted_pos;
    Procedures proc;
    array<array<char, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> state;
    array<array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> now_orig_pos;


    State() : selected_pos(Pos(-1,-1)) , first_sorted_pos() , proc(), state(), now_orig_pos() {}
    State(const OriginalPositions& orig_pos)
        : selected_pos(Pos(-1,-1))
        , first_sorted_pos()
        , proc(), state(), now_orig_pos() {
        rep(i, orig_pos.size()) rep(j, orig_pos[i].size()) now_orig_pos[i][j] = orig_pos[i][j];
    }
    State(const OriginalPositions& orig_pos, Pos first_selected_pos)
        : selected_pos(first_selected_pos)
        , first_sorted_pos()
        , proc(), state(), now_orig_pos() {
        rep(i, orig_pos.size()) rep(j, orig_pos[i].size()) now_orig_pos[i][j] = orig_pos[i][j];
        proc.emplace_back(first_selected_pos, Path());
    }
    void dump(const Vec2<int>& div_num) const {
//        PRINT(selected_pos);
//        DUMP("proc:");
//        DUMP(proc);
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                stringstream ss;
                ss << hex << uppercase << now_orig_pos[i][j].x << now_orig_pos[i][j].y << "'";
                if (state[i][j] != ordinary) ss << (int)state[i][j];
                else ss << " ";
                cout << ss.str() << " ";
            }
            cout << endl;
        }
    }
};

class Context {
protected:
    const Vec2<int> div_num;
    const int selectable_times;
    const int select_cost, swap_cost;

    Context(Vec2<int> div_num, int selectable_times, int select_cost, int swap_cost)
        : div_num(div_num), selectable_times(selectable_times), select_cost(select_cost), swap_cost(swap_cost) {}
};

class ISorter : protected Context {
protected:
    void move_toraly(Pos& p, const Pos& dp) const {
        p += dp;
        p.x = (p.x + div_num.x) % div_num.x;
        p.y = (p.y + div_num.y) % div_num.y;
    }
    void move_toraly(Pos& p, const Direction& dir) const {
        move_toraly(p, Pos(dir));
    }
    [[nodiscard]] Pos get_moved_toraly_pos(Pos p, const Pos& dp) const {
        move_toraly(p, dp);
        return p;
    }
    [[nodiscard]] Pos get_moved_toraly_pos(Pos p, const Direction& dir) const {
        move_toraly(p, dir);
        return p;
    }

    void move_selected_pos(const Direction& dir, State& state) const {
        const Pos now = state.selected_pos;
        const Pos next = get_moved_toraly_pos(now, dir);
        swap(state.now_orig_pos[now.y][now.x], state.now_orig_pos[next.y][next.x]);
        state.selected_pos = next;
    }
    void revert_move_selected_pos(const Direction& dir, State& state) const {
        move_selected_pos(dir.get_dir_reversed(), state);
    }
    void move_selected_pos(const Path& path, State& state) const {
        for(const auto& dir : path) move_selected_pos(dir, state);
    }
    void revert_move_selected_pos(const Path& path, State& state) const {
        revrep(i, path.size()) revert_move_selected_pos(path[i], state);
    }

    [[nodiscard]] Pos get_now_pos_by_original_pos(Pos orig_pos, const State& state) const {
        rep(i,div_num.y) rep(j,div_num.x) if (state.now_orig_pos[i][j] == orig_pos) return Vec2(j,i);
        EXIT_DEBUG("reached end");
    }

    [[nodiscard]] bool is_completable(const State& state) const {
        const int n = div_num.x * div_num.y;
        vector<int> now_order(n), rev_now_order(n);
        rep(i, div_num.y) rep(j, div_num.x) {
            int idx = Pos(j,i).to_idx(div_num.x);
            int orig_idx = state.now_orig_pos[i][j].to_idx(div_num.x);
            now_order[idx] = orig_idx;
            rev_now_order[orig_idx] = idx;
        }
        int swap_count = 0;
        rep(i,n) {
            if (now_order[i] != i) {
                int target_idx = rev_now_order[i];
                swap(now_order[i], now_order[target_idx]);
                rev_now_order[i] = i;
                rev_now_order[now_order[target_idx]] = target_idx;
                swap_count++;
            }
        }
        int dist = state.selected_pos.sum();
        PRINT(dist);
        PRINT(swap_count);
        return ((swap_count+dist)%2) == 0;
    }
    void check_ans(OriginalPositions initial_orig_pos, const Procedures &procs) const {
        auto& now_orig_pos = initial_orig_pos;
        for(const auto &proc : procs) {
            Pos current = proc.selected_pos;
            Pos tmp = now_orig_pos[current.y][current.x];
            for(const auto dir : proc.path) {
                Pos next = current; move_toraly(next, dir);
                now_orig_pos[current.y][current.x] = now_orig_pos[next.y][next.x];
                current = next;
            }
            now_orig_pos[current.y][current.x] = tmp;
        }
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                cout << hex << uppercase << now_orig_pos[i][j].x << now_orig_pos[i][j].y << " ";
            }
            cout << endl;
        }
        cout << endl;
        rep(i,div_num.y) rep(j,div_num.x) if (Vec2(j,i) != now_orig_pos[i][j]) {
            EPRINT(i, j, Vec2(j,i), now_orig_pos[i][j]);
            exit(-1);
        }
    }

    ISorter(Vec2<int> div_num, int selectable_times, int select_cost, int swap_cost)
        : Context(div_num, selectable_times, select_cost, swap_cost) {}
};

class RoughSorter : protected ISorter {
protected:

    [[nodiscard]] int calc_mh_dist_toraly(const Pos& p1, const Pos& p2) const {
        const int dx = abs(p1.x - p2.x);
        const int dy = abs(p1.y - p2.y);
        return min(dx, div_num.x-dx) + min(dy, div_num.y-dy);
    }
    [[nodiscard]] double calc_penalty(const Pos& p, const State& state) const {
        const int mh_dist = calc_mh_dist_toraly(p, state.now_orig_pos[p.y][p.x]);
        return mh_dist*mh_dist;
    }

    double move_and_calc_diff_penalty(const Direction& dir, State& state) const {
        const Pos now = state.selected_pos;
        const Pos next = get_moved_toraly_pos(now, dir);
        int diff_penalty = -calc_penalty(next, state);
//        move_selected_pos(dir, state);
        swap(state.now_orig_pos[now.y][now.x], state.now_orig_pos[next.y][next.x]);
        state.selected_pos = next;
        diff_penalty += calc_penalty(now, state);
        return diff_penalty;
    }
    double revert_move_and_calc_diff_penalty(const Direction& dir, State& state) const {
        return move_and_calc_diff_penalty(dir.get_dir_reversed(), state);
    }

public:
    RoughSorter(Vec2<int> div_num, int selectable_times, int select_cost, int swap_cost)
        : ISorter(div_num, selectable_times, select_cost, swap_cost) {}

    Pos get_max_penalty_pos(State& state) {
        Pos p(-1, -1);
        int max_penalty = 0;
        rep(i,div_num.y) rep(j,div_num.x) {
            int tmp_penalty = calc_penalty(Vec2(j,i), state);
            if (max_penalty < tmp_penalty) {
                max_penalty = tmp_penalty;
                p = Vec2(j,i);
            }
        }
        return p;
    }

    double sort_roughly(State& state, int time_limit = 5000, int depth = 9) {
        State original_state = state;
        const int dnx = div_num.x;
        const int dny = div_num.y;
        double coef = 0.0;

        double now_penalty = 0;
        rep(i,dny) rep(j,dnx) if (Pos(j,i) != state.selected_pos) now_penalty += calc_penalty(Pos(j,i), state);
        PRINT(now_penalty);

        int loop_count = 0;
        int prev_update_time = -1;
        int prev_upgrade_time = -1;

        double best_penalty = now_penalty;
        Path best_path;

        Path now_path;
        TimeManager tm(time_limit);

        {
            int count = 0;
            rep(i, dny) {
                rep(j,dnx) {
                    int penalty = calc_penalty(Vec2(j,i), state);
                    if (penalty == 0) cout << "   .";
                    else cout << setw(4) << penalty, count++;
                }
                cout << endl;
            }
            cout << endl;
        }

///*

        constexpr int threads_num = 12;
        array<double, threads_num> min_diff_penalties{};
        array<Path, threads_num> best_addnl_paths{}, initial_addnl_paths{};
        {
            constexpr Direction U = Direction::U, R = Direction::R, D = Direction::D, L = Direction::L;
            initial_addnl_paths = {{{{U,U}}, {{U,R}}, {{U,L}}, {{R,U}}, {{R,R}}, {{R,D}}, {{D,R}}, {{D,D}}, {{D,L}}, {{L,U}}, {{L,D}}, {{L,L}}}};
        }

//*/
        while(tm.in_time_limit()) {
            PRINT(loop_count, now_path.size(), now_penalty, best_penalty);
            loop_count++;

            // serach additional path


///*
            rep(i,threads_num) min_diff_penalties[i] = 0, best_addnl_paths[i] = Path();

#pragma omp parallel for num_threads(threads_num)
            for(int i=0; i<threads_num; ++i) {
//            rep(i,threads_num) {
                State tmp_state = state;

                double min_diff_penalty = 0;
                Path best_addnl_path;

                double now_diff_penalty = 0;
                Path now_addnl_path = initial_addnl_paths[i];

                now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path[0], tmp_state);

                {
                    double tmp_now_diff_penalty = now_diff_penalty +  coef;
                    if ((min_diff_penalty > tmp_now_diff_penalty) || (min_diff_penalty == tmp_now_diff_penalty && best_addnl_path.size() > now_addnl_path.size())) {
                        min_diff_penalty = tmp_now_diff_penalty;
                        best_addnl_path.push_back(now_addnl_path[0]);
                    }
                }
                now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path[1], tmp_state);

                while(true) {
                    if ((int)now_addnl_path.size() == depth) {
                        while(now_addnl_path.size() > 2 && now_addnl_path.back() == Direction::L) {
                            now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), tmp_state);
                            now_addnl_path.pop_back();
                        }
                        if (now_addnl_path.size() == 2) break;
                        now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), tmp_state);
                        now_addnl_path.back().rotate_cw();
                        while(now_addnl_path.size() >= 3) {
                            int idx = now_addnl_path.size()-2;
                            if (Direction(now_addnl_path[idx].get_dir_reversed()) == now_addnl_path.back()) {
                                if (now_addnl_path.back() == Direction::L) {
                                    now_addnl_path.pop_back();
                                    now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), tmp_state);
                                    if (now_addnl_path.size() == 2) break;
                                }
                                now_addnl_path.back().rotate_cw();
                            } else break;
                        }
                        if (now_addnl_path.size() == 2) break;
                    } else {
                        if (now_addnl_path.back() != Direction::D) now_addnl_path.push_back(Direction::U);
                        else now_addnl_path.push_back(Direction::R);
                    }

                    now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path.back(), tmp_state);

                    double tmp_now_diff_penalty = now_diff_penalty + now_addnl_path.size() * coef;
                    if ((min_diff_penalty > tmp_now_diff_penalty) || (min_diff_penalty == tmp_now_diff_penalty && best_addnl_path.size() > now_addnl_path.size())) {
                        min_diff_penalty = tmp_now_diff_penalty;
                        best_addnl_path = now_addnl_path;
                    }
                }

                min_diff_penalties[i] = min_diff_penalty;
                best_addnl_paths[i] = best_addnl_path;
            }

            double min_diff_penalty = 1e20;
            Path best_addnl_path;

            rep(i,threads_num) {
                if (min_diff_penalty > min_diff_penalties[i] || (min_diff_penalty == min_diff_penalties[i] && best_addnl_paths.size() > best_addnl_paths[i].size())) {
                    min_diff_penalty = min_diff_penalties[i];
                    best_addnl_path = best_addnl_paths[i];
                }
            }

//*/

            if (min_diff_penalty < 0) {// if I found good additional path
                // add path and change
                for(const auto& dir : best_addnl_path) move_selected_pos(dir, state);
                join_path(now_path, best_addnl_path);
                now_penalty += min_diff_penalty;
                prev_update_time = loop_count;
                if ((best_penalty > now_penalty) || (best_penalty == now_penalty && best_path.size() > now_path.size())) {
                    best_penalty = now_penalty;
                    best_path = now_path;
                    PRINT(loop_count, best_penalty);
                    prev_upgrade_time = loop_count;
                }
            } else if (!now_path.empty()) { // if I couldn't find additional path
                // kick
                if (loop_count > prev_upgrade_time*100) break;
                else if (loop_count - prev_update_time > 300) {
                    now_path = best_path;
                    now_penalty = best_penalty;
                    state = original_state;
                    for(const auto& dir : now_path) move_selected_pos(dir, state);
                    prev_update_time = loop_count;
                } else {
                    now_penalty -= now_path.size() * coef;
                    int delete_num = Random::simple_exp_rand(now_path.size()) + 1;
                    rep(_,delete_num) {
                        now_penalty += revert_move_and_calc_diff_penalty(now_path.back(), state);
                        now_path.pop_back();
                    }
                    now_penalty += now_path.size() * coef;
                }
            } else break;
        }
        PRINT(loop_count);
        PRINT(prev_update_time);
        PRINT(prev_upgrade_time);
        PRINT(best_penalty);
        PRINT(best_path.size());
        PRINT(best_path);

        state = original_state;
        join_path(state.proc.back().path, best_path);
        for(const auto& dir : best_path) move_selected_pos(dir, state);

        double tmp_penalty = - calc_penalty(state.selected_pos, state);
        rep(i,dny) rep(j,dnx) tmp_penalty += calc_penalty(Pos(j,i), state);
        PRINT(tmp_penalty);
        PRINT(tmp_penalty + best_path.size()*coef);
        int count = 0;
        rep(i, dny) {
            rep(j,dnx) {
                int penalty = calc_penalty(Vec2(j,i), state);
                if (penalty == 0) cout << "  .";
                else cout << setw(3) << penalty, count++;
            }
            cout << endl;
        }
        cout << endl;
        PRINT(count);

        rep(i, dny) {
            rep(j,dnx) {
                if (Pos(j,i) == state.now_orig_pos[i][j]) cout << ". ";
                else {
                    for(const auto& dir : Direction::All) {
                        if (get_moved_toraly_pos(Pos(j,i), dir) == state.now_orig_pos[i][j]) {
                            cout << dir << " ";
                            break;
                        }
                        else if (dir == Direction::L) cout << (int)calc_penalty(Vec2(j,i), state) << " ";
                    }
                }
            }
            cout << endl;
        }
        cout << endl;

        return best_penalty;
    }
    double sort__roughly(State& state, int time_limit = 5000) {
        State original_state = state;
        const int dnx = div_num.x;
        const int dny = div_num.y;
        double coef = 0.;

        double now_penalty = 0;
        rep(i,dny) rep(j,dnx) if (Pos(j,i) != state.selected_pos) now_penalty += calc_penalty(Pos(j,i), state);

        int loop_count = 0;
        int prev_update_time = -1;
        int prev_upgrade_time = -1;

        double best_penalty = now_penalty;
        Path best_path;

        Path now_path;
        TimeManager tm(time_limit);
        constexpr int depth = 9;

        while(tm.in_time_limit()) {
            loop_count++;

            double min_diff_penalty = 1e20;
            Path best_addnl_path;

            double now_diff_penalty = 0;
            Path now_addnl_path; now_addnl_path.reserve(depth);
            while(true) {
                if (now_addnl_path.size() == depth) {
                    while(!now_addnl_path.empty() && now_addnl_path.back() == Direction::L) {
                        now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), state);
                        now_addnl_path.pop_back();
                    }
                    if (now_addnl_path.empty()) break;
                    now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), state);
                    now_addnl_path.back().rotate_cw();
                    while(now_addnl_path.size() >= 2) {
                        int idx = now_addnl_path.size()-2;
                        if (Direction(now_addnl_path[idx].get_dir_reversed()) == now_addnl_path.back()) {
                            if (now_addnl_path.back() == Direction::L) {
                                now_addnl_path.pop_back();
                                now_diff_penalty += revert_move_and_calc_diff_penalty(now_addnl_path.back(), state);
                            }
                            now_addnl_path.back().rotate_cw();
                        } else break;
                    }
                } else {
                    if (now_addnl_path.empty() || now_addnl_path.back() != Direction::D) now_addnl_path.push_back(Direction::U);
                    else now_addnl_path.push_back(Direction::R);
                }

                now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path.back(), state);

                double tmp_now_diff_penalty = now_diff_penalty + now_addnl_path.size() * coef;
                if ((min_diff_penalty > tmp_now_diff_penalty) || (min_diff_penalty == tmp_now_diff_penalty && best_addnl_path.size() > now_addnl_path.size())) {
                    min_diff_penalty = tmp_now_diff_penalty;
                    best_addnl_path = now_addnl_path;
                }
            }

            if (min_diff_penalty < 0) {// if I found good additional path
                // add path and change
                for(const auto& dir : best_addnl_path) move_selected_pos(dir, state);
                join_path(now_path, best_addnl_path);
                now_penalty += min_diff_penalty;
                prev_update_time = loop_count;
                if ((best_penalty > now_penalty) || (best_penalty == now_penalty && best_path.size() > now_path.size())) {
                    best_penalty = now_penalty;
                    best_path = now_path;
                    prev_upgrade_time = loop_count;
                }
            } else if (!now_path.empty()) { // if I couldn't find additional path
                // kick
                if (loop_count > prev_upgrade_time*100) break;
                else if (loop_count - prev_update_time > 300) {
                    now_path = best_path;
                    now_penalty = best_penalty;
                    state = original_state;
                    for(const auto& dir : now_path) move_selected_pos(dir, state);
                    prev_update_time = loop_count;
                } else {
                    now_penalty -= now_path.size() * coef;
                    int delete_num = Random::simple_exp_rand(now_path.size()) + 1;
                    rep(_,delete_num) {
                        now_penalty += revert_move_and_calc_diff_penalty(now_path.back(), state);
                        now_path.pop_back();
                    }
                    now_penalty += now_path.size() * coef;
                }
            } else break;
        }

        state = original_state;
        join_path(state.proc.back().path, best_path);
        for(const auto& dir : best_path) move_selected_pos(dir, state);
        return best_penalty;
    }
};

class StrictSorter : RoughSorter {
    bool caluclated_best_procs_for_2x2;
    array<Procedures, 4096> best_procs_for_2x2;

    [[nodiscard]] optional<Path> calc_shortest_path(const Pos& s, const Pos& t, const State& state) const {
        const int dnx = div_num.x, dny = div_num.y;
        static deque<pair<Pos, Path>> que;
        static array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> visited{};
        que.clear();
        rep(i,dny) rep(j,dnx) visited[i][j] = false;

        if (state.state[s.y][s.x] != state.ordinary) {
            auto message = Utility::concat("state[s_idx] is ", state.state[s.y][s.x]);
            EXIT_DEBUG(message);
        }
        if (state.state[t.y][t.x] != state.ordinary) {
            auto message = Utility::concat("state[t_idx] is ", state.state[t.y][t.x]);
            EXIT_DEBUG(message);
        }

        que.emplace_back(s, Path());
        while(!que.empty()) {
            Path path = que.front().second;
            const Pos now = que.front().first;
            que.pop_front();
            if (now == t) return path;
            if (visited[now.y][now.x]) continue;
            visited[now.y][now.x] = true;
            for(const auto& dir : Direction::All) {
                Pos next = get_moved_toraly_pos(now, dir);
                if (state.state[next.y][next.x] == state.ordinary) {
                    path.push_back(dir);
                    que.emplace_back(next, path);
                    path.pop_back();
                }
            }
        }
        return nullopt;
    }
    [[nodiscard]] optional<Path> calc_nice_path(const Pos& s, const Pos& t, const State& state) const {
        const int dnx = div_num.x, dny = div_num.y;
        static deque<pair<Pos, Path>> que;
        static array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> visited{};
        que.clear();
        rep(i,dny) rep(j,dnx) visited[i][j] = false;

        if (state.state[s.y][s.x] != state.ordinary) {
            auto message = Utility::concat("state[s_idx] is ", state.state[s.y][s.x]);
            EXIT_DEBUG(message);
        }
        if (state.state[t.y][t.x] != state.ordinary) {
            auto message = Utility::concat("state[t_idx] is ", state.state[t.y][t.x]);
            EXIT_DEBUG(message);
        }

        que.emplace_back(s, Path());
        while(!que.empty()) {
            Path path = que.front().second;
            const Pos now = que.front().first;
            que.pop_front();
            if (now == t) return path;
            if (visited[now.y][now.x]) continue;
            visited[now.y][now.x] = true;
            for(const auto& dir : Direction::All) {
                Pos next = get_moved_toraly_pos(now, dir);
                if (state.state[next.y][next.x] == state.ordinary) {
                    bool back = 0;

                    {
                        Pos orig_next = state.now_orig_pos[next.y][next.x];

                        int now_to_orig_next_dist = 0;
                        int next_to_orig_next_dist = 0;

                        const Pos offset = state.first_sorted_pos;
                        Pos shifted_now = now + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;
                        Pos shifted_next = next + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;
                        Pos shifted_orig_next = orig_next + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;

                        now_to_orig_next_dist += abs(shifted_now.x - shifted_orig_next.x);
                        now_to_orig_next_dist += abs(shifted_now.y - shifted_orig_next.y);

                        next_to_orig_next_dist += abs(shifted_next.x - shifted_orig_next.x);
                        next_to_orig_next_dist += abs(shifted_next.y - shifted_orig_next.y);

                        back = now_to_orig_next_dist > next_to_orig_next_dist;
                    }

                    path.push_back(dir);
                    if (back) que.emplace_back(next, path);
                    else que.emplace_front(next, path);
                    path.pop_back();
                }
            }
        }
        return nullopt;
    }

    tuple<Path, bool> move_target_to_destination_by_selected_pos(const Pos& target, const Pos& destination, State& state) {
        if (state.state[target.y][target.x] != state.ordinary) {
            EXIT_DEBUG("target's state isn't ordinary");
        }

        Pos now = target;
        Path path;
        Path targets_path = calc_shortest_path(target, destination, state).value();
        for(const auto& dir : targets_path) {
            Pos next = get_moved_toraly_pos(now, dir);
            state.state[now.y][now.x] = state.unmovable;
            optional<Path> optional_additional_path = calc_nice_path(state.selected_pos, next, state);
            state.state[now.y][now.x] = state.ordinary;
            if (optional_additional_path) {
                Path& additional_path = optional_additional_path.value();
                additional_path.push_back(dir.get_dir_reversed());

                move_selected_pos(additional_path, state);
                join_path(path, additional_path);
                now = next;
            } else return {path, false};
        }
        return {path, true};
    }

    void move_to_correct_pos(const Pos& target, State& state) {
        Pos destination = state.now_orig_pos[target.y][target.x];
        if (target == destination) {
            state.state[destination.y][destination.x] = state.sorted;
            return;
        }
        auto [path, succeeded] = move_target_to_destination_by_selected_pos(target, destination, state);
        if (succeeded) {
            join_path(state.proc.back().path, path);
            state.state[destination.y][destination.x] = state.sorted;
        } else EXIT_DEBUG("failed");
    }

    void move_to_correct_pos(const Pos& target, const Pos& first_buddy, Direction free_dir, State& state) {
        const Pos orig_target = state.now_orig_pos[target.y][target.x];
        const Pos orig_buddy = state.now_orig_pos[first_buddy.y][first_buddy.x];

        if (target == orig_target && first_buddy == orig_buddy) {
            state.state[orig_target.y][orig_target.x] = state.sorted;
            state.state[orig_buddy.y][orig_buddy.x] = state.sorted;
            return;
        }
        const Pos destination1 = orig_buddy;
        const Pos destination2 = get_moved_toraly_pos(orig_buddy, free_dir);
        const Pos destination3 = orig_target;
        Direction target_to_buddy_dir = Direction::U;
        rep(_,4) {
            if (get_moved_toraly_pos(orig_target, target_to_buddy_dir) == orig_buddy) break;
            target_to_buddy_dir.rotate_cw();
            if (_ == 3) EXIT_DEBUG("");
        }

        if (state.state[destination1.y][destination1.x] != state.ordinary || state.state[destination2.y][destination2.x] != state.ordinary) {
            EXIT_DEBUG("");
        }

        Path path, tmp_path;
        bool succeeded;

        tie(tmp_path, succeeded) = move_target_to_destination_by_selected_pos(target, destination1, state);
        if (!succeeded) EXIT_DEBUG("");
        join_path(path, tmp_path);
        state.state[destination1.y][destination1.x] = state.unmovable;

        Pos now_buddy = get_now_pos_by_original_pos(orig_buddy, state);
        tie(tmp_path, succeeded) = move_target_to_destination_by_selected_pos(now_buddy, destination2, state);
        join_path(path, tmp_path);
        state.state[destination2.y][destination2.x] = state.unmovable;
        if (!succeeded) {
            if (state.selected_pos != orig_target) EXIT_DEBUG("moving_pos, orig_target: ",state.selected_pos,orig_target);
            if (target_to_buddy_dir.is_vertical() == free_dir.is_vertical()) EXIT_DEBUG("");
            if (target_to_buddy_dir == Direction::U) {
                if (free_dir == Direction::R) tmp_path = column_UR;
                else tmp_path = column_UL;
            } else if (target_to_buddy_dir == Direction::R) {
                if (free_dir == Direction::U) tmp_path = row_UR;
                else tmp_path = row_DR;
            } else if (target_to_buddy_dir == Direction::D) {
                if (free_dir == Direction::R) tmp_path = column_DR;
                else tmp_path = column_DL;
            } else {
                if (free_dir == Direction::U) tmp_path = row_UL;
                else tmp_path = row_DL;
            }
            state.state[destination1.y][destination1.x] = state.ordinary;
            state.state[destination2.y][destination2.x] = state.ordinary;

            move_selected_pos(tmp_path, state);
            join_path(path, tmp_path);

            join_path(state.proc.back().path, path);
            state.state[orig_target.y][orig_target.x] = state.sorted;
            state.state[orig_buddy.y][orig_buddy.x] = state.sorted;
            return;
        }

        auto opt_path = calc_nice_path(state.selected_pos, destination3, state);
        if (!opt_path) EXIT_DEBUG("");
        move_selected_pos(opt_path.value(), state);
        join_path(path, opt_path.value());

        state.state[destination1.y][destination1.x] = state.ordinary;
        state.state[destination2.y][destination2.x] = state.ordinary;

        tmp_path = {target_to_buddy_dir, free_dir};
        move_selected_pos(tmp_path, state);
        join_path(path, tmp_path);

        join_path(state.proc.back().path, path);
        state.state[orig_target.y][orig_target.x] = state.sorted;
        state.state[orig_buddy.y][orig_buddy.x] = state.sorted;
    }

    struct TransitionType {
        Pos target;
        bool has_buddy;
        Pos buddy;
        Direction free_dir;
        TransitionType(Pos target) : target(target), has_buddy(false), buddy(), free_dir() {}
        TransitionType(Pos target, Pos buddy, Direction free_dir) : target(target), has_buddy(true), buddy(buddy), free_dir(free_dir) {}
    };

    [[nodiscard]] const vector<TransitionType> get_all_possible_transition_types(const State& state) const {
        vector<TransitionType> transition_types;

        const int dnx = div_num.x, dny = div_num.y;
        const int n = dnx*dny;
        array<array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> rev_now_orig_pos{};
        static array<unsigned char, MAX_DIV_NUM_X> vertical_sorted_count{};
        static array<unsigned char, MAX_DIV_NUM_Y> horizontal_sorted_count{};

        rep(i,dny) rep(j,dnx) rev_now_orig_pos[state.now_orig_pos[i][j].y][state.now_orig_pos[i][j].x] = Vec2(j,i);
        rep(i,dny) horizontal_sorted_count[i] = 0;
        rep(j,dnx) vertical_sorted_count[j] = 0;

        int sorted_count = 0;
        rep(i, dny) rep(j, dnx) if (state.state[i][j] == state.sorted) {
            sorted_count++;
            vertical_sorted_count[j]++;
            horizontal_sorted_count[i]++;
        }
        if (sorted_count+4 == n) return {};

        rep(j,dnx) if (vertical_sorted_count[j] == dny-2) rep(i,dny) {
            const Pos orig_p1 = Vec2(j,i);
            const Pos orig_p2 = get_moved_toraly_pos(orig_p1,Direction::D);
            if (state.state[orig_p1.y][orig_p2.x] == state.ordinary && state.state[orig_p2.y][orig_p2.x] == state.ordinary) {
                const Pos p1 = rev_now_orig_pos[orig_p1.y][orig_p1.x];
                const Pos p2 = rev_now_orig_pos[orig_p2.y][orig_p2.x];
                if (p1 == state.selected_pos || p2 == state.selected_pos) continue;
                if (j == state.first_sorted_pos.x) {
                    transition_types.emplace_back(p1, p2, Direction::L);
                    transition_types.emplace_back(p1, p2, Direction::R);
                    transition_types.emplace_back(p2, p1, Direction::L);
                    transition_types.emplace_back(p2, p1, Direction::R);
                } else if (vertical_sorted_count[(j+1)%dnx] == dny) {
                    transition_types.emplace_back(p1, p2, Direction::L);
                    transition_types.emplace_back(p2, p1, Direction::L);
                } else if (vertical_sorted_count[(j+dnx-1)%dnx] == dny) {
                    transition_types.emplace_back(p1, p2, Direction::R);
                    transition_types.emplace_back(p2, p1, Direction::R);
                }
                break;
            }
        }
        rep(i,dny) if (horizontal_sorted_count[i] == dnx-2) rep(j,dnx) {
            const Pos orig_p1 = Vec2(j,i);
            const Pos orig_p2 = get_moved_toraly_pos(orig_p1,Direction::R);
            if (state.state[orig_p1.y][orig_p1.x] == state.ordinary && state.state[orig_p2.y][orig_p2.x] == state.ordinary) {
                const Pos p1 = rev_now_orig_pos[orig_p1.y][orig_p1.x];
                const Pos p2 = rev_now_orig_pos[orig_p2.y][orig_p2.x];
                if (p1 == state.selected_pos || p2 == state.selected_pos) continue;
                if (i == state.first_sorted_pos.y) {
                    transition_types.emplace_back(p1, p2, Direction::U);
                    transition_types.emplace_back(p1, p2, Direction::D);
                    transition_types.emplace_back(p2, p1, Direction::U);
                    transition_types.emplace_back(p2, p1, Direction::D);
                } else if (horizontal_sorted_count[(i+1)%dny] == dnx) {
                    transition_types.emplace_back(p1, p2, Direction::U);
                    transition_types.emplace_back(p2, p1, Direction::U);
                } else if (horizontal_sorted_count[(i+dny-1)%dny] == dnx) {
                    transition_types.emplace_back(p1, p2, Direction::D);
                    transition_types.emplace_back(p2, p1, Direction::D);
                }
                break;
            }
        }
        rep(i,dny) if (horizontal_sorted_count[i] != dnx-2) rep(j,dnx) if (vertical_sorted_count[j] != dny-2) {
            const Pos orig_candidate = Pos(j,i);
            const Pos candidate = rev_now_orig_pos[orig_candidate.y][orig_candidate.x];
            if (candidate != state.selected_pos && state.state[orig_candidate.y][orig_candidate.x] != state.sorted) {
                for(const auto& dir : Direction::All) {
                    const Pos p1 = get_moved_toraly_pos(orig_candidate, dir);
                    if (state.state[p1.y][p1.x] == state.sorted) {
                        if (i == state.first_sorted_pos.y || j == state.first_sorted_pos.x) {
                            transition_types.emplace_back(candidate);
                            break;
                        }
                        const Pos p2 = get_moved_toraly_pos(orig_candidate, dir.get_dir_rotated_cw());
                        if (state.state[p2.y][p2.x] == state.sorted) {
                            const Pos dp = Pos(dir.get_dir_rotated_ccw()) + Pos(dir.get_dir_reversed());
                            const Pos p3 = get_moved_toraly_pos(orig_candidate, dp*2);
                            if (state.state[p3.y][p3.x] != state.sorted) {
                                transition_types.emplace_back(candidate);
                                break;
                            }
                        }
                    }
                }
            }
        }

        return transition_types;
    }

    void calc_best_procs_for_2x2() {
        if (caluclated_best_procs_for_2x2) return;
        caluclated_best_procs_for_2x2 = true;
        constexpr array<int, 4> offset{16,64,256, 1024};
        array<bool, 4096> visited{};
        Utility::reverse_priority_queue<ComparableData<int, tuple<int, int, vector<int>, Procedures>>> pq;

        rep(i,4) {
            vector<int> order = {0,1,3,2};
            SingleProcedure sp(Pos(order[i]%2, order[i]/2), Path());
            pq.emplace(0, make_tuple(1, i, order, Procedures{sp}));
        }

        while(!pq.empty()) {
            int cost = pq.top().rating;
            int select_times, selected_idx;
            vector<int> order;
            Procedures proc;
            tie(select_times, selected_idx, order, proc) = pq.top().data;
            pq.pop();

            int now_idx = select_times + 4*selected_idx;
            rep(i,4) now_idx += order[i]*offset[i];
            if (visited[now_idx]) continue;
            repr(i, select_times, 4) {
                int tmp_idx = now_idx - select_times + i;
                if (visited[tmp_idx]) break;
                visited[tmp_idx] = true;
                best_procs_for_2x2[tmp_idx] = proc;
            }

            {
                int swap_target_idx = (selected_idx+1)%4;
                auto tmp_order = order;
                auto tmp_proc = proc;
                swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
                join_path(tmp_proc.back().path, {Direction(Direction::R).get_dir_rotated_cw(selected_idx)});
                pq.emplace(cost+swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
            }
            {
                int swap_target_idx = (selected_idx+3)%4;
                auto tmp_order = order;
                auto tmp_proc = proc;
                swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
                join_path(tmp_proc.back().path, {Direction(Direction::D).get_dir_rotated_cw(selected_idx)});
                pq.emplace(cost+swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
            }
            if (select_times < 3) rep(i,4) if (selected_idx != i) {
                auto tmp_proc = proc;
                constexpr array<int, 4> tmp{0,1,3,2};
                tmp_proc.emplace_back(Pos(tmp[i]%2, tmp[i]/2), Path());
                pq.emplace(cost+select_cost, make_tuple(select_times+1, i, order, tmp_proc));
            }
        }

        for(auto& procs : best_procs_for_2x2) {
            for(auto& proc : procs) {
                for(auto& dir : proc.path) {
                    proc.selected_pos.move(dir);
                    dir.reverse();
                }
                reverse(all(proc.path));
            }
            reverse(all(procs));
        }
    }

    void transition(const TransitionType& tt, State& state) {
        if (tt.has_buddy) move_to_correct_pos(tt.target, tt.buddy, tt.free_dir, state);
        else move_to_correct_pos(tt.target, state);
    }
    void sort_last_2x2(State& state, int max_select_times) {
        if (max_select_times < 1) EXIT_DEBUG("");
        constexpr array<int, 4> offset{16,64,256, 1024};
        Pos p;
        int selected_idx;
        if (p = get_moved_toraly_pos(state.selected_pos, Pos(-1,-1)); state.state[p.y][p.x] == state.ordinary) selected_idx = 2;
        else if (p = get_moved_toraly_pos(state.selected_pos, Pos( 1,-1)); state.state[p.y][p.x] == state.ordinary) {
            p = get_moved_toraly_pos(state.selected_pos, Pos(0,-1));
            selected_idx = 3;
        }
        else if (p = get_moved_toraly_pos(state.selected_pos, Pos(-1, 1)); state.state[p.y][p.x] == state.ordinary) {
            p = get_moved_toraly_pos(state.selected_pos, Pos(-1,0));
            selected_idx = 1;
        }
        else if (p = get_moved_toraly_pos(state.selected_pos, Pos( 1, 1)); state.state[p.y][p.x] == state.ordinary) {
            p = get_moved_toraly_pos(state.selected_pos, Pos(0,0));
            selected_idx = 0;
        }
        else EXIT_DEBUG("");

        vector<int> order;
        {
            vector<Pos> tmp;
            Direction dir = Direction::R;
            rep(i,4) {
                tmp.push_back(p);
                move_toraly(p, dir);
                dir.rotate_cw();
            }
            swap(tmp[2], tmp[3]);
            rep(i,4) {
                rep(j,4) if (state.now_orig_pos[p.y][p.x] == tmp[j]) order.push_back(j);
                move_toraly(p, dir);
                dir.rotate_cw();
            }
        }

        int idx = min(3, max_select_times) + 4*selected_idx;
        rep(i,4) idx += order[i]*offset[i];

        auto& procs = best_procs_for_2x2[idx];
        if (procs.empty()) EXIT_DEBUG("");

        {
            Path path = procs.front().path;
            move_selected_pos(path, state);
            join_path(state.proc.back().path, path);
        }

        repr(i,1,procs.size()) {
            Path path = procs[i].path;
            state.selected_pos = get_moved_toraly_pos(p, procs[i].selected_pos);
            state.proc.emplace_back(state.selected_pos, Path());
            move_selected_pos(path, state);
            join_path(state.proc.back().path, path);
        }
        {
            Direction dir = Direction::R;
            rep(i,4) {
                state.state[p.y][p.x] = state.sorted;
                move_toraly(p, dir);
                dir.rotate_cw();
            }
        }
    }

public:
    [[nodiscard]] tuple<double, size_t> evaluate(const State& state) const {
        // todo: ちゃんと書く, A*探索みたいに期待値を加える感じで

        size_t hash_val = 0;
        rep(i,div_num.x) rep(j,div_num.y) hash_val ^= state.now_orig_pos[i][j].to_idx(div_num.x) + 0x9e3779b9 + (hash_val << 6) + (hash_val >> 2);

        int cost = state.proc.size() * select_cost;
        for(const auto& proc : state.proc) cost += proc.path.size() * swap_cost;
        return { cost, hash_val };
    }

    StrictSorter(Vec2<int> div_num, int selectable_times, int select_cost, int swap_cost)
        : RoughSorter(div_num, selectable_times, select_cost, swap_cost) {
        calc_best_procs_for_2x2();
    }

    void sort_start2finish(const Pos& first_selected_pos, const Pos& first_target, State& state) {

        if (state.selected_pos != first_selected_pos) {
            state.selected_pos = first_selected_pos;
            state.proc.emplace_back(first_selected_pos, Path());
        }

        state.first_sorted_pos = state.now_orig_pos[first_target.y][first_target.x];
        move_to_correct_pos(first_target, state);

        while(true) {
            auto transition_types = get_all_possible_transition_types(state);
            if (transition_types.empty()) break;
            double min_cost = 1e20;
            State best_state;
            for(const auto& transition_type : transition_types) {
                State tmp_state = state;
                transition(transition_type, tmp_state);
                double cost = get<0>(evaluate(tmp_state));
                if (min_cost > cost) {
                    min_cost = cost;
                    best_state = tmp_state;
                }
            }

            state = best_state;

        }

        sort_last_2x2(state, 3);
    }

    void sort_partially(const Pos& first_selected_pos, const Pos& UL, const Pos& dp, State& state) {

        State orig_state = state;

        // UL~UL+dpの範囲内が元座標の奴らが含まれるような最小の四角形を探索しそれ以外をsortedで埋める
        Pos exUL(0,0), exdp(div_num);
        array<array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> rev_now_orig_pos;
        {
            array<bool, MAX_DIV_NUM_Y> exists_within_range_y{};
            array<bool, MAX_DIV_NUM_X> exists_within_range_x{};

            rep(i,div_num.y) rep(j,div_num.x) {
                const Pos p = state.now_orig_pos[i][j];
                rev_now_orig_pos[p.y][p.x] = Vec2(j,i);
            }

            rep(dy, dp.y) rep(dx, dp.x) {
                const Pos tmp_dp(dx, dy);
                const Pos p = get_moved_toraly_pos(UL, tmp_dp);
                const Pos pos = rev_now_orig_pos[p.y][p.x];
                exists_within_range_x[p.x] = true;
                exists_within_range_y[p.y] = true;
                exists_within_range_x[pos.x] = true;
                exists_within_range_y[pos.y] = true;
            }
            rep(i,div_num.x) if (!exists_within_range_x[i]) {
                rep(space,div_num.x) {
                    const int j = (i+space+1) % div_num.x;
                    if (exists_within_range_x[j]) {
                        if (div_num.x-exdp.x < space+1) {
                            exUL.x = j;
                            exdp.x = div_num.x-(space+1);
                        }
                        break;
                    }
                }
            }
            rep(i,div_num.y) if (!exists_within_range_y[i]) {
                rep(space,div_num.y) {
                    const int j = (i+space+1) % div_num.y;
                    if (exists_within_range_y[j]) {
                        if (div_num.y-exdp.y < space+1) {
                            exUL.y = j;
                            exdp.y = div_num.y-(space+1);
                        }
                        break;
                    }
                }
            }
            rep(i,div_num.y) rep(j,div_num.x) state.state[i][j] = state.sorted;
            rep(dy, exdp.y) rep(dx, exdp.x) {
                const Pos tmp_dp(dx, dy);
                const Pos p = get_moved_toraly_pos(exUL, tmp_dp);
                state.state[p.y][p.x] = state.ordinary;
            }
        }

        // state.now_orig_posを操作しやすいように改変する
        // 具体的にはUL~UL+dpの範囲内にあるorig_posが全てUL~UL+dpの座標に収まるようにする
        vector<pair<Pos, Pos>> replace, replace_log;
        {
            array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> within_range{};
            array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> targeted{};
            rep(dy, exdp.y) rep(dx, exdp.x) {
                const Pos tmp_dp(dx, dy);
                const Pos pos = get_moved_toraly_pos(exUL, tmp_dp);
                const Pos orig_pos = state.now_orig_pos[pos.y][pos.x];
                within_range[pos.y][pos.x] = true;
                targeted[orig_pos.y][orig_pos.x] = true;
            }
            vector<Pos> not_within_range, not_targeted;
            rep(dy, exdp.y) rep(dx, exdp.x) {
                const Pos tmp_dp(dx, dy);
                const Pos pos = get_moved_toraly_pos(exUL, tmp_dp);
                const Pos orig_pos = state.now_orig_pos[pos.y][pos.x];
                if (!targeted[pos.y][pos.x]) not_targeted.push_back(pos);
                if (!within_range[orig_pos.y][orig_pos.x]) not_within_range.push_back(pos);
            }

            vector<ComparableData<double, pair<Pos, Pos>>> v;
            for(const auto& p1 : not_within_range) {
                for(const auto& p2 : not_targeted) {
                    int mh_dist = calc_mh_dist_toraly(p1, p2);
                    v.emplace_back(mh_dist*mh_dist, make_pair(p1,p2));
                }
            }
            sort(all(v));
            for(const auto& x : v) {
                const auto& p1 = x.data.first;
                const auto& p2 = x.data.second;
                const auto orig_p1 = state.now_orig_pos[p1.y][p1.x];
                if (!within_range[orig_p1.y][orig_p1.x] && !targeted[p2.y][p2.x]) {
                    replace.emplace_back(p1, p2);
                    targeted[p2.y][p2.x] = true;
                    within_range[orig_p1.y][orig_p1.x] = true;
                }
            }
        }
        for(const auto& [p1, p2] : replace) {
            Pos p = rev_now_orig_pos[p2.y][p2.x];
            replace_log.emplace_back(p, state.now_orig_pos[p.y][p.x]);
            replace_log.emplace_back(p2, state.now_orig_pos[p1.y][p1.x]);
            state.now_orig_pos[p.y][p.x] = state.now_orig_pos[p1.y][p1.x];
            state.now_orig_pos[p1.y][p1.x] = p2;
        }

        if (state.selected_pos != first_selected_pos) {
            state.selected_pos = first_selected_pos;
            state.proc.emplace_back(first_selected_pos, Path());
        }
        state.first_sorted_pos = get_moved_toraly_pos(exUL, exdp);
        if (state.now_orig_pos[state.selected_pos.y][state.selected_pos.x].x == state.first_sorted_pos.x) move_toraly(state.first_sorted_pos, Direction::R);
        if (state.now_orig_pos[state.selected_pos.y][state.selected_pos.x].y == state.first_sorted_pos.y) move_toraly(state.first_sorted_pos, Direction::D);
        if (state.state[state.first_sorted_pos.y][state.first_sorted_pos.x] != state.sorted) move_to_correct_pos(rev_now_orig_pos[state.first_sorted_pos.y][state.first_sorted_pos.x], state);


        while(true) {

            auto transition_types = get_all_possible_transition_types(state);
            if (transition_types.empty()) break;

            // target == destinationのやつがあれば遷移
            {
                bool transitioned = false;
                for(const auto& transition_type : transition_types) {
                    if (!transition_type.has_buddy) {
                        const auto& target = transition_type.target;
                        const auto& destination = state.now_orig_pos[target.y][target.x];
                        if (target == destination) {
//                            transition(transition_type, state);
                            state.state[destination.y][destination.x] = state.sorted;
                            transitioned = true;
                            break;
                        }
                    }
                }
                if (transitioned) continue;
            }

            // costが最も小さい次のstateを探す
            double min_cost = 1e20;
            State best_state;
            for(const auto& transition_type : transition_types) {
                State tmp_state = state;
                transition(transition_type, tmp_state);
                double cost = get<0>(evaluate(tmp_state));
                if (min_cost > cost) {
                    min_cost = cost;
                    best_state = tmp_state;
                }
            }
            state = best_state;
        }

        sort_last_2x2(state, 2);

        // 変更したところを戻していく
        rep(i,div_num.y) rep(j,div_num.x) state.state[i][j] = orig_state.state[i][j];
        rep(dy, exdp.y) rep(dx, exdp.x) {
            const Pos tmp_dp(dx, dy);
            const Pos pos = get_moved_toraly_pos(exUL, tmp_dp);
            state.state[pos.y][pos.x] = state.sorted;
        }
        for(const auto& [p1, p2] : replace_log) {
            state.now_orig_pos[p1.y][p1.x] = p2;
            state.state[p1.y][p1.x] = state.ordinary;
        }
    }
};

void check_ans(OriginalPositions initial_orig_pos, const Procedures &procs, const Vec2<int> div_num) {
    auto& now_orig_pos = initial_orig_pos;
    for(const auto &proc : procs) {
        Pos current = proc.selected_pos;
        Pos tmp = now_orig_pos[current.y][current.x];
        for(const auto dir : proc.path) {
            Pos next = current;
            next.move(dir);

            next.x = (next.x + div_num.x) % div_num.x;
            next.y = (next.y + div_num.y) % div_num.y;

            now_orig_pos[current.y][current.x] = now_orig_pos[next.y][next.x];
            current = next;
        }
        now_orig_pos[current.y][current.x] = tmp;
    }
    rep(i,div_num.y) rep(j,div_num.x) if (Vec2(j,i) != now_orig_pos[i][j]) {
        EPRINT(i, j, Vec2(j,i), now_orig_pos[i][j]);
        exit(-1);
    }
}

Procedures KrSolver::operator()(const OriginalPositions& original_positions, const Settings& settings, int argc, char *argv[]) {
    const Vec2<int> div_num = static_cast<Vec2<int>>(settings.div_num);
    const int selectable_times = settings.selectable_times;
    const int select_cost = settings.choice_cost;
    const int swap_cost = settings.swap_cost;

    State state(original_positions);
    RoughSorter rough_sorter(div_num, selectable_times, select_cost, swap_cost);
    StrictSorter strict_sorter(div_num, selectable_times, select_cost, swap_cost);


//        rep(_, 100)
    //    rep(_, selectable_times-2)

    {
//        Pos p = rough_sorter.get_max_penalty_pos(state);
//        if (p.x < 0 || p.y < 0) break;
//        Pos p(idx%div_num.x, idx / div_num.x);
//        PRINT(p, cost[idx]);
//        Pos p = Pos(5,8);
        Pos p(8,8);
        state.proc.emplace_back(p, Path());
        state.selected_pos = p;
        rough_sorter.sort_roughly(state, 5000, 12);
    }


    state.dump(div_num);


    {
//        Pos offset(4,4);
        Pos rect_size(6,8);
        int min_cost = INT_MAX;
        State best_state;
        rep(offset_y, div_num.y) rep(offset_x, div_num.x) {
            State tmp_state = state;
            rep(i,(div_num.y + rect_size.y-1) / rect_size.y) rep(j, (div_num.x + rect_size.x-1) / rect_size.x) {
                Pos p((j*rect_size.x+offset_x)%div_num.x, (i*rect_size.y+offset_y)%div_num.y);
                strict_sorter.sort_partially(p, p, rect_size, tmp_state);
            }
            optimize_procedures(tmp_state.proc);
            int tmp_cost = get<0>(strict_sorter.evaluate(tmp_state));
            PRINT(min_cost, tmp_cost);
            if (min_cost > tmp_cost && tmp_state.proc.size() <= 10) {
                min_cost = tmp_cost;
                best_state = tmp_state;
            }
        }
        state = best_state;
    }

    Utility::print_wall();

    state.dump(div_num);
    PRINT(state.proc.size());
    for(const auto& proc : state.proc) cout << proc << endl;

    PRINT(sizeof(State));
    check_ans(original_positions, state.proc, div_num);
    DUMP();
    Utility::print_wall("finish");
    PRINT(state.proc.size());
    PRINT(calc_cost(state.proc, settings));
    return state.proc;
}
