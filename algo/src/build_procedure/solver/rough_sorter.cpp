#include <rough_sorter.hpp>

#include <omp.h>

#include <constant.hpp>
#include <macro.hpp>
#include <utility.hpp>
#include <time_manager.hpp>

using namespace std;

[[nodiscard]] double calc_penalty(const Pos& p, const State& state) {
    const int mh_dist = p.calc_mh_dist_toraly(state.orig_pos[p.y][p.x], div_num);
    return mh_dist*mh_dist;
}
double move_and_calc_diff_penalty(const Direction& dir, State& state) {
    const Pos now = state.selected_pos;
    const Pos next = now.get_moved_toraly_pos(dir, div_num);
    int diff_penalty = -calc_penalty(next, state);
    swap(state.orig_pos[now.y][now.x], state.orig_pos[next.y][next.x]);
    state.selected_pos = next;
    diff_penalty += calc_penalty(now, state);
    return diff_penalty;
}
double revert_move_and_calc_diff_penalty(const Direction& dir, State& state) {
    return move_and_calc_diff_penalty(dir.get_dir_reversed(), state);
}

int RoughSorter::calc_penalty_sum(const State& state) {
    int penalty_sum = 0;
    rep(i,div_num.y) rep(j,div_num.x) penalty_sum += calc_penalty(Pos(j,i), state);
    return penalty_sum;
}

optional<Pos> RoughSorter::get_max_penalty_pos(const State& state) {
    optional<Pos> p = nullopt;
    int max_penalty = 0;
    rep(i,div_num.y) rep(j,div_num.x) {
        int tmp_penalty = calc_penalty(Vec2(j,i), state);
        if (max_penalty < tmp_penalty) {
            max_penalty = tmp_penalty;
            p = Pos(j,i);
        }
    }
    return p;
}

double RoughSorter::parallelized_sort_roughly(State& state, const int loose_time_limit, const int strict_time_limit, const int depth) {
    State original_state = state;
    const int dnx = div_num.x;
    const int dny = div_num.y;
    double coef = 0.0;

    double now_penalty = 0;
    rep(i,dny) rep(j,dnx) if (Pos(j,i) != state.selected_pos) now_penalty += calc_penalty(Pos(j,i), state);
    //        PRINT(now_penalty);

    int loop_count = 0;
    int prev_update_time = -1;
    int prev_upgrade_time = -1;

    double best_penalty = now_penalty;

    TimeManager tm(strict_time_limit);
/*

    {
        int count = 0;
        rep(i, dny) {
            rep(j,dnx) {
                int penalty = static_cast<int>(calc_penalty(Vec2(j,i), state));
                if (penalty == 0) cout << "   .";
                else cout << setw(4) << penalty, count++;
            }
            cout << endl;
        }
        cout << endl;
    }

*/

    constexpr int ini_path_num = 12;
    array<double, ini_path_num> min_diff_penalties{};
    array<Path, ini_path_num> best_addnl_paths{}, initial_addnl_paths{};
    {
        constexpr Direction U = Direction::U, R = Direction::R, D = Direction::D, L = Direction::L;
        initial_addnl_paths = {{{{U,U}}, {{U,R}}, {{U,L}}, {{R,U}}, {{R,R}}, {{R,D}}, {{D,R}}, {{D,D}}, {{D,L}}, {{L,U}}, {{L,D}}, {{L,L}}}};
    }

    Path& now_path = state.proc.back().path;
    Path best_path = now_path;
    while(tm.in_time_limit()) {
        PRINT(loop_count, now_path.size(), now_penalty, best_penalty);
        loop_count++;

        // serach additional path

        rep(i,ini_path_num) min_diff_penalties[i] = 0, best_addnl_paths[i] = Path();

#pragma omp parallel for num_threads(12)
        for(int i=0; i<ini_path_num; ++i) {
            //            rep(i,threads_num) {
            State tmp_state = state;

            double min_diff_penalty = 0;
            Path best_addnl_path;

            double now_diff_penalty = 0;
            Path now_addnl_path = initial_addnl_paths[i];

            now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path[0], tmp_state);
            {
                double tmp_now_diff_penalty = now_diff_penalty + coef;
                if ((min_diff_penalty > tmp_now_diff_penalty) || (min_diff_penalty == tmp_now_diff_penalty && best_addnl_path.size() > 1)) {
                    min_diff_penalty = tmp_now_diff_penalty;
                    best_addnl_path = {now_addnl_path[0]};
                }
            }

            now_diff_penalty += move_and_calc_diff_penalty(now_addnl_path[1], tmp_state);

            {
                double tmp_now_diff_penalty = now_diff_penalty + coef * now_addnl_path.size();
                if ((min_diff_penalty > tmp_now_diff_penalty) || (min_diff_penalty == tmp_now_diff_penalty && best_addnl_path.size() > now_addnl_path.size())) {
                    min_diff_penalty = tmp_now_diff_penalty;
                    best_addnl_path = now_addnl_path;
                }
            }

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

        rep(i,ini_path_num) {
            if (min_diff_penalty > min_diff_penalties[i] || (min_diff_penalty == min_diff_penalties[i] && best_addnl_path.size() > best_addnl_paths[i].size())) {
                min_diff_penalty = min_diff_penalties[i];
                best_addnl_path = best_addnl_paths[i];
            }
        }


        if (min_diff_penalty < 0) {// if I found good additional path
            // add path and change
            state.move_selected_pos(best_addnl_path);
            now_penalty += min_diff_penalty;
            prev_update_time = loop_count;
            if ((best_penalty > now_penalty) || (best_penalty == now_penalty && best_path.size() > now_path.size())) {
                best_penalty = now_penalty;
                best_path = now_path;
                //                    PRINT(loop_count, best_penalty);
                prev_upgrade_time = loop_count;
            }
        } else if (!now_path.empty()) { // if I couldn't find additional path
            // kick
            if ((int)tm.get_ms() > loose_time_limit) break;
            else if (loop_count - prev_update_time > 300) {
                now_penalty = best_penalty;
                state = original_state;
                state.move_selected_pos(best_path);
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
    state.revert_move_selected_pos(0);
    state.move_selected_pos(best_path);

/*

    PRINT(loop_count);
    PRINT(prev_update_time);
    PRINT(prev_upgrade_time);
    PRINT(best_penalty);
    PRINT(best_path.size());
    PRINT(best_path);

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
            if (Pos(j,i) == state.orig_pos[i][j]) cout << ". ";
            else {
                for(const auto& dir : Direction::All) {
                    if (Pos(j,i).get_moved_toraly_pos(dir, div_num) == state.orig_pos[i][j]) {
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

*/
    return best_penalty;
}
double sort_roughly(State& state, const int loose_time_limit, const int strict_time_limit, const int depth) {
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

    TimeManager tm(strict_time_limit);

    Path& now_path = state.proc.back().path;
    Path best_path = now_path;
    while(tm.in_time_limit()) {
        PRINT(loop_count, now_path.size(), now_penalty, best_penalty);

        loop_count++;

        double min_diff_penalty = 1e20;
        Path best_addnl_path;

        double now_diff_penalty = 0;
        Path now_addnl_path; now_addnl_path.reserve(depth);
        while(true) {
            if ((int)now_addnl_path.size() == depth) {
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
            state.move_selected_pos(best_addnl_path);
            now_penalty += min_diff_penalty;
            prev_update_time = loop_count;
            if ((best_penalty > now_penalty) || (best_penalty == now_penalty && best_path.size() > now_path.size())) {
                best_penalty = now_penalty;
                best_path = now_path;
                prev_upgrade_time = loop_count;
            }
        } else if (!now_path.empty()) { // if I couldn't find additional path
            // kick
            if ((int)tm.get_ms() > loose_time_limit) break;
            else if (loop_count - prev_update_time > 300) {
                now_penalty = best_penalty;
                state = original_state;
                state.move_selected_pos(best_path);
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
    state.revert_move_selected_pos(0);
    state.move_selected_pos(best_path);


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
            if (Pos(j,i) == state.orig_pos[i][j]) cout << ". ";
            else {
                for(const auto& dir : Direction::All) {
                    if (Pos(j,i).get_moved_toraly_pos(dir, div_num) == state.orig_pos[i][j]) {
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

double RoughSorter::sort_roughly_greedily(State& state, const int depth) {
    State original_state = state;
    const int dnx = div_num.x;
    const int dny = div_num.y;
    double coef = 0.;

    double now_penalty = 0;
    rep(i,dny) rep(j,dnx) if (Pos(j,i) != state.selected_pos) now_penalty += calc_penalty(Pos(j,i), state);

    double best_penalty = now_penalty;
    Path best_path;

    Path& now_path = state.proc.back().path;
    while(true) {

        double min_diff_penalty = 1e20;
        Path best_addnl_path;

        double now_diff_penalty = 0;
        Path now_addnl_path; now_addnl_path.reserve(depth);
        while(true) {
            if ((int)now_addnl_path.size() == depth) {
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
            state.move_selected_pos(best_addnl_path);
            now_penalty += min_diff_penalty;
            if ((best_penalty > now_penalty) || (best_penalty == now_penalty && best_path.size() > now_path.size())) {
                best_penalty = now_penalty;
                best_path = now_path;
            }
        } else break;
    }

    state = original_state;
    state.move_selected_pos(best_path);

    return best_penalty;
}

