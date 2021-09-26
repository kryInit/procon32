#include <comparable_data.hpp>
#include <timing_device.hpp>
#include <utility.hpp>

#include <settings.hpp>
#include <procedures.hpp>
#include <original_positions.hpp>

#include <state.hpp>
#include <constant.hpp>
#include <rough_sorter.hpp>

using namespace std;

int main(int argc, char *argv[]) {
    const string prob_dir = string(argv[1]);
    const string settings_path = prob_dir + "/prob.txt";
    const string original_state_path = prob_dir + "/original_state.txt";

    Settings settings(settings_path);

    auto [rotations, original_positions] = input_original_state(original_state_path, settings);

    div_num = settings.div_num;
    swap_cost = settings.swap_cost;
    select_cost = settings.choice_cost;
    selectable_times = settings.selectable_times;

    const string initial_procs_path = string(argv[2]);
    Procedures procs = input_procedure(initial_procs_path);

    if (procs.size() > selectable_times) exit(-1);

    State state(original_positions);
    int cost = procs.size() * select_cost;
    for(const auto& proc : procs) {
        cost += proc.path.size() * swap_cost;
        state.select(proc.selected_pos);
        state.move_selected_pos(proc.path);
    }

    int penalty_sum = RoughSorter::calc_penalty_sum(state);

    cout << penalty_sum << " " << cost << endl;
}