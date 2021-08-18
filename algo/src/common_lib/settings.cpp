#include <settings.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <utility.hpp>

using namespace std;

void Settings::load(const string& path) {
    Vec2<unsigned int> pix_num;
    ifstream ifs(path);
    Utility::input_from(ifs, div_num, selectable_times, choice_cost, swap_cost, pix_num);
    if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
        cerr << "h_pix / h_div != w_pix / w_div" << endl;
        exit(-1);
    }
    frag_size = pix_num.y / div_num.y;
}
void Settings::dump() const {
    DUMP("frag size           :", frag_size);
    DUMP("div_num             :", div_num);
    DUMP("selectable_times    :", selectable_times);
    DUMP("choice cost         :", choice_cost);
    DUMP("swap cost           :", swap_cost);
}

unsigned int Settings::FRAG_SIZE() const { return frag_size; }
Vec2<unsigned int> Settings::DIV_NUM() const { return div_num; }
unsigned int Settings::SELECTABLE_TIMES() const { return selectable_times; }
unsigned int Settings::CHOICE_COST() const { return choice_cost; }
unsigned int Settings::SWAP_COST() const { return swap_cost; }
