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
