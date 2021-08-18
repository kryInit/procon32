#include <fstream>
#include <omp.h>
#include <timing_device.hpp>
#include <settings.hpp>
#include <image.hpp>
#include <answer.hpp>
#include <adjacency_evaluators.hpp>
#include <solvers.hpp>

using namespace std;

template<class ADJACENCY_EVALUATOR, class SOLVER>
Answer solve(const Image& img, const Settings& settings) {
    // img : RGB[DIV_NUM.y][DIV_NUM.x][FRAG_SIZE][FRAG_SIZE]
    // adjacency : double[DIV_NUM.y][DIV_NUM.x][4][DIV_NUM.y][DIV_NUM.x][4]
    // ans : ImageFragmentState[DIV_NUM.y][DIV_NUM.x]

    TimingDevice td;

    const unsigned int N = settings.DIV_NUM().x*settings.DIV_NUM().y*4;

    td.restart();
    auto *adjacency = new double[N*N];
    ADJACENCY_EVALUATOR()(img, settings, adjacency);
    td.print_elapsed();

    td.restart();
    Answer ans = SOLVER()(adjacency, settings);
    td.print_elapsed();

    delete[] adjacency;
    return ans;
}

void dump_openmp_info() {
    printf("使用可能な最大スレッド数：%d\n", omp_get_max_threads());

    vector<int> thread_nums(omp_get_max_threads());
#pragma omp parallel for default(none) shared(thread_nums)
    for(int i=0; i<omp_get_max_threads(); ++i) thread_nums[i] = omp_get_thread_num();
    for(int i=0; i<omp_get_max_threads(); ++i) cout << "hello world " << thread_nums[i] << endl;
}

int main(int argc, char *argv[]) {
    //    dump_openmp_info();

    string frag_dir_path = string(argv[1]) + "/frags/";
    string settings_path = string(argv[1]) + "/prob.txt";

    Settings settings;
    settings.load(settings_path);
    settings.dump();

    Image img;
    img.load(frag_dir_path, settings);
//    img.dump();
    cout << "finish load" << endl;

    Answer ans = solve<OnePixelAdjacencyEvaluator, SideBeamSearchSolver>(img, settings);
    string ans_path = string(argv[1]) + "/original_state.txt";
    std::ofstream ofs(ans_path);
    ans.dump();
    ans.dump(ofs);
}