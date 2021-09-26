#include <original_positions.hpp>
#include <fstream>
#include <macro.hpp>

std::tuple<std::string, OriginalPositions> input_original_state(const std::string& path, const Settings& settings) {
    const auto div_num = settings.div_num;
    std::string rotations;
    OriginalPositions original_positions(div_num.y, std::vector<Pos>(div_num.x));
    std::ifstream ifs(path);
    if (!ifs) {
        std::cerr << "[input original state] load failed" << std::endl;
        exit(-1);
    }
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

