#include <state.hpp>
#include <macro.hpp>
#include <utility.hpp>
#include <iomanip>

State::State()
    : selected_pos(Pos(-1,-1))
    , first_sorted_pos()
    , proc()
    , board_state()
    , orig_pos() {}

State::State(const OriginalPositions& _orig_pos)
    : selected_pos(Pos(-1,-1))
    , first_sorted_pos()
    , proc()
    , board_state()
    , orig_pos() {
    rep(i,div_num.y) rep(j,div_num.x) orig_pos[i][j] = _orig_pos[i][j];
}

void State::select(const Pos& p) {
    if (selected_pos != p) {
        proc.emplace_back(p, Path());
        selected_pos = p;
    }
}

void State::move_selected_pos(const Path& path) {
    for(const auto& dir : path) {
        const Pos now = selected_pos;
        const Pos next = now.get_moved_toraly_pos(dir, div_num);
        std::swap(orig_pos[now.y][now.x], orig_pos[next.y][next.x]);
        selected_pos = next;
    }
    proc.back().path.join(path);
}

void State::revert_move_selected_pos(int prev_path_size) {
    while((int)proc.back().path.size() != prev_path_size) {
        Direction dir = proc.back().path.back();
        const Pos now = selected_pos;
        const Pos prev = now.get_moved_toraly_pos(dir.get_dir_reversed(), div_num);
        std::swap(orig_pos[now.y][now.x], orig_pos[prev.y][prev.x]);
        selected_pos = prev;
        proc.back().path.pop_back();
    }
}

Pos State::get_now_pos_by_original_pos(Pos p) const {
    rep(i,div_num.y) rep(j,div_num.x) if (orig_pos[i][j] == p) return Vec2(j,i);
    EXIT_DEBUG("reached end");
    return Pos(-1,-1);
}

void State::dump() const {
    //        PRINT(selected_pos);
    //        DUMP("proc:");
    //        DUMP(proc);
    rep(i,div_num.y) {
        rep(j,div_num.x) {
            std::stringstream ss;
            ss << std::hex << std::uppercase << orig_pos[i][j].x << orig_pos[i][j].y << "'";
            if (board_state[i][j] != ordinary) ss << (int)board_state[i][j];
            else ss << " ";
            std::cout << ss.str() << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
