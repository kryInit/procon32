#include <original_positions.hpp>
#include <fstream>
#include <utility.hpp>

std::tuple<std::string, OriginalPositions> input_original_state(const std::string& path, const Settings& settings) {
    const auto div_num = settings.DIV_NUM();
    std::string rotations;
    OriginalPositions original_positions = Utility::gen_vec(Pos(), div_num.y, div_num.x);
    std::ifstream ifs(path);
    ifs >> rotations;
    rep(i, div_num.y) rep(j, div_num.x) {
        std::string s;
        ifs >> s;
        int idx = stoi(s, nullptr, 16);
        int x = idx / 16, y = idx % 16;
        original_positions[y][x] = Vec2(j,i);
    }
    return { rotations, original_positions };
}
