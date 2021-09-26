// build_procedure [simply, initial, complete] [cout, path] [cost, procedure, both] args...

#include <comparable_data.hpp>
#include <timing_device.hpp>
#include <weak_vector.hpp>
#include <utility.hpp>
#include <macro.hpp>
#include <vec2.hpp>

#include <settings.hpp>
#include <procedures.hpp>
#include <original_positions.hpp>

#include <state.hpp>
#include <solvers.hpp>
#include <constant.hpp>
#include <rough_sorter.hpp>
#include <strict_sorter.hpp>

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

int main(int argc, char *argv[]) {
    const string prob_dir = string(argv[1]);
    const string settings_path = prob_dir + "/prob.txt";
    const string original_state_path = prob_dir + "/original_state.txt";

    Settings settings(settings_path);

    auto [rotations, original_positions] = input_original_state(original_state_path, settings);

    const string mode = string(argv[2]);
    const string out_dest = string(argv[3]);
    const string out_kind = string(argv[4]);

    if (!(mode == "simply" || mode == "initial" || mode == "complete")) EXIT_DEBUG("mode: ", mode);
    if (!(out_kind == "cout" || out_kind == "procedure" || out_kind == "both")) EXIT_DEBUG("out_kind: ", out_kind);

    div_num = settings.div_num;
    swap_cost = settings.swap_cost;
    select_cost = settings.choice_cost;
    selectable_times = settings.selectable_times;

    StrictSorter::init();

    Procedures ans;


    if (mode == "simply") {
        ans = SimpleSolver()(original_positions, settings, argc, argv);
    } else if (mode == "initial") {
        // program /Users/rk/Projects/procon32/.tmp_data/tmp initial cout new 1 1 10000 120000 16
        State state(original_positions);
        if (string(argv[5]) != "new") {
            const string initial_procs_path = string(argv[5]);
            Procedures procs = input_procedure(initial_procs_path);
            for(const auto& proc : procs) {
                state.select(proc.selected_pos);
                state.move_selected_pos(proc.path);
            }
        }

        const Pos p(atoi(argv[6]), atoi(argv[7]));
        if (p.in_area(div_num)) state.select(p);
        else if (string(argv[5]) == "new") return -1;
        const int loose_time_limit = atoi(argv[8]);
        const int strict_time_limit = atoi(argv[9]);
        const int depth = atoi(argv[10]);

        RoughSorter::parallelized_sort_roughly(state, loose_time_limit, strict_time_limit, depth);
        ans = state.proc;
    } else /* if (mode == "complete") */ {
        State state(original_positions);
        if (string(argv[5]) != "new") {
            const string initial_procs_path = string(argv[5]);
            Procedures procs = input_procedure(initial_procs_path);
            for(const auto& proc : procs) {
                state.select(proc.selected_pos);
                state.move_selected_pos(proc.path);
            }
        }

        if (argc == 5) {
            StrictSorter::sort_by_roughly_sort(state);
        } else if (Pos pos1(atoi(argv[6]), atoi(argv[7])), pos2(atoi(argv[8]), atoi(argv[9])); pos1.x == 16 && pos1.y == 16) {
            // (const Pos& first_selected_pos, const Pos& first_target, State& state)
            int min_cost = INT_MAX;
            State best_state;
            const Pos first_selected_pos(pos2);
            rep(i,div_num.y) rep(j,div_num.x) {
                const Pos first_target = Pos(j,i);
                const Pos p = state.orig_pos[first_selected_pos.y][first_selected_pos.x];
                const Pos q = state.orig_pos[first_target.y][first_target.x];
                if (p.x != q.x && p.y != q.y) {
                    State tmp_state = state;
                    StrictSorter::sort_start2finish(first_selected_pos, first_target, tmp_state);
                    int cost = StrictSorter::calc_cost(tmp_state);
                    if ((int)tmp_state.proc.size() <= selectable_times && min_cost > cost) {
                        min_cost = cost;
                        best_state = tmp_state;
                    }
                }
            }
            if (min_cost == INT_MAX) return -1;
            ans = best_state.proc;
        } else {
            const Pos rect_size(pos1);
            const Pos offset(pos2);

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
            if (failed) return -1;
            ans = state.proc;
        }
    }

    optimize_procedures(ans);
    if (out_kind == "cost" || out_kind == "both") {
        int cost = ans.size() * select_cost;
        for(const auto& proc : ans) cost += proc.path.size() * swap_cost;
        if (out_dest == "cout") cout << cost << endl;
        else {
            ofstream ofs(out_dest);
            ofs << cost << endl;
        }
    }
    if (out_kind == "procedure" || out_kind == "both") {
        if (out_dest == "cout") cout << rotations << endl << ans << endl;
        else {
            ofstream ofs(out_dest);
            ofs << rotations << endl << ans << endl;
        }
    }

    return 0;
}