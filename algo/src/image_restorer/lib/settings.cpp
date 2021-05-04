#include "settings.hpp"
#include <iomanip>
#include <fstream>

using namespace std;

void Settings::load(const string& path) {
    Vec2<unsigned int> pix_num;
    ifstream ifs(path);
    ifs >> div_num.x >> div_num.y;
    ifs >> selectable_times >> choice_cost >> repl_cost;
    ifs >> pix_num.x >> pix_num.y;
    if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
        cerr << "h_pix / h_div != w_pix / w_div" << endl;
        exit(-1);
    }
    frag_size = pix_num.y / div_num.y;
}
void Settings::dump() {
    constexpr unsigned int w = 20;
    cout << left << setw(w) << "frag size" << ": " << frag_size << endl;
    cout << left << setw(w) << "div num"  << ": " << div_num << endl;
    cout << left << setw(w) << "selectable times"  << ": " << selectable_times << endl;
    cout << left << setw(w) << "choice cost"  << ": " << choice_cost << endl;
    cout << left << setw(w) << "replacement cost"  << ": " << repl_cost << endl;
}
