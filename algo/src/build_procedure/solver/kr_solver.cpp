#include "kr_solver.hpp"
#include <bits/stdc++.h>
#include <utility>
#include <comparable_data.hpp>
#include <time_manager.hpp>
#include <limited_storage.hpp>
#include <utility.hpp>

using namespace std;


/*/
 *
 * 最初に盤面の偶奇性を判定
 * 必要最小限の手数を求める
 * Stateを適当に遷移させる
 * 遷移の種類は以下のふたつ
 * 適当な座標を選択し動かす
 * 条件を満たす座標を揃える
 *
 * 盤面の偶奇性は意味をなさない?
 * 長さが奇数の行/列の盤外移動で偶奇性が変わる
 *
 *
/*/

constexpr int MIN_DIV_NUM_Y = 2;
constexpr int MIN_DIV_NUM_X = 2;
constexpr int MIN_DIV_NUM   = MIN_DIV_NUM_Y * MIN_DIV_NUM_X;
constexpr int MAX_DIV_NUM_Y = 16;
constexpr int MAX_DIV_NUM_X = 16;
constexpr int MAX_DIV_NUM   = MAX_DIV_NUM_Y * MAX_DIV_NUM_X;

constexpr int MIN_SELECTABLE_TIMES = 2;
constexpr int MAX_SELECTABLE_TIMES = 128;

constexpr int MIN_SWAP_COST   = 1;
constexpr int MIN_SELECT_COST = 1;
constexpr int MAX_SWAP_COST   = 100;
constexpr int MAX_SELECT_COST = 500;


/* * * * * * * * * * * * * * * *
 * row   , UR: RUULDRDLUURDLDRU *
 * row   , DR: RDDLURULDDRULURD *
 * row   , DL: LDDRULURDDLURULD *
 * row   , UL: LUURDLDRUULDRDLU *
 * column, UR: URRDLULDRRULDLUR *
 * column, DR: DRRULDLURRDLULDR *
 * column, DL: DLLURDRULLDRURDL *
 * column, UL: ULLDRURDLLURDRUL *
  * * * * * * * * * * * * * * * */

const Path row_UR = { Direction::R, Direction::U, Direction::U, Direction::L, Direction::D, Direction::R, Direction::D, Direction::L, Direction::U, Direction::U, Direction::R, Direction::D, Direction::L, Direction::D, Direction::R, Direction::U };
const Path row_DR = Utility::get_mapped<Direction>(row_UR, [](const Direction& dir) { return dir.is_vertical()   ? dir.get_dir_reversed() : dir; });
const Path row_DL = Utility::get_mapped<Direction>(row_DR, [](const Direction& dir) { return dir.is_horizontal() ? dir.get_dir_reversed() : dir; });
const Path row_UL = Utility::get_mapped<Direction>(row_DL, [](const Direction& dir) { return dir.is_vertical()   ? dir.get_dir_reversed() : dir; });

const Path column_UR = Utility::get_mapped<Direction>(row_UR, [](const Direction& dir) { return dir.is_vertical() ? dir.get_dir_rotated_cw() : dir.get_dir_rotated_ccw(); });
const Path column_DR = Utility::get_mapped<Direction>(column_UR, [](const Direction& dir) { return dir.is_vertical()   ? dir.get_dir_reversed() : dir; });
const Path column_DL = Utility::get_mapped<Direction>(column_DR, [](const Direction& dir) { return dir.is_horizontal() ? dir.get_dir_reversed() : dir; });
const Path column_UL = Utility::get_mapped<Direction>(column_DL, [](const Direction& dir) { return dir.is_vertical()   ? dir.get_dir_reversed() : dir; });


void join_path(Path &p1, const Path& p2) { p1.insert(p1.end(), p2.begin(), p2.end()); }

struct Context {
    Vec2<int> div_num;
    int selectable_times;
    int select_cost, swap_cost;
    Context(Vec2<int> div_num, int selectable_times, int select_cost, int swap_cost)
        : div_num(div_num)
        , selectable_times(selectable_times)
        , select_cost(select_cost)
        , swap_cost(swap_cost) {}
};

void move_toraly(Pos& p, const Pos& dp, const Context& ctx);
void move_toraly(Pos& p, const Direction& dir, const Context& ctx);
Pos get_moved_toraly_pos(Pos p, const Pos& dp, const Context& ctx);
Pos get_moved_toraly_pos(Pos p, const Direction& dir, const Context& ctx);


/*/
 *
 * Stateの遷移
 * ・新しくSingleProcedureを生やす
 * ・適当な座標を揃える
 *
 *
 *
 *
/*/

class LightSingleProcedure {
    Pos selected_pos;
    string path;
public:
    explicit LightSingleProcedure(Pos selected_pos) :selected_pos(selected_pos), path() {}
    void join_path(const Path& additional_path) {
        path.reserve(path.size()+additional_path.size());
        for(const auto& dir : additional_path) {
            if (path.empty()) path.push_back(dir.to_char());
            else {
                if (path.back() == Direction(dir.get_dir_reversed()).to_char()) path.pop_back();
                else path.push_back(dir.to_char());
            }
        }
    }
    const Pos& get_selected_pos() const { return selected_pos; }
    const string& get_spath() const { return path; }
    Path get_path() const {
        Path tmp_path; tmp_path.reserve(path.size());
        for(const auto& c : path) {
            Direction dir;
            if (c == 'U') dir = Direction::U;
            else if (c == 'R') dir = Direction::R;
            else if (c == 'D') dir = Direction::D;
            else dir = Direction::L;
            tmp_path.push_back(dir);
        }
        return tmp_path;
    }
    size_t get_path_size() const { return path.size(); }
    void dump() const {
        PRINT(selected_pos);
        PRINT(path.size());
        PRINT(path);
    }
    void reverse(const Context& ctx) {
        Pos tmp_pos = selected_pos;
        for(auto& c : path) {
            Direction dir;
            if (c == 'U') dir = Direction::U, c = 'D';
            else if (c == 'R') dir = Direction::R, c = 'L';
            else if (c == 'D') dir = Direction::D, c = 'U';
            else dir = Direction::L, c = 'R';
            move_toraly(tmp_pos, dir, ctx);
        }
        selected_pos = tmp_pos;
        std::reverse(all(path));
    }
    SingleProcedure convert_single_procedure() const {
        return SingleProcedure(selected_pos, get_path());
    }
};
using LightProcedures = vector<LightSingleProcedure>;

class State {
    static constexpr char ordinary = 0, sorted = 1, unmovable = 2;

    int intentionally_sorted_count;
    LightProcedures procedures;
    Pos moving_pos, first_sorted_pos;
    array<unsigned char, MAX_DIV_NUM> state; // 0: ordinary, 1: sorted, 2: unmovable
    array<unsigned char, MAX_DIV_NUM> now_orig_pos;

    [[nodiscard]] bool is_completable_if_cannot_move_toraly(const Context& ctx) {
        const auto& div_num = ctx.div_num;
        const int n = div_num.x * div_num.y;
        array<int, MAX_DIV_NUM> now_order, rev_now_order;
        rep(i,n) {
            Index orig_idx = now_orig_pos[i];
            now_order[i] = orig_idx;
            rev_now_order[orig_idx] = i;
        }
        int swap_count = 0;
        rep(i,n) {
            if (now_order[i] != i) {
                Index target_idx = rev_now_order[i];
                swap(now_order[i], now_order[target_idx]);
                rev_now_order[i] = i;
                rev_now_order[now_order[target_idx]] = target_idx;
                swap_count++;
            }
        }

        Pos dp = moving_pos - Pos::idx2pos(now_orig_pos[moving_pos.pos2idx(div_num.x)], div_num.x);
        int dist = abs(dp.x) + abs(dp.y);
        PRINT(dist);
        PRINT(swap_count);
        return ((swap_count+dist)%2) == 0;
    }

    [[nodiscard]] unsigned char get_now_pos_by_original_pos_uc(unsigned char orig_pos, const Context& ctx) const {
        rep(i,MAX_DIV_NUM) if (now_orig_pos[i] == orig_pos) return i;

    }
    [[nodiscard]] Pos get_now_pos_by_original_pos(OriginalPos orig_pos, const Context& ctx) const {
        return Pos::idx2pos(get_now_pos_by_original_pos_uc(orig_pos.pos2idx(ctx.div_num.x), ctx), ctx.div_num.x);
    }

    void move_current_selected_pos(const Direction& dir, const Context& ctx) {
        Pos next = get_moved_toraly_pos(moving_pos, dir, ctx);
        Index now_idx = moving_pos.pos2idx(ctx.div_num.x);
        Index next_idx = next.pos2idx(ctx.div_num.x);
        swap(now_orig_pos[now_idx], now_orig_pos[next_idx]);
        moving_pos = next;
    }
    void move_current_selected_pos(const Path& path, const Context& ctx) {
        for(const auto& dir : path) move_current_selected_pos(dir, ctx);
    }

    [[nodiscard]] optional<Path> calc_shortest_path(const Pos& s, const Pos& t, const Context& ctx) const {
        const int div_num_x = ctx.div_num.x;
        deque<pair<Pos, Path>> que;
        array<bool, MAX_DIV_NUM> visited{};

        if (state[s.pos2idx(div_num_x)] != ordinary) {
            auto message = Utility::concat("[calc_shortest_path]: state[s_idx] is ", state[s.pos2idx(div_num_x)]);
            Utility::exit_with_message(message);
        }
        if (state[t.pos2idx(div_num_x)] != ordinary) {
            auto message = Utility::concat("[calc_shortest_path]: state[t_idx] is ", state[t.pos2idx(div_num_x)]);
            Utility::exit_with_message(message);
        }

        que.emplace_back(s, Path());
        while(!que.empty()) {
            Path path = que.front().second;
            const Pos now = que.front().first;
            const Index idx = now.pos2idx(div_num_x);
            que.pop_front();
            if (now == t) return path;
            if (visited[idx]) continue;
            visited[idx] = true;
            for(const auto& dir : Utility::get_neighborhood4_dir()) {
                Pos next = get_moved_toraly_pos(now, dir, ctx);
                if (state[next.pos2idx(div_num_x)] == ordinary) {
                    path.push_back(dir);
                    que.emplace_back(next, path);
                    path.pop_back();
                }
            }
        }
        return nullopt;
    }
    [[nodiscard]] optional<Path> calc_nice_path(const Pos& s, const Pos& t, const Context& ctx) const {
        const int div_num_x = ctx.div_num.x, div_num_y = ctx.div_num.y;
        static deque<pair<Pos, Path>> que;
        static array<bool, MAX_DIV_NUM> visited{};
        que.clear();
        rep(i,div_num_x*div_num_y) visited[i] = false;

        if (state[s.pos2idx(div_num_x)] != ordinary) {
            auto message = Utility::concat("[calc_nice_path]: state[s_idx] is ", state[s.pos2idx(div_num_x)]);
            Utility::exit_with_message(message);
        }
        if (state[t.pos2idx(div_num_x)] != ordinary) {
            auto message = Utility::concat("[calc_nice_path]: state[t_idx] is ", state[t.pos2idx(div_num_x)]);
            Utility::exit_with_message(message);
        }

        que.emplace_back(s, Path());
        while(!que.empty()) {
            Path path = que.front().second;
            const Pos now = que.front().first;
            const Index idx = now.pos2idx(div_num_x);
            que.pop_front();
            if (now == t) return path;
            if (visited[idx]) continue;
            visited[idx] = true;
            for(const auto& dir : Utility::get_neighborhood4_dir()) {
                Pos next = get_moved_toraly_pos(now, dir, ctx);
                Pos orig_next = Pos::idx2pos(now_orig_pos[next.pos2idx(div_num_x)], div_num_x);
                if (state[next.pos2idx(div_num_x)] == ordinary) {
                    path.push_back(dir);

                    bool back = 0;

                    {
                        int now_to_orig_next_dist = 0;
                        int next_to_orig_next_dist = 0;

                        Pos shifted_now = Pos((now.x-first_sorted_pos.x+div_num_x)%div_num_x, (now.y-first_sorted_pos.y+div_num_y)%div_num_y);
                        Pos shifted_next = Pos((next.x-first_sorted_pos.x+div_num_x)%div_num_x, (next.y-first_sorted_pos.y+div_num_y)%div_num_y);
                        Pos shifted_orig_next = Pos((orig_next.x-first_sorted_pos.x+div_num_x)%div_num_x, (orig_next.y-first_sorted_pos.y+div_num_y)%div_num_y);

/*
                        if (shifted_now.x == 0) now_to_orig_next_dist += min(shifted_orig_next.x, div_num_x-shifted_orig_next.x);
                        else now_to_orig_next_dist += abs(shifted_now.x - shifted_orig_next.x);
                        if (shifted_now.x == 0) now_to_orig_next_dist += min(shifted_orig_next.y, div_num_x-shifted_orig_next.y);
                        else now_to_orig_next_dist += abs(shifted_now.y - shifted_orig_next.y);

                        if (shifted_next.x == 0) next_to_orig_next_dist += min(shifted_orig_next.x, div_num_x-shifted_orig_next.x);
                        else next_to_orig_next_dist += abs(shifted_next.x - shifted_orig_next.x);
                        if (shifted_next.x == 0) next_to_orig_next_dist += min(shifted_orig_next.y, div_num_x-shifted_orig_next.y);
                        else next_to_orig_next_dist += abs(shifted_next.y - shifted_orig_next.y);
*/
                        now_to_orig_next_dist += abs(shifted_now.x - shifted_orig_next.x);
                        now_to_orig_next_dist += abs(shifted_now.y - shifted_orig_next.y);

                        next_to_orig_next_dist += abs(shifted_next.x - shifted_orig_next.x);
                        next_to_orig_next_dist += abs(shifted_next.y - shifted_orig_next.y);


                        back = now_to_orig_next_dist > next_to_orig_next_dist;
                    }

                    if (back) que.emplace_back(next, path);
                    else que.emplace_front(next, path);
                    path.pop_back();
                }
            }
        }
        return nullopt;
    }

    tuple<Path, bool> move_target_to_destination_by_selected_pos(const Pos& target, const Pos& destination, const Context& ctx) {
        const int div_num_x = ctx.div_num.x;
        if (state[target.pos2idx(div_num_x)] != ordinary) {
            Utility::exit_with_message("[move_target_to_destination_by_selected_pos]: target's state isn't ordinary");
        }
        Pos now = target;
        Path path;
//        PRINT(target, destination, moving_pos);
        Path targets_path = calc_nice_path(target, destination, ctx).value();
        for(const auto& dir : targets_path) {
            Pos next = get_moved_toraly_pos(now, dir, ctx);
            state[now.pos2idx(div_num_x)] = unmovable;
//            PRINT(now, next, moving_pos);
            optional<Path> optional_additional_path = calc_nice_path(moving_pos, next, ctx);
            state[now.pos2idx(div_num_x)] = ordinary;
            if (optional_additional_path) {
                Path& additional_path = optional_additional_path.value();
                additional_path.push_back(dir.get_dir_reversed());

                move_current_selected_pos(additional_path, ctx);
                join_path(path, additional_path);
                now = next;
            } else return {path, false};
        }
        return {path, true};
    }

    void move_to_correct_pos(const Pos& target, const Context& ctx) {
        const int div_num_x = ctx.div_num.x;
        Pos destination = Pos::idx2pos(now_orig_pos[target.pos2idx(div_num_x)], div_num_x);
        auto [path, succeeded] = move_target_to_destination_by_selected_pos(target, destination, ctx);
        if (succeeded) {
//            PRINT(path);
            procedures.back().join_path(path);
            intentionally_sorted_count++;
            state[destination.pos2idx(div_num_x)] = sorted;
        } else Utility::exit_with_message("[move_to_correct_pos]: ");
    }

    void move_to_correct_pos(const Pos& target, const Pos& first_buddy, Direction free_dir, bool should_adjust_parity, const Context& ctx) {
        const int div_num_x = ctx.div_num.x;
        const OriginalPos orig_target = Pos::idx2pos(now_orig_pos[target.pos2idx(div_num_x)], div_num_x);
        const OriginalPos orig_buddy = Pos::idx2pos(now_orig_pos[first_buddy.pos2idx(div_num_x)], div_num_x);
        if (target == orig_target && first_buddy == orig_buddy) {
            intentionally_sorted_count += 2;
            state[orig_target.pos2idx(div_num_x)] = sorted;
            state[orig_buddy.pos2idx(div_num_x)] = sorted;
            return;
        }
        const Pos destination1 = orig_buddy;
        const Pos destination2 = get_moved_toraly_pos(orig_buddy, free_dir, ctx);
        const Pos destination3 = orig_target;
        Direction target_to_buddy_dir = Direction::U;
        rep(_,4) {
            if (get_moved_toraly_pos(orig_target, target_to_buddy_dir, ctx) == orig_buddy) break;
            target_to_buddy_dir.rotate_cw();
            if (_ == 3) Utility::exit_with_message("[move_to_current_pos]: 4");
        }


        if (state[destination1.pos2idx(div_num_x)] != ordinary || state[destination2.pos2idx(div_num_x)] != ordinary) {
            Utility::exit_with_message("[move_to_current_pos]: 0");
        }

        Path path, tmp_path;
        bool succeeded;

//        PRINT(target, destination1);
        tie(tmp_path, succeeded) = move_target_to_destination_by_selected_pos(target, destination1, ctx);
        if (!succeeded) Utility::exit_with_message("[move_to_current_pos]: 1");
        join_path(path, tmp_path);
        state[destination1.pos2idx(div_num_x)] = unmovable;

//        PRINT(target, destination2);
        Pos now_buddy = get_now_pos_by_original_pos(orig_buddy, ctx);
        tie(tmp_path, succeeded) = move_target_to_destination_by_selected_pos(now_buddy, destination2, ctx);
        join_path(path, tmp_path);
        state[destination2.pos2idx(div_num_x)] = unmovable;
        if (!succeeded) {
            if (moving_pos != orig_target) Utility::exit_with_message(Utility::concat("[move_to_current_pos] moving_pos, orig_target: ",moving_pos,orig_target));
            if (target_to_buddy_dir.is_vertical() == free_dir.is_vertical()) Utility::exit_with_message("[move_to_current_pos] 2");
            if (target_to_buddy_dir == Direction::U) {
                if (free_dir == Direction::R) tmp_path = column_UR;
                else tmp_path = column_UL;
            } else if (target_to_buddy_dir == Direction::R) {
                if (free_dir == Direction::U) tmp_path = row_UR;
                else tmp_path = row_DR;
            } else if (target_to_buddy_dir == Direction::D) {
                if (free_dir == Direction::R) tmp_path = column_DR;
                else tmp_path = column_DL;
            } else {
                if (free_dir == Direction::U) tmp_path = row_UL;
                else tmp_path = row_DL;
            }
            state[destination1.pos2idx(div_num_x)] = ordinary;
            state[destination2.pos2idx(div_num_x)] = ordinary;

            move_current_selected_pos(tmp_path, ctx);
            join_path(path, tmp_path);

//            PRINT(path);
            procedures.back().join_path(path);
            intentionally_sorted_count += 2;
            state[orig_target.pos2idx(div_num_x)] = sorted;
            state[orig_buddy.pos2idx(div_num_x)] = sorted;
            return;
        }

        if (should_adjust_parity) { /* todo: 書く */ }

//        PRINT(target, destination3);
        auto opt_path = calc_nice_path(moving_pos, destination3, ctx);
        if (!opt_path) Utility::exit_with_message("[move_to_current_pos]: 3");
        move_current_selected_pos(opt_path.value(), ctx);
        join_path(path, opt_path.value());

//        PRINT("4");
        state[destination1.pos2idx(div_num_x)] = ordinary;
        state[destination2.pos2idx(div_num_x)] = ordinary;

        tmp_path = {target_to_buddy_dir, free_dir};
        move_current_selected_pos(tmp_path, ctx);
        join_path(path, tmp_path);

//        PRINT(path);
        procedures.back().join_path(path);
        intentionally_sorted_count += 2;
        state[orig_target.pos2idx(div_num_x)] = sorted;
        state[orig_buddy.pos2idx(div_num_x)] = sorted;
    }

public:
    State() = default;
    State(const OriginalPositions& original_positions, const Pos first_selected_pos, const Pos first_target_pos, const Context& ctx)
        : intentionally_sorted_count(), procedures(), moving_pos(first_selected_pos), first_sorted_pos(original_positions[first_target_pos.y][first_target_pos.x]), state(), now_orig_pos() {
        procedures.emplace_back(first_selected_pos);
        rep(i, ctx.div_num.y) rep(j, ctx.div_num.x) {
            const Index now_idx = Vec2(j,i).pos2idx(ctx.div_num.x);
            const Index orig_idx = original_positions[i][j].pos2idx(ctx.div_num.x);
            now_orig_pos[now_idx] = orig_idx;
        }

        move_to_correct_pos(first_target_pos, ctx);
    }

    struct TransitionType {
        Pos target;
        bool has_buddy;
        Pos buddy;
        Direction free_dir;
        bool should_adjust_parity;
        TransitionType(Pos target) : target(target), has_buddy(false), buddy(), free_dir(), should_adjust_parity(false) {}
        TransitionType(Pos target, Pos buddy, Direction free_dir) : target(target), has_buddy(true), buddy(buddy), free_dir(free_dir), should_adjust_parity(false) {}
        TransitionType(Pos target, Pos buddy, Direction free_dir, bool should_adjust_parity) : target(target), has_buddy(true), buddy(buddy), free_dir(free_dir), should_adjust_parity(should_adjust_parity) {}
    };

    [[nodiscard]] tuple<double, size_t> evaluate(const Context& ctx) const {
        // todo: ちゃんと書く, A*探索みたいに期待値を加える感じで

        const size_t n = ctx.div_num.x * ctx.div_num.y;
        size_t hash_val = n;
        rep(i,n) hash_val ^= now_orig_pos[i] + 0x9e3779b9 + (hash_val << 6) + (hash_val >> 2);

        int cost = procedures.size() * ctx.select_cost;
        for(const auto& proc : procedures) cost += proc.get_path_size() * ctx.swap_cost;
        return { cost, hash_val };
    }
    [[nodiscard]] const vector<TransitionType> get_all_possible_transition_types(const Context& ctx) const {
        vector<TransitionType> transition_types;

        constexpr unsigned char unsortable = 255;
        const int dnx = ctx.div_num.x, dny = ctx.div_num.y;
        const int n = dnx*dny;
        if (intentionally_sorted_count+4 == n) return {};
        static array<unsigned char, MAX_DIV_NUM> rev_now_orig_pos{};
        static array<unsigned char, MAX_DIV_NUM_X> vertical_sorted_count{};
        static array<unsigned char, MAX_DIV_NUM_Y> horizontal_sorted_count{};

        rep(i,n) rev_now_orig_pos[now_orig_pos[i]] = i;
        rep(i,dny) horizontal_sorted_count[i] = 0;
        rep(j,dnx) vertical_sorted_count[j] = 0;

        rep(i, dny) rep(j, dnx) if (state[Vec2(j,i).pos2idx(dnx)] == sorted) {
            vertical_sorted_count[j]++;
            horizontal_sorted_count[i]++;
        }

        int adjustable_parity_chance_num = (vertical_sorted_count[first_sorted_pos.x] != dny && dnx%2 == 1) +
                                           (horizontal_sorted_count[first_sorted_pos.y] != dnx && dny%2 == 1);

        rep(j,dnx) if (vertical_sorted_count[j] == dny-2) rep(i,dny) {
            const OriginalPos orig_p1 = Vec2(j,i);
            const OriginalPos orig_p2 = get_moved_toraly_pos(orig_p1,Direction::D,ctx);
            if (state[orig_p1.pos2idx(dnx)] == ordinary && state[orig_p2.pos2idx(dnx)] == ordinary) {
                const Pos p1 = Pos::idx2pos(rev_now_orig_pos[orig_p1.pos2idx(dnx)], dnx);
                const Pos p2 = Pos::idx2pos(rev_now_orig_pos[orig_p2.pos2idx(dnx)], dnx);
                if (p1 == moving_pos || p2 == moving_pos) continue;
                if (j == first_sorted_pos.x) {
                    if (adjustable_parity_chance_num == 1 && dnx%2 == 1) {
                        transition_types.emplace_back(p1, p2, Direction::L, true);
                        transition_types.emplace_back(p1, p2, Direction::R, true);
                        transition_types.emplace_back(p2, p1, Direction::L, true);
                        transition_types.emplace_back(p2, p1, Direction::R, true);
                    }
                    transition_types.emplace_back(p1, p2, Direction::L);
                    transition_types.emplace_back(p1, p2, Direction::R);
                    transition_types.emplace_back(p2, p1, Direction::L);
                    transition_types.emplace_back(p2, p1, Direction::R);
                } else if (vertical_sorted_count[(j+1)%dnx] == dny) {
                    transition_types.emplace_back(p1, p2, Direction::L);
                    transition_types.emplace_back(p2, p1, Direction::L);
                } else if (vertical_sorted_count[(j+dnx-1)%dnx] == dny) {
                    transition_types.emplace_back(p1, p2, Direction::R);
                    transition_types.emplace_back(p2, p1, Direction::R);
                }
            }
        }
        rep(i,dny) if (horizontal_sorted_count[i] == dnx-2) rep(j,dnx) {
            const OriginalPos orig_p1 = Vec2(j,i);
            const OriginalPos orig_p2 = get_moved_toraly_pos(orig_p1,Direction::R,ctx);
            if (state[orig_p1.pos2idx(dnx)] == ordinary && state[orig_p2.pos2idx(dnx)] == ordinary) {
                const Pos p1 = Pos::idx2pos(rev_now_orig_pos[orig_p1.pos2idx(dnx)], dnx);
                const Pos p2 = Pos::idx2pos(rev_now_orig_pos[orig_p2.pos2idx(dnx)], dnx);
                if (p1 == moving_pos || p2 == moving_pos) continue;
                if (i == first_sorted_pos.y) {
                    if (adjustable_parity_chance_num == 1 && dny%2 == 1) {
                        transition_types.emplace_back(p1, p2, Direction::U, true);
                        transition_types.emplace_back(p1, p2, Direction::D, true);
                        transition_types.emplace_back(p2, p1, Direction::U, true);
                        transition_types.emplace_back(p2, p1, Direction::D, true);
                    }
                    transition_types.emplace_back(p1, p2, Direction::U);
                    transition_types.emplace_back(p1, p2, Direction::D);
                    transition_types.emplace_back(p2, p1, Direction::U);
                    transition_types.emplace_back(p2, p1, Direction::D);
                } else if (horizontal_sorted_count[(i+1)%dny] == dnx) {
                    transition_types.emplace_back(p1, p2, Direction::U);
                    transition_types.emplace_back(p2, p1, Direction::U);
                } else if (horizontal_sorted_count[(i+dny-1)%dny] == dnx) {
                    transition_types.emplace_back(p1, p2, Direction::D);
                    transition_types.emplace_back(p2, p1, Direction::D);
                }
            }
        }
        rep(i,dny) if (horizontal_sorted_count[i] != dnx-2) rep(j,dnx) if (vertical_sorted_count[j] != dny-2) {
            OriginalPos orig_candidate = Pos(j,i);
            Pos candidate = Pos::idx2pos(rev_now_orig_pos[orig_candidate.pos2idx(dnx)], dnx);
            Index orig_candidate_idx = orig_candidate.pos2idx(dnx);
            if (candidate != moving_pos && state[orig_candidate_idx] != sorted) {
                Direction dir = Direction::U;
                rep(k,4) {
                    Pos p1 = get_moved_toraly_pos(orig_candidate, dir, ctx); dir.rotate_cw();
                    if (i == first_sorted_pos.y || j == first_sorted_pos.x) {
                        if (state[p1.pos2idx(dnx)] == sorted) {
                            transition_types.emplace_back(candidate);
                            break;
                        }
                    } else {
                        Pos p2 = get_moved_toraly_pos(orig_candidate, dir, ctx);
                        if (state[p1.pos2idx(dnx)] == sorted && state[p2.pos2idx(dnx)] == sorted) {
                            const Pos dp = Pos(dir.get_dir_rotated_cw()) + Pos(dir.get_dir_reversed());
                            const Pos p3 = get_moved_toraly_pos(orig_candidate, dp*2, ctx);
                            if (state[p3.pos2idx(dnx)] != sorted) {
                                transition_types.emplace_back(candidate);
                                break;
                            }
                        }
                    }
                }
            }
        }

        return transition_types;
    }

    void transition(const TransitionType& tt, const Context& ctx) {
        if (tt.has_buddy) move_to_correct_pos(tt.target, tt.buddy, tt.free_dir, tt.should_adjust_parity, ctx);
        else move_to_correct_pos(tt.target, ctx);
    }
    void sort_last_2x2(const array<LightProcedures, 4096>& best_procs_for_2x2, const Context& ctx) {
        constexpr array<int, 4> offset{16,64,256, 1024};
        const int dnx = ctx.div_num.x, dny = ctx.div_num.y;
        Pos p;
        int selected_idx;
        if (state[get_moved_toraly_pos(moving_pos, Pos(-1, -1), ctx).pos2idx(dnx)] == ordinary) {
            p = get_moved_toraly_pos(moving_pos, Pos(-1, -1), ctx);
            selected_idx = 2;
        } else if (state[get_moved_toraly_pos(moving_pos, Pos(1, -1), ctx).pos2idx(dnx)] == ordinary) {
            p = get_moved_toraly_pos(moving_pos, Pos(0, -1), ctx);
            selected_idx = 3;
        } else if (state[get_moved_toraly_pos(moving_pos, Pos(-1, 1), ctx).pos2idx(dnx)] == ordinary) {
            p = get_moved_toraly_pos(moving_pos, Pos(-1, 0), ctx);
            selected_idx = 1;
        } else if (state[get_moved_toraly_pos(moving_pos, Pos(1, 1), ctx).pos2idx(dnx)] == ordinary) {
            p = get_moved_toraly_pos(moving_pos, Pos(0, 0), ctx);
            selected_idx = 0;
        } else Utility::exit_with_message("[sort_last_2x2] 0");
        vector<int> order;
        {
            vector<int> tmp;
            Direction dir = Direction::R;
            rep(i,4) {
                tmp.push_back(p.pos2idx(dnx));
                move_toraly(p, dir, ctx);
                dir.rotate_cw();
            }
            PRINT(tmp);
            swap(tmp[2], tmp[3]);
            rep(i,4) {
                rep(j,4) if (now_orig_pos[p.pos2idx(dnx)] == tmp[j]) order.push_back(j);
                move_toraly(p, dir, ctx);
                dir.rotate_cw();
            }
        }
        int now_selectable_times = ctx.selectable_times - procedures.size();

        int idx = min(3, now_selectable_times) + 4*selected_idx;
        rep(i,4) idx += order[i]*offset[i];
        PRINT(order);
        PRINT(idx);

        auto& procs = best_procs_for_2x2[idx];
        if (procs.empty()) Utility::exit_with_message("[sort_last_2x2] 1");

        {
            Path path = procs.front().get_path();
            move_current_selected_pos(path, ctx);
            procedures.back().join_path(path);
            PRINT(path);
        }

        repo(i,1,procs.size()) {
            Path path = procs[i].get_path();
            moving_pos = get_moved_toraly_pos(p, procs[i].get_selected_pos(), ctx);
            procedures.emplace_back(moving_pos);
            move_current_selected_pos(path, ctx);
            procedures.back().join_path(path);
            PRINT(path);
        }
        {
            Direction dir = Direction::R;
            rep(i,4) {
                state[p.pos2idx(dnx)] = sorted;
                move_toraly(p, dir, ctx);
                dir.rotate_cw();
            }
        }
    }
    Procedures simulate(const array<LightProcedures, 4096>& best_procs_for_2x2, const Context& ctx) {
        dump(ctx);
        Utility::print_wall("finish");
        DUMP();

        while(true) {
            DUMP("all possible transition");
            auto transition_types = get_all_possible_transition_types(ctx);
            for(const auto& transition_type : transition_types) {
                Pos target = transition_type.target;
                bool has_buddy = transition_type.has_buddy;
                Pos buddy = transition_type.buddy;
                Direction free_dir = transition_type.free_dir;
                bool parity = transition_type.should_adjust_parity;
                if (has_buddy) PRINT(target, buddy, free_dir, parity);
                else PRINT(target);
            }
            if (transition_types.empty()) {
                PRINT(procedures.size());
                procedures.front().dump();
                break;
            }

            int idx = Random::rand_range(transition_types.size());
            PRINT(idx);
            //            Utility::input(idx);

            transition(transition_types[idx], ctx);

            dump(ctx);
            Utility::print_wall("finish");
            DUMP();
        }

        sort_last_2x2(best_procs_for_2x2, ctx);

        dump(ctx);
        Utility::print_wall("finish");
        DUMP();

        Procedures procs;
        for(const auto& proc : procedures) procs.push_back(proc.convert_single_procedure());
        return procs;
    }
    Procedures get_procedures() const {
        Procedures procs;
        for(const auto& proc : procedures) procs.push_back(proc.convert_single_procedure());
        return procs;
    }

    void dump(const Context& ctx) {
        PRINT(intentionally_sorted_count);
        PRINT(moving_pos);
        rep(i, ctx.div_num.y) {
            rep(j, ctx.div_num.x) {
                Index idx = Vec2(j,i).pos2idx(ctx.div_num.x);
                stringstream ss;
                ss << hex << uppercase << now_orig_pos[idx] % ctx.div_num.x << now_orig_pos[idx] / ctx.div_num.x;
                if (state[idx] == 0) cout << ss.str() << "'  ";
                else cout << ss.str() << "'# ";
            }
            cout << endl;
        }
        cout << endl;
    }
};

/* *
 * 遷移
 * 最初に選択する場所(selected_pos)と最初に揃える場所(first_sorted_pos)を決めうつことで初期状態を作成(256*(256 - 31) = 57600)
 *
 * Pを見た時、Xがordinaryならok, それ以外ならng, ただし残り2点のところを除く
 *
 * # # . .
 * # P . .
 * . . . .
 * . . . X
 *
 * 普通のある一点を揃える(各点につき1) target: Pos, type: one_sort
 * 最初に揃えた場所があるライン以外のライン上の残りの2点を揃える(各ポイントにつき2, 実質各点につき1), target: Pos, buddy: optional<Pos>,
 * 最初に揃えた場所があるライン上で残りの2点を普通に揃える(各ポイントにつき4, 実質各点につき2) target: Pos, type
 * 最初に揃えた場所があるライン上で残りの2点を盤面が選択操作をこれ以上行わずに完成可能なようにして揃える(各ポイントにつき4, 実質各点につき2)
 *      ただし奇数のラインを超えられるのが最後の時に限り含ませる
 *
 * 全部やると仮定すると揃える順列を列挙する感じになると思う
 *
 * 256! ≒ 10^507
 *
 *
 *
 * enumerate_initial_state
 * step = div_num.x * div_num.y - 4;
 * rep(i, step)
 *     step-i's best state = ~~
 *     transition_types = tmp_state.get_all_possiable_transition_types;
 *     if (i+1 == step) HOGEHOEG
 *     for(transition_type : transition_types)
 *         tmp_state = step-i's best state
 *         tmp_state.transition(transition_type)
 *         storage.push(tmp_state)
 *
 *
 * get_all_possiable_transition_types
 * 必要なこと
 *
 *
 *
 *
 * */

void move_toraly(Pos& p, const Pos& dp, const Context& ctx) {
    p += dp;
    p.x = (p.x + ctx.div_num.x) % ctx.div_num.x;
    p.y = (p.y + ctx.div_num.y) % ctx.div_num.y;
}
void move_toraly(Pos& p, const Direction& dir, const Context& ctx) {
    move_toraly(p, Pos(dir), ctx);
}
Pos get_moved_toraly_pos(Pos p, const Pos& dp, const Context& ctx) {
    move_toraly(p, dp, ctx);
    return p;
}
Pos get_moved_toraly_pos(Pos p, const Direction& dir, const Context& ctx) {
    move_toraly(p, dir, ctx);
    return p;
}

void swap(Pos& selected_pos, const Direction& dir, OriginalPositions& original_positions, const Context& ctx) {
    Pos next = get_moved_toraly_pos(selected_pos, dir, ctx);
    swap(original_positions[selected_pos.y][selected_pos.x], original_positions[next.y][next.x]);
    selected_pos = next;
}

void dump_original_positions(const OriginalPositions& original_positions, const Context& ctx) {
    rep(i,ctx.div_num.y) {
        rep(j, ctx.div_num.x) {
            stringstream ss;
            ss << hex << uppercase << original_positions[i][j].x << original_positions[i][j].y;
            cout << ss.str() << " ";
        }
        cout << endl;
    }
}

bool is_completable(const OriginalPositions& original_positions, const Pos& selected_pos, const Context& ctx) {
    const auto& div_num = ctx.div_num;
    const int n = div_num.x * div_num.y;
    vector<int> now_order(n), rev_now_order(n);
    rep(i, div_num.y) rep(j, div_num.x) {
        Index idx = Vec2(j,i).pos2idx(div_num.x);
        Index orig_idx = original_positions[i][j].pos2idx(div_num.x);
        now_order[idx] = orig_idx;
        rev_now_order[orig_idx] = idx;
    }
    int swap_count = 0;
    rep(i,n) {
        if (now_order[i] != i) {
            Index target_idx = rev_now_order[i];
            swap(now_order[i], now_order[target_idx]);
            rev_now_order[i] = i;
            rev_now_order[now_order[target_idx]] = target_idx;
            swap_count++;
        }
    }
    int dist = selected_pos.sum();
    PRINT(dist);
    PRINT(swap_count);
    return ((swap_count+dist)%2) == 0;
}

array<LightProcedures, 4096> get_best_procs_for_2x2(const Context& ctx) {
    // 4^5 = 1024
    constexpr array<int, 4> offset{16,64,256, 1024};
    array<LightProcedures, 4096> best_procs_for_2x2;
    array<bool, 4096> visited{};
    reverse_priority_queue<ComparableData<int, tuple<int, int, vector<int>, LightProcedures>>> pq;

    rep(i,4) {
        vector<int> order = {0,1,3,2};
        LightSingleProcedure sp(Pos(order[i]%2, order[i]/2));
        pq.emplace(0, make_tuple(1, i, order, LightProcedures{sp}));
    }

    while(!pq.empty()) {
        int cost = pq.top().eval_val;
        int select_times, selected_idx;
        vector<int> order;
        LightProcedures proc;
        tie(select_times, selected_idx, order, proc) = pq.top().data;
        pq.pop();

        int now_idx = select_times + 4*selected_idx;
        rep(i,4) now_idx += order[i]*offset[i];
        if (visited[now_idx]) continue;
        repo(i, select_times, 4) {
            int tmp_idx = now_idx - select_times + i;
            if (visited[tmp_idx]) break;
            visited[tmp_idx] = true;
            best_procs_for_2x2[tmp_idx] = proc;
        }

        {
            int swap_target_idx = (selected_idx+1)%4;
            auto tmp_order = order;
            auto tmp_proc = proc;
            swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
            tmp_proc.back().join_path({Direction(Direction::R).get_dir_rotated_cw(selected_idx)});
            pq.emplace(cost+ctx.swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
        }
        {
            int swap_target_idx = (selected_idx+3)%4;
            auto tmp_order = order;
            auto tmp_proc = proc;
            swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
            tmp_proc.back().join_path({Direction(Direction::D).get_dir_rotated_cw(selected_idx)});
            pq.emplace(cost+ctx.swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
        }
        if (select_times < 3) rep(i,4) if (selected_idx != i) {
            auto tmp_proc = proc;
            constexpr array<int, 4> tmp{0,1,3,2};
            tmp_proc.emplace_back(Pos(tmp[i]%2, tmp[i]/2));
            pq.emplace(cost+ctx.select_cost, make_tuple(select_times+1, i, order, tmp_proc));
        }
    }

    Context tmp_ctx(Vec2<int>(2,2), 0, 0, 0);
    for(auto& procs : best_procs_for_2x2) {
        for(auto& proc : procs) {
            proc.reverse(tmp_ctx);
        }
        reverse(all(procs));
    }

    return best_procs_for_2x2;
}

void check_ans(OriginalPositions initial_orig_pos, const Procedures &procs, const Context& ctx) {
    auto div_num = ctx.div_num;
    auto& now_orig_pos = initial_orig_pos;
    for(const auto &proc : procs) {
        Pos current = proc.selected_pos;
        OriginalPos tmp = now_orig_pos[current.y][current.x];
        for(const auto dir : proc.path) {
            Pos next = current; move_toraly(next, dir, ctx);
            now_orig_pos[current.y][current.x] = now_orig_pos[next.y][next.x];
            current = next;
        }
        now_orig_pos[current.y][current.x] = tmp;
    }
    rep(i,div_num.y) {
        rep(j,div_num.x) {
            cout << hex << uppercase << now_orig_pos[i][j].x << now_orig_pos[i][j].y << " ";
        }
        cout << endl;
    }
    cout << endl;
    rep(i,div_num.y) rep(j,div_num.x) if (Vec2(j,i) != now_orig_pos[i][j]) {
        EPRINT(i, j, Vec2(j,i), now_orig_pos[i][j]);
        exit(-1);
    }
}


int calc_dist_toraly(const Pos& p1, const Pos& p2, const Context& ctx) {
    const int dx = abs(p1.x - p2.x);
    const int dy = abs(p1.y - p2.y);
    return min(dx, ctx.div_num.x-dx) + min(dy, ctx.div_num.y-dy);
}

int calc_penalty(const Pos& p, const OriginalPositions& original_positions, const Context& ctx) {
    const int dist = calc_dist_toraly(p, original_positions[p.y][p.x], ctx);
    return dist*dist;
}

void move(Pos& p, const Direction& dir, OriginalPositions& now_orig_pos, const Context& ctx) {
    Pos next = get_moved_toraly_pos(p, dir, ctx);
    swap(now_orig_pos[p.y][p.x], now_orig_pos[next.y][next.x]);
    p = next;
}
void revert_move(Pos& p, const Direction& dir, OriginalPositions& now_orig_pos, const Context& ctx) {
    move(p, dir.get_dir_reversed(), now_orig_pos, ctx);
}
int move_and_calc_diff_penalty(Pos& p, const Direction& dir, OriginalPositions& now_orig_pos, const Context& ctx) {
    int diff_penalty = 0;
    Pos next = get_moved_toraly_pos(p, dir, ctx);
    diff_penalty -= calc_penalty(next, now_orig_pos, ctx);
    swap(now_orig_pos[p.y][p.x], now_orig_pos[next.y][next.x]);
    diff_penalty += calc_penalty(p, now_orig_pos, ctx);
    p = next;
    return diff_penalty;
}
int revert_move_and_calc_diff_penalty(Pos& p, const Direction& dir, OriginalPositions& now_orig_pos, const Context& ctx) {
    return move_and_calc_diff_penalty(p, dir.get_dir_reversed(), now_orig_pos, ctx);
}

void sort_roughly(OriginalPositions& now_orig_pos, const Context& ctx) {
    OriginalPositions original_orig_pos = now_orig_pos;
    const int dnx = ctx.div_num.x;
    const int dny = ctx.div_num.y;
    double coef = 0.;

    int now_penalty = 0;
    rep(i,dny) rep(j,dnx) if (i+j) now_penalty += calc_penalty(Vec2(j,i), now_orig_pos, ctx);
    PRINT(now_penalty);

    int loop_count = 0;
    int prev_upgrade_time = -1;

    int best_penalty = now_penalty;
    Path best_path;

    Pos moving_pos;
    Path now_path;
    TimeManager tm(10000);

    PRINT(now_penalty);
    while(tm.in_time_limit()) {
        PRINT(loop_count, now_path.size());
        loop_count++;

        // serach additional path
        constexpr int depth = 9;
        int min_diff_penalty = INT_MAX;
        Path best_addnl_path;

        int now_diff_penalty = 0;
        Path now_addnl_path; now_addnl_path.reserve(depth);
        while(true) {
            if (now_addnl_path.size() == depth) {
                while(!now_addnl_path.empty() && now_addnl_path.back() == Direction::L) {
                    Direction back_rev_dir = now_addnl_path.back().get_dir_reversed();
                    now_diff_penalty += revert_move_and_calc_diff_penalty(moving_pos, now_addnl_path.back(), now_orig_pos, ctx);
                    now_addnl_path.pop_back();
                }
                if (now_addnl_path.empty()) break;
                now_diff_penalty += revert_move_and_calc_diff_penalty(moving_pos, now_addnl_path.back(), now_orig_pos, ctx);
                now_addnl_path.back().rotate_cw();
                now_diff_penalty += move_and_calc_diff_penalty(moving_pos, now_addnl_path.back(), now_orig_pos, ctx);
            } else {
                constexpr Direction dir = Direction::U;
                now_addnl_path.push_back(dir);
                now_diff_penalty += move_and_calc_diff_penalty(moving_pos, dir, now_orig_pos, ctx);
            }

            if (min_diff_penalty > now_diff_penalty + now_addnl_path.size() * coef) {
                min_diff_penalty = now_diff_penalty + now_addnl_path.size() * coef;
                best_addnl_path = now_addnl_path;
            }
        }

        if (min_diff_penalty < 0) { // if I found good additional path
            // add path and change
            for(const auto& dir : best_addnl_path) move(moving_pos, dir, now_orig_pos, ctx);
            join_path(now_path, best_addnl_path);
            now_penalty += min_diff_penalty;
            if (best_penalty > now_penalty) {
                best_penalty = now_penalty;
                best_path = now_path;
                PRINT(loop_count, best_penalty);
                prev_upgrade_time = loop_count;
            }
        } else if (!now_path.empty()) { // if I couldn't find additional path
            // kick
            if (loop_count - prev_upgrade_time > 100) {
                now_path = best_path;
                now_penalty = best_penalty;
                now_orig_pos = original_orig_pos;
                moving_pos = Pos();
                for(const auto& dir : now_path) move(moving_pos, dir, now_orig_pos, ctx);
                prev_upgrade_time = loop_count;
            } else {
                now_penalty -= now_path.size() * coef;
                int delete_num = Random::simple_exp_rand(now_path.size());
                rep(_,delete_num) {
                    now_penalty += revert_move_and_calc_diff_penalty(moving_pos, now_path.back(), now_orig_pos, ctx);
                    now_path.pop_back();
                }
                now_penalty += now_path.size() * coef;
            }
        } else break;
    }
    PRINT(loop_count);
    PRINT(best_penalty);
    PRINT(best_path.size());
    PRINT(best_path);

    moving_pos = Pos();
    now_orig_pos = original_orig_pos;
    for(const auto& dir : best_path) move(moving_pos, dir, now_orig_pos, ctx);
    int tmp_penalty = - calc_penalty(moving_pos, now_orig_pos, ctx);
    rep(i,dny) rep(j,dnx) tmp_penalty += calc_penalty(Pos(j,i), now_orig_pos, ctx);
    PRINT(tmp_penalty);
    PRINT(tmp_penalty + best_path.size()*coef);
    PRINT(moving_pos);
    rep(i, dny) {
        rep(j,dnx) {
            int penalty = calc_penalty(Vec2(j,i), now_orig_pos, ctx);
            if (penalty == 0) cout << "  .";
            else cout << setw(3) << calc_penalty(Vec2(j,i), now_orig_pos, ctx);
        }
        cout << endl;
    }
    cout << endl;
}

/*/
 *
 * 2x2の場合は完全解を求めることが可能
 * 3x3もギリいけるかも
 * でも3x3は実用性薄そう、一応制限にもなってしまうし
 *
 * 最初に分ける点を決め打って探索した方が良いのかもしれない
 * 実は盤外移動するメリットはあんまりない？
 *
 * 最初に適当に揃える時には盤外移動を考慮するけどちゃんと揃えるときは移動しないようにしたほうが楽なのかもしれない
 * 偶奇性の調整は最初でやっておくと良いかも
 *
 *
 *
/*/


Procedures KrSolver::operator()(const OriginalPositions& original_positions, const Settings& settings) {
    const Context ctx( static_cast<Vec2<int>>(settings.DIV_NUM())
                       , static_cast<int>(settings.SELECTABLE_TIMES())
                       , static_cast<int>(settings.CHOICE_COST())
                       , static_cast<int>(settings.SWAP_COST()) );

    OriginalPositions now_orig_pos = original_positions;
    sort_roughly(now_orig_pos, ctx);
    dump_original_positions(now_orig_pos, ctx);

    int loop_count = 0;
    while(++loop_count && Random::rand_range(256) != 1) {}
    PRINT(loop_count);
    PRINT(sizeof(string));
    PRINT(sizeof(Pos));
    PRINT(sizeof(optional<char>));
    PRINT(sizeof(optional<int>));
    PRINT(sizeof(optional<Direction>));
    PRINT(sizeof(LimitedStorage<State>));
    PRINT(sizeof(State));
    PRINT(sizeof(char));
    PRINT(sizeof(unsigned char));
    PRINT(sizeof(bool));


    auto best_procs_for_2x2 = get_best_procs_for_2x2(ctx);

/*
    int count = 0;
    rep(i,4096) {
        if (!tmp[i].empty()) {
            int select_times = i&3;
            if (select_times == 3) {
                int selected_idx = (i&12)>>2;
                vector<int> order = {(i&48)>>4, (i&192)>>6, (i&768)>>8, (i&3072)>>10};
                PRINT(i, select_times, selected_idx, order);
                for(const auto& hoge : tmp[i]) {
                    hoge.dump();
                }
                Utility::print_wall();
            }
            if (select_times == 3) count++;
        }
    }
    PRINT(count);
*/

    constexpr size_t storage_size = 200000;
    const int dnx = ctx.div_num.x, dny = ctx.div_num.y;
    const size_t n = dnx*dny;
    const size_t unit_storage_size = storage_size / n;
    vector<LimitedStorage<State>> storages(n, LimitedStorage<State>(unit_storage_size));

    dump_original_positions(now_orig_pos, ctx);
//    dump_original_positions(original_positions, ctx);

    vector<reverse_priority_queue<ComparableData<double, LimitedStorage<State>::Ticket>>> pq(n);
    set<pair<Pos, Pos>> pushed_initial_state;
    vector<map<size_t, double>> hash__score(n);

    double min_cost = 1e20;
    Procedures best_procs;

/*
    int count = 0;
    rep(i,4096) {
        if (!best_procs_for_2x2[i].empty()) {
            int select_times = i&3;
            if (select_times == 3) {
                int selected_idx = (i&12)>>2;
                vector<int> order = {(i&48)>>4, (i&192)>>6, (i&768)>>8, (i&3072)>>10};
                PRINT(i, select_times, selected_idx, order);
                for(const auto& hoge : best_procs_for_2x2[i]) {
                    hoge.dump();
                }
                Utility::print_wall();
            }
            if (select_times == 3) count++;
        }
    }
    PRINT(count);
*/

/*
    rep(i,dny) rep(j,dnx) rep(I,dny) rep(J,dnx) {
        PRINT(i, j, I, J);
        Pos selected_pos = Pos(j,i);
        Pos first_target_pos = Pos(J,I);
        Pos s_orig_pos = original_positions[selected_pos.y][selected_pos.x];
        Pos t_orig_pos = original_positions[first_target_pos.y][first_target_pos.x];
        if (s_orig_pos.x == t_orig_pos.x || s_orig_pos.y == t_orig_pos.y) continue;
        State state(original_positions, selected_pos, first_target_pos, ctx);

        while(true) {
            auto transition_types = state.get_all_possible_transition_types(ctx);
            if (transition_types.empty()) break;
            double tmp_min_cost = 1e20;
            State best_state;
            for(const auto& tt : transition_types) {
                State tmp_state = state;
                tmp_state.transition(tt, ctx);
                auto [cost, hash_val] = tmp_state.evaluate(ctx);
                if (tmp_min_cost > cost) {
                    tmp_min_cost = cost;
                    best_state = tmp_state;
                }
            }
            state = best_state;
        }

        state.dump(ctx);

        state.sort_last_2x2(best_procs_for_2x2, ctx);

        auto [cost, hash_val] = state.evaluate(ctx);
        PRINT(min_cost, cost);
        if (min_cost > cost) {
            min_cost = cost;
            best_procs = state.get_procedures();
        }
    }
*/

    int step = 0, step_zero_count = 0, continue_count = 0;
    const int step_zero_limit = n * (n - ctx.div_num.x - ctx.div_num.y + 1);
    TimeManager tm(10000);
    while(tm.in_time_limit()) {
        if (continue_count >= n) break;
        if (step == 0) {
            if (step_zero_count == step_zero_limit) { step++; continue_count++; continue; }
            Pos selected_pos = Pos(Random::rand_range(ctx.div_num.x), Random::rand_range(ctx.div_num.y));
            Pos first_target_pos = Pos(Random::rand_range(ctx.div_num.x), Random::rand_range(ctx.div_num.y));
            Pos s_orig_pos = now_orig_pos[selected_pos.y][selected_pos.x];
            Pos t_orig_pos = now_orig_pos[first_target_pos.y][first_target_pos.x];
//            Pos s_orig_pos = original_positions[selected_pos.y][selected_pos.x];
//            Pos t_orig_pos = original_positions[first_target_pos.y][first_target_pos.x];
            while(s_orig_pos.x == t_orig_pos.x || s_orig_pos.y == t_orig_pos.y || pushed_initial_state.count(make_pair(selected_pos, first_target_pos))) {
                first_target_pos = Pos(Random::rand_range(ctx.div_num.x), Random::rand_range(ctx.div_num.y));
                t_orig_pos = now_orig_pos[first_target_pos.y][first_target_pos.x];
//                t_orig_pos = original_positions[first_target_pos.y][first_target_pos.x];
            }
            State state(now_orig_pos, selected_pos, first_target_pos, ctx);
//            State state(original_positions, selected_pos, first_target_pos, ctx);

            auto [cost, hash_val] = state.evaluate(ctx);
            auto tkt = storages[step].push(state);
            if (hash__score[step].count(hash_val) == 0 || hash__score[step][hash_val] > cost) {
                hash__score[step][hash_val] = cost;
                pq[step].emplace(cost, tkt);
            }
            pushed_initial_state.emplace(selected_pos, first_target_pos);
            step++;
            step_zero_count++;
        } else {
            if (pq[step-1].empty()) { step = (step+1)%n; continue_count++; continue;}
            auto cd = pq[step-1].top(); pq[step-1].pop();
            auto tkt = cd.data;
            auto op_state = storages[step-1].pop(tkt);
            if (op_state) {
                if (step == 224) PRINT(step);
                auto state = op_state.value();
                auto transition_types = state.get_all_possible_transition_types(ctx);
                if (transition_types.empty()) {
                    state.dump(ctx);
                    state.sort_last_2x2(best_procs_for_2x2, ctx);
                    auto [cost, hash_val] = state.evaluate(ctx);
                    PRINT(cost, min_cost);
                    if (min_cost > cost) {
                        min_cost = cost;
                        best_procs = state.get_procedures();
                    }
                } else {
                    double tmp_min_cost = 1e20;
                    State tmp_best_state;
                    for(const auto& tt : transition_types) {
                        State tmp_state = state;
                        tmp_state.transition(tt, ctx);
                        auto [cost, hash_val] = tmp_state.evaluate(ctx);
                        if (tmp_min_cost > cost) {
                            tmp_min_cost = cost;
                            tmp_best_state = tmp_state;
                        }
/*
                        if (hash__score[step].count(hash_val) == 0 || hash__score[step][hash_val] > cost) {
                            PRINT("HOGE ##########");
                            auto tkt = storages[step].push(tmp_state);
                            hash__score[step][hash_val] = cost;
                            pq[step].emplace(cost, tkt);
                        } else PRINT("HOGE ........");
*/
                    }
                    auto [cost, hash_val] = tmp_best_state.evaluate(ctx);
                    if (hash__score[step].count(hash_val) == 0 || hash__score[step][hash_val] > cost) {
                        auto tkt = storages[step].push(tmp_best_state);
                        hash__score[step][hash_val] = cost;
                        pq[step].emplace(cost, tkt);
                    }
                }
            } else DUMP("HOGEHOGEHOGEHOGHOEHGOEHOGEHGOE");
            step++;
        }
        continue_count = 0;
    }

    cout << min_cost << endl;
    cout << best_procs << endl;

//    check_ans(original_positions, best_procs, ctx);
    check_ans(now_orig_pos, best_procs, ctx);

    return best_procs;
}