#pragma once
#include <array>
#include <pos.hpp>
#include <constant.hpp>
#include <procedures.hpp>
#include <original_positions.hpp>


struct State {
    static constexpr char ordinary = 0, sorted = 1, unmovable = 2;

    Pos selected_pos, first_sorted_pos;
    Procedures proc;
    std::array<std::array<char, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> board_state;
    std::array<std::array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> orig_pos;


    State();
    explicit State(const OriginalPositions& _orig_pos);
    void select(const Pos& p);
    void move_selected_pos(const Path& path);
    void revert_move_selected_pos(int prev_path_size);

    [[nodiscard]] Pos get_now_pos_by_original_pos(Pos p) const;

    void dump() const;
};

