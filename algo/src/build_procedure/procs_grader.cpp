#include <comparable_data.hpp>
#include <utility.hpp>
#include <macro.hpp>

#include <settings.hpp>
#include <procedures.hpp>
#include <original_positions.hpp>

#include <state.hpp>
#include <constant.hpp>

using namespace std;

int main(int argc, char *argv[]) {
    const string prob_dir = string(argv[1]);
    const string settings_path = prob_dir + "/prob.txt";
    const string true_original_state_path = prob_dir + "/true_original_state.txt";

    Settings settings(settings_path);

    auto [true_rotations, true_original_positions] = input_original_state(true_original_state_path, settings);

    div_num = settings.div_num;
    swap_cost = settings.swap_cost;
    select_cost = settings.choice_cost;
    selectable_times = settings.selectable_times;

    const string initial_procs_path = string(argv[2]);
    ifstream ifs(initial_procs_path);
    string rotations;
    ifs >> rotations;
    Procedures procs = input_procedure(initial_procs_path);

    if (procs.size() > selectable_times) return -1;
    if (rotations.size() != true_rotations.size()) return -1;

    State state(true_original_positions);
    for(const auto& proc : procs) {
        state.select(proc.selected_pos);
        state.move_selected_pos(proc.path);
    }

    int pos_mismatch_cnt = 0, dir_mismatch_cnt = 0;

    rep(i,div_num.y) rep(j,div_num.x) if (state.orig_pos[i][j] != Pos(j,i)) pos_mismatch_cnt++;
    rep(i,rotations.size()) if (rotations[i] != true_rotations[i]) dir_mismatch_cnt++;

    cout << pos_mismatch_cnt << " " << dir_mismatch_cnt << endl;
}