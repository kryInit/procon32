#include <fstream>
#include <omp.h>
#include <timing_device.hpp>
#include <settings.hpp>
#include <vec2.hpp>
#include <original_positions.hpp>
#include <procedures.hpp>
#include <solvers.hpp>

#include <utility.hpp>
#include <macro.hpp>

using namespace std;

void dump_openmp_info() {
    printf("使用可能な最大スレッド数：%d\n", omp_get_max_threads());

    vector<int> thread_nums(omp_get_max_threads());
#pragma omp parallel for default(none) shared(thread_nums)
    for(int i=0; i<omp_get_max_threads(); ++i) thread_nums[i] = omp_get_thread_num();
    for(int i=0; i<omp_get_max_threads(); ++i) cout << "hello world " << thread_nums[i] << endl;
}

int main(int argc, char *argv[]) {
    dump_openmp_info();

    TimingDevice td;

    const string prob_dir = string(argv[1]);
    const string settings_path = prob_dir + "/prob.txt";
    const string original_state_path = prob_dir + "/original_state.txt";

    Settings settings(settings_path);
    settings.dump();
    td.print_elapsed();

    auto [rotations, original_positions] = input_original_state(original_state_path, settings);

//    Procedures procs = KrSolver()(original_positions, settings, argc, argv);
    Procedures procs = RkSolver()(original_positions, settings, argc, argv);
//    Procedures procs = SimpleSolver()(original_positions, settings);

    const string procedure_path = prob_dir + "/procedure.txt";
    ofstream ofs(procedure_path);
    ofs << rotations << endl << procs << endl;
    cout << rotations << endl << procs << endl;
    td.print_elapsed();
}