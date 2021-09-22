#include "rk_solver.hpp"
#include <comparable_data.hpp>
#include <time_manager.hpp>
#include <weak_vector.hpp>
#include <utility.hpp>
#include <macro.hpp>
#include <constant.hpp>
#include <state.hpp>
#include <rough_sorter.hpp>
#include <strict_sorter.hpp>
#include <omp.h>

using namespace std;

Procedures get_initial_procedure(const State& initial_state, const Settings& settings) {
    State state = initial_state;
//    state.select(Pos(5,5));
    state.select(Pos(0,0));
    RoughSorter::parallelized_sort_roughly(state, 5000, 12);
    optimize_procedures(state.proc);

    return state.proc;
}

optional<pair<int, Procedures>> get_complete_procedure(const State& initial_state, const Settings& settings, const Vec2<int> rect_size, const Vec2<int> offset) {
    State state = initial_state;

    TimingDevice td;

    const int h = (div_num.y + rect_size.y-1) / rect_size.y, w = (div_num.x + rect_size.x-1) / rect_size.x;
    bool failed = false;
    rep(i,h) rep(j,w) {
        if (failed) break;
        int min_cost = INT_MAX;
        bool updated = false;
        State best_state;
        Pos p((j*rect_size.x+offset.x)%div_num.x, (i*rect_size.y+offset.y)%div_num.y);
        rep(dy, rect_size.y) rep(dx,rect_size.x) {
            State tmp_state = state;
            Pos sp((p.x+dx)%div_num.x,(p.y+dy)%div_num.y);
            StrictSorter::sort_partially(sp, p, rect_size, tmp_state);
            optimize_procedures(tmp_state.proc);

            int tmp_cost = StrictSorter::calc_cost(tmp_state);
            if (min_cost > tmp_cost && (int)tmp_state.proc.size() <= selectable_times) {
                min_cost = tmp_cost;
                best_state = tmp_state;
                updated = true;
            }
        }
        if (updated) state = best_state;
        else { failed = true; break; }
    }
    if (failed) return nullopt;
    int cost = StrictSorter::calc_cost(state);
    return make_pair(cost, state.proc);
}

void simulate(const State& initial_state, const Settings& settings) {
    TimingDevice td;
    int N = 5*5*16*16;
#pragma omp parallel for
    for(int idx=0; idx<N; ++idx) {
        State state = initial_state;
        int i = idx / (5*16*16) + 2;
        int j = (idx % (5*16*16)) / (16*16) + 2;
        int I = (idx % (16*16)) / 16;
        int J = idx % (16);
//    }
//    repr(i,2,7) repr(j,2,7) rep(I,16) rep(J,16) {
        Vec2<int> rect_size(j,i);
        Vec2<int> offset(I,J);
//        PRINT(rect_size, offset);
        get_complete_procedure(state, settings, rect_size, offset);
//        td.print_elapsed();
    }
    td.print_elapsed();
}

/*

Procedures get_complete_procedure(const State& initial_state, const Settings& settings) {
    State state = initial_state;


    TimingDevice td;


    {
        //        Pos offset(4,4);
        //        Pos rect_size(16,8);
        //        Pos rect_size(16,16);
        State ini_state = state;
        array<array<int, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> v;
        array<array<State, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> memo{};

        rep(i,div_num.y) rep(j,div_num.x) v[i][j] = INT_MAX;

        optional<int> min_rect_size_sum = nullopt;
        set<Pos> dns;
        repr(I,4,div_num.x + div_num.y + 1) repr(J,2,div_num.x+1) if (I > J) {
            Pos rect_size(J, I-J), dn((div_num.x+J-2) / J, (div_num.y+I-J-2)/(I-J));
            if (rect_size.y < 2 || rect_size.y > div_num.y || rect_size.x > div_num.x) continue;
            if (max(I-J,J) > min(I-J,J)*2) continue;
            //            if (abs((div_num.y + rect_size.y-1) / rect_size.y - (div_num.x + rect_size.x-1) / rect_size.x) > 2) continue;
            if (min_rect_size_sum &&  I > min_rect_size_sum.value()+3) continue;
            if (dns.count(dn)) continue;
            dns.insert(dn);
            PRINT(rect_size.x, rect_size.y);
            td.print_elapsed();

//#pragma omp parallel for
            for(int offset = 0; offset < div_num.x*div_num.y; ++offset)
                //        rep(offset, rect_size.y*rect_size.x)
                //        rep(offset, div_num.x*div_num.y)
                //        rep(offset_y, div_num.y) rep(offset_x, div_num.x)
                //        int offset_y = 1, offset_x = 1;
                //        int offset_y = 1, offset_x = 5;
            {
//                PRINT(offset);

                int offset_y = offset / div_num.x;
                int offset_x = offset % div_num.x;

                State tmp_state = ini_state;
                const int h = (div_num.y + rect_size.y-1) / rect_size.y, w = (div_num.x + rect_size.x-1) / rect_size.x;
                bool failed = false;
                rep(i,h) rep(j,w) {
                    if (failed) break;
                    int min_cost = INT_MAX;
                    bool updated = false;
                    State best_state;
                    Pos p((j*rect_size.x+offset_x)%div_num.x, (i*rect_size.y+offset_y)%div_num.y);
                    rep(dy, rect_size.y) rep(dx,rect_size.x) {
                        State tmp_tmp_state = tmp_state;
                        Pos sp((p.x+dx)%div_num.x,(p.y+dy)%div_num.y);
                        StrictSorter::sort_partially(sp, p, rect_size, tmp_tmp_state);
                        optimize_procedures(tmp_tmp_state.proc);

                        int tmp_cost = StrictSorter::calc_cost(tmp_tmp_state);
                        if (min_cost > tmp_cost && tmp_tmp_state.proc.size() <= 10) {
                            min_cost = tmp_cost;
                            best_state = tmp_tmp_state;
                            updated = true;
                        }
                    }
                    if (updated) tmp_state = best_state;
                    else { failed = true; break; }
                }
                int tmp_cost = StrictSorter::calc_cost(tmp_state);
                if (failed) tmp_cost = 99999;
                if (v[offset_y][offset_x] > tmp_cost) {
                    if (tmp_cost == 36110) PRINT(offset_y, offset_x, rect_size);
                    v[offset_y][offset_x] = tmp_cost;
                    memo[offset_y][offset_x] = tmp_state;
                }
            }
            if (!min_rect_size_sum) {
                rep(i,div_num.y) rep(j,div_num.x) {
                    if (v[i][j] != 99999) {
                        min_rect_size_sum = I;
                        PRINT("FIND");
                    }
                }
            }
        }

        double min_cost = 1e20;
        rep(i,div_num.y) rep(j,div_num.x) {
            if (min_cost > v[i][j]) {
                min_cost = v[i][j];
                state = memo[i][j];
            }
        }

        //        state = best_state;
        rep(i,div_num.y) {
            rep(j,div_num.x) cout << setw(6) << v[i][j];
            cout << endl;
        }
        cout << endl;

        PRINT(min_cost);

    }

    td.print_elapsed();


    return state.proc;
}

*/

void check_ans(OriginalPositions initial_orig_pos, const Procedures &procs) {
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

Procedures RkSolver::operator()(const OriginalPositions& original_positions, const Settings& settings, int argc, char *argv[]) {
    if (argc <= 2) EXIT_DEBUG("required 3 or more arguments");

    div_num = settings.div_num;
    swap_cost = settings.swap_cost;
    select_cost = settings.choice_cost;
    selectable_times = settings.selectable_times;

    StrictSorter::init();

    const State initial_state(original_positions);
    Procedures ans;

    if (string(argv[2]) == "initial") {
        ans = get_initial_procedure(initial_state, settings);
    } else if (string(argv[2]) == "complete") {
//        auto procs = get_initial_procedure(initial_state, settings);
        State state = initial_state;
        const string initial_procs_path = string(argv[3]);
        Procedures procs = input_procedure(initial_procs_path);
        for(const auto& proc : procs) {
            state.select(proc.selected_pos);
            state.move_selected_pos(proc.path);
        }

//        ans = get_complete_procedure(state, settings);

//        simulate(state, settings);
//        exit(-1);

        const Vec2<int> rect_size(atoi(argv[4]), atoi(argv[5]));
        const Vec2<int> offset(atoi(argv[6]), atoi(argv[7]));

        auto op_ans = get_complete_procedure(state, settings, rect_size, offset);
        if (op_ans) {
            ans = op_ans.value().second;
            check_ans(original_positions, ans);
            DUMP(op_ans.value().first);
        } else {
            DUMP(-1);
        }
    } else {
        EXIT_DEBUG("2nd argument must be initial or complete");
    }
    return ans;
}
