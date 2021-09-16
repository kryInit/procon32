#include <settings.hpp>
#include <iostream>
#include <fstream>

using namespace std;

Settings::Settings(const string& path) : frag_size(), div_num(), selectable_times(), choice_cost(), swap_cost() {
    load(path);
}

void Settings::load(const string& path) {
    Vec2<int> pix_num;
    ifstream ifs(path);
    if (!ifs) {
        cerr << "load failed" << endl;
        exit(-1);
    }
    ifs >> div_num >> selectable_times >> choice_cost >> swap_cost >> pix_num;
    if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
        cerr << "h_pix / h_div != w_pix / w_div" << endl;
        dump();
        exit(-1);
    }
    frag_size = pix_num.y / div_num.y;
}
void Settings::dump() const {
    cout << "frag size           :" << frag_size        << endl;
    cout << "div_num             :" << div_num          << endl;
    cout << "selectable_times    :" << selectable_times << endl;
    cout << "choice cost         :" << choice_cost      << endl;
    cout << "swap cost           :" << swap_cost        << endl;
}