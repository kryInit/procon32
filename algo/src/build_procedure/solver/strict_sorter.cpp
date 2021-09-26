#include <strict_sorter.hpp>
#include <rough_sorter.hpp>

#include <comparable_data.hpp>
#include <time_manager.hpp>
#include <weak_vector.hpp>
#include <utility.hpp>
#include <macro.hpp>
#include <constant.hpp>

using namespace std;

array<array<array<pair<Pos, Direction>, 4>, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> get_all_neighborhood_pos() {
    array<array<array<pair<Pos, Direction>, 4>, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> result{};
    rep(i,div_num.y) rep(j,div_num.x) {
        int th = 0;
        for(const auto dir : Direction::All) {
            result[i][j][th] = make_pair(Pos(j,i).get_moved_toraly_pos(dir, div_num), dir);
            th++;
        }
    }
    return result;
}


array<array<array<pair<Pos, Direction>, 4>, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> neighborhood_pos{};

bool calc_shortest_path(Path& path, const Pos& s, const Pos& t, const State& state) {
    path.clear();
    if (s == t) return true;

//    weak_vector<pair<Pos, Direction>, MAX_DIV_NUM> wv1, wv2;
//    array<array<Direction, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> from{};
//    array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> visited{};

    static weak_vector<pair<Pos, Direction>, MAX_DIV_NUM> wv1, wv2;
    static array<array<Direction, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> from{};
    static array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> pushed{};
    wv1.clear(), wv2.clear();
    rep(i,div_num.y) pushed[i].fill(false);

/*

    if (state.board_state[s.y][s.x] != State::ordinary) {
        auto message = Utility::concat("state[s_idx] is ", state.board_state[s.y][s.x]);
        EXIT_DEBUG(message);
    }
    if (state.board_state[t.y][t.x] != State::ordinary) {
        auto message = Utility::concat("state[t_idx] is ", state.board_state[t.y][t.x]);
        EXIT_DEBUG(message);
    }

*/
    int loop_count = 0;
    wv1.emplace_back(s, Direction::U);
    while(true) {
        loop_count++;
        auto& current = loop_count%2 ? wv1 : wv2;
        auto& next = loop_count%2 ? wv2 : wv1;
        for(const auto& [now, prev_dir] : current) {
            from[now.y][now.x] = prev_dir;
            for(const auto& [next_p, dir] : neighborhood_pos[now.y][now.x]) {
                if (state.board_state[next_p.y][next_p.x] == State::ordinary && !pushed[next_p.y][next_p.x]) {
                    if (next_p == t) {
                        from[next_p.y][next_p.x] = dir;

                        Pos p = t;
                        while(p != s) {
                            const Direction tmp_dir = from[p.y][p.x];
                            path.push_back(tmp_dir);
                            p.move_toraly(tmp_dir.get_dir_reversed(), div_num);
                        }
                        reverse(all(path));
                        return true;
                    }
                    next.emplace_back(next_p, dir);
                    pushed[next_p.y][next_p.x] = true;
                }
            }
        }
        if (next.empty()) break;
        current.clear();
    }

    return false;
}

/*

[[nodiscard]] optional<Path> calc_nice_path(const Pos& s, const Pos& t, const State& state) {
    deque<pair<Pos, Direction>> que;
    array<array<Direction, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> from{};
    array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> visited{};

    if (state.board_state[s.y][s.x] != State::ordinary) {
        auto message = Utility::concat("state[s_idx] is ", state.board_state[s.y][s.x]);
        EXIT_DEBUG(message);
    }
    if (state.board_state[t.y][t.x] != State::ordinary) {
        auto message = Utility::concat("state[t_idx] is ", state.board_state[t.y][t.x]);
        EXIT_DEBUG(message);
    }

    bool reached = false;
    que.emplace_back(s, Direction::U);
    while(!que.empty()) {
        const Pos now = que.front().first;
        const Direction prev_dir = que.front().second;
        que.pop_front();
        if (visited[now.y][now.x]) continue;
        visited[now.y][now.x] = true;
        from[now.y][now.x] = prev_dir;
        if (now == t) { reached = true; break; }
        for(const auto& dir : Direction::All) {
            Pos next = now.get_moved_toraly_pos(dir, div_num);
            if (state.board_state[next.y][next.x] == State::ordinary) {
                bool back = false;

                {
                    Pos orig_next = state.orig_pos[next.y][next.x];

                    int now_to_orig_next_dist = 0;
                    int next_to_orig_next_dist = 0;

                    const Pos offset = state.first_sorted_pos;
                    Pos shifted_now = now + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;
                    Pos shifted_next = next + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;
                    Pos shifted_orig_next = orig_next + div_num - offset; shifted_now.x %= div_num.x; shifted_now.y %= div_num.y;

                    now_to_orig_next_dist += abs(shifted_now.x - shifted_orig_next.x);
                    now_to_orig_next_dist += abs(shifted_now.y - shifted_orig_next.y);

                    next_to_orig_next_dist += abs(shifted_next.x - shifted_orig_next.x);
                    next_to_orig_next_dist += abs(shifted_next.y - shifted_orig_next.y);

                    back = now_to_orig_next_dist > next_to_orig_next_dist;
                }

                if (back) que.emplace_back(next, dir);
                else que.emplace_front(next, dir);
            }
        }
    }
    if (!reached) return nullopt;
    Path path;
    Pos now = t;
    while(now != s) {
        const Direction dir = from[now.y][now.x];
        path.push_back(dir);
        now.move_toraly(dir.get_dir_reversed(), div_num);
    }
    reverse(all(path));
    return path;
}

*/

bool move_target_to_destination_by_selected_pos(const Pos& target, const Pos& destination, State& state) {
    if (state.board_state[target.y][target.x] != State::ordinary) {
        EXIT_DEBUG("target's state isn't ordinary");
    }

    Pos now = target;
    static Path targets_path;
    calc_shortest_path(targets_path, target, destination, state);
    for(const auto& dir : targets_path) {
        Pos next = now.get_moved_toraly_pos(dir, div_num);
        state.board_state[now.y][now.x] = State::unmovable;

        static Path additional_path;
        bool succeeded = calc_shortest_path(additional_path, state.selected_pos, next, state);
        state.board_state[now.y][now.x] = State::ordinary;
        if (succeeded) {
            additional_path.push_back(dir.get_dir_reversed());
            state.move_selected_pos(additional_path);
            now = next;
        } else return false;
    }
    return true;
}

void move_to_correct_pos(const Pos& target, State& state) {
    Pos destination = state.orig_pos[target.y][target.x];
    if (target == destination) {
        state.board_state[destination.y][destination.x] = State::sorted;
        return;
    }
    auto succeeded = move_target_to_destination_by_selected_pos(target, destination, state);
    if (succeeded) {
        state.board_state[destination.y][destination.x] = State::sorted;
    } else EXIT_DEBUG("failed");
}

void move_to_correct_pos(const Pos& target, const Pos& first_buddy, Direction free_dir, State& state) {
    const Pos orig_target = state.orig_pos[target.y][target.x];
    const Pos orig_buddy = state.orig_pos[first_buddy.y][first_buddy.x];

    if (target == orig_target && first_buddy == orig_buddy) {
        state.board_state[orig_target.y][orig_target.x] = State::sorted;
        state.board_state[orig_buddy.y][orig_buddy.x] = State::sorted;
        return;
    }
    const Pos destination1 = orig_buddy;
    const Pos destination2 = orig_buddy.get_moved_toraly_pos(free_dir, div_num);
    const Pos destination3 = orig_target;
    Direction target_to_buddy_dir = Direction::U;
    rep(_,4) {
        if (orig_target.get_moved_toraly_pos(target_to_buddy_dir, div_num) == orig_buddy) break;
        target_to_buddy_dir.rotate_cw();
        if (_ == 3) EXIT_DEBUG("");
    }

    if (state.board_state[destination1.y][destination1.x] != State::ordinary || state.board_state[destination2.y][destination2.x] != State::ordinary) {
        EXIT_DEBUG("");
    }

    bool succeeded;

    succeeded = move_target_to_destination_by_selected_pos(target, destination1, state);
    if (!succeeded) EXIT_DEBUG("");
    state.board_state[destination1.y][destination1.x] = State::unmovable;

    Pos now_buddy = state.get_now_pos_by_original_pos(orig_buddy);
    succeeded = move_target_to_destination_by_selected_pos(now_buddy, destination2, state);
    state.board_state[destination2.y][destination2.x] = State::unmovable;
    if (!succeeded) {
        const Path* p_path;
        if (state.selected_pos != orig_target) EXIT_DEBUG("moving_pos, orig_target: ",state.selected_pos,orig_target);
        if (target_to_buddy_dir.is_vertical() == free_dir.is_vertical()) EXIT_DEBUG("");
        if (target_to_buddy_dir == Direction::U) {
            if (free_dir == Direction::R) p_path = &column_UR;
            else p_path = &column_UL;
        } else if (target_to_buddy_dir == Direction::R) {
            if (free_dir == Direction::U) p_path = &row_UR;
            else p_path = &row_DR;
        } else if (target_to_buddy_dir == Direction::D) {
            if (free_dir == Direction::R) p_path = &column_DR;
            else p_path = &column_DL;
        } else {
            if (free_dir == Direction::U) p_path = &row_UL;
            else p_path = &row_DL;
        }
        state.board_state[destination1.y][destination1.x] = State::ordinary;
        state.board_state[destination2.y][destination2.x] = State::ordinary;

        state.move_selected_pos(*p_path);

        state.board_state[orig_target.y][orig_target.x] = State::sorted;
        state.board_state[orig_buddy.y][orig_buddy.x] = State::sorted;
        return;
    }

    static Path tmp_path;
    succeeded = calc_shortest_path(tmp_path, state.selected_pos, destination3, state);
    if (!succeeded) EXIT_DEBUG("");
    state.move_selected_pos(tmp_path);

    state.board_state[destination1.y][destination1.x] = State::ordinary;
    state.board_state[destination2.y][destination2.x] = State::ordinary;

    state.move_selected_pos({target_to_buddy_dir, free_dir});

    state.board_state[orig_target.y][orig_target.x] = State::sorted;
    state.board_state[orig_buddy.y][orig_buddy.x] = State::sorted;
}

struct TransitionType {
    Pos target;
    bool has_buddy;
    Pos buddy;
    Direction free_dir;
    explicit TransitionType(Pos target) : target(target), has_buddy(false), buddy(), free_dir() {}
    TransitionType(Pos target, Pos buddy, Direction free_dir) : target(target), has_buddy(true), buddy(buddy), free_dir(free_dir) {}
};

[[nodiscard]] vector<TransitionType> get_all_possible_transition_types(const State& state) {
    vector<TransitionType> transition_types;

    const int dnx = div_num.x, dny = div_num.y;
    const int n = dnx*dny;
    static array<array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> rev_now_orig_pos{};
    static array<unsigned char, MAX_DIV_NUM_X> vertical_sorted_count{};
    static array<unsigned char, MAX_DIV_NUM_Y> horizontal_sorted_count{};
    vertical_sorted_count.fill(0);
    horizontal_sorted_count.fill(0);

    rep(i,dny) rep(j,dnx) rev_now_orig_pos[state.orig_pos[i][j].y][state.orig_pos[i][j].x] = Vec2(j,i);
    rep(i,dny) horizontal_sorted_count[i] = 0;
    rep(j,dnx) vertical_sorted_count[j] = 0;

    int sorted_count = 0;
    rep(i, dny) rep(j, dnx) if (state.board_state[i][j] == State::sorted) {
        sorted_count++;
        vertical_sorted_count[j]++;
        horizontal_sorted_count[i]++;
    }
    if (sorted_count+4 == n) return {};

    rep(j,dnx) if (vertical_sorted_count[j] == dny-2) rep(i,dny) {
        const Pos orig_p1 = Vec2(j,i);
        const Pos orig_p2 = orig_p1.get_moved_toraly_pos(Direction::D, div_num);
        if (state.board_state[orig_p1.y][orig_p2.x] == State::ordinary && state.board_state[orig_p2.y][orig_p2.x] == State::ordinary) {
            const Pos p1 = rev_now_orig_pos[orig_p1.y][orig_p1.x];
            const Pos p2 = rev_now_orig_pos[orig_p2.y][orig_p2.x];
            if (p1 == state.selected_pos || p2 == state.selected_pos) continue;
            if (j == state.first_sorted_pos.x) {
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
            break;
        }
    }
    rep(i,dny) if (horizontal_sorted_count[i] == dnx-2) rep(j,dnx) {
        const Pos orig_p1 = Vec2(j,i);
        const Pos orig_p2 = orig_p1.get_moved_toraly_pos(Direction::R,div_num);
        if (state.board_state[orig_p1.y][orig_p1.x] == State::ordinary && state.board_state[orig_p2.y][orig_p2.x] == State::ordinary) {
            const Pos p1 = rev_now_orig_pos[orig_p1.y][orig_p1.x];
            const Pos p2 = rev_now_orig_pos[orig_p2.y][orig_p2.x];
            if (p1 == state.selected_pos || p2 == state.selected_pos) continue;
            if (i == state.first_sorted_pos.y) {
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
            break;
        }
    }
    rep(i,dny) if (horizontal_sorted_count[i] != dnx-2) rep(j,dnx) if (vertical_sorted_count[j] != dny-2) {
        const Pos orig_candidate = Pos(j,i);
        const Pos candidate = rev_now_orig_pos[orig_candidate.y][orig_candidate.x];
        if (candidate != state.selected_pos && state.board_state[orig_candidate.y][orig_candidate.x] != State::sorted) {
            for(const auto& dir : Direction::All) {
                const Pos p1 = orig_candidate.get_moved_toraly_pos(dir, div_num);
                if (state.board_state[p1.y][p1.x] == State::sorted) {
                    if (i == state.first_sorted_pos.y || j == state.first_sorted_pos.x) {
                        transition_types.emplace_back(candidate);
                        break;
                    }
                    const Pos p2 = orig_candidate.get_moved_toraly_pos(dir.get_dir_rotated_cw(), div_num);
                    if (state.board_state[p2.y][p2.x] == State::sorted) {
                        const Pos dp = Pos(dir.get_dir_rotated_ccw()) + Pos(dir.get_dir_reversed());
                        const Pos p3 = orig_candidate.get_moved_toraly_pos(dp*2, div_num);
                        if (state.board_state[p3.y][p3.x] != State::sorted) {
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


void calc_best_procs_for_2x2() {
    constexpr array<int, 4> offset{16,64,256, 1024};
    array<bool, 4096> visited{};
    Utility::reverse_priority_queue<ComparableData<int, tuple<int, int, vector<int>, Procedures>>> pq;

    rep(i,4) {
        vector<int> order = {0,1,3,2};
        SingleProcedure sp(Pos(order[i]%2, order[i]/2), Path());
        pq.emplace(0, make_tuple(1, i, order, Procedures{sp}));
    }

    while(!pq.empty()) {
        int cost = pq.top().rating;
        int select_times, selected_idx;
        vector<int> order;
        Procedures proc;
        tie(select_times, selected_idx, order, proc) = pq.top().data;
        pq.pop();

        int now_idx = select_times + 4*selected_idx;
        rep(i,4) now_idx += order[i]*offset[i];
        if (visited[now_idx]) continue;
        repr(i, select_times, 4) {
            int tmp_idx = now_idx - select_times + i;
            if (visited[tmp_idx]) break;
            visited[tmp_idx] = true;
            StrictSorter::best_procs_for_2x2[tmp_idx] = proc;
        }

        {
            int swap_target_idx = (selected_idx+1)%4;
            auto tmp_order = order;
            auto tmp_proc = proc;
            swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
            tmp_proc.back().path.join({Direction(Direction::R).get_dir_rotated_cw(selected_idx)});
            pq.emplace(cost+swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
        }
        {
            int swap_target_idx = (selected_idx+3)%4;
            auto tmp_order = order;
            auto tmp_proc = proc;
            swap(tmp_order[selected_idx], tmp_order[swap_target_idx]);
            tmp_proc.back().path.join({Direction(Direction::D).get_dir_rotated_cw(selected_idx)});
            pq.emplace(cost+swap_cost, make_tuple(select_times, swap_target_idx, tmp_order, tmp_proc));
        }
        if (select_times < 3) rep(i,4) if (selected_idx != i) {
            auto tmp_proc = proc;
            constexpr array<int, 4> tmp{0,1,3,2};
            tmp_proc.emplace_back(Pos(tmp[i]%2, tmp[i]/2), Path());
            pq.emplace(cost+select_cost, make_tuple(select_times+1, i, order, tmp_proc));
        }
    }

    for(auto& procs : StrictSorter::best_procs_for_2x2) {
        for(auto& proc : procs) {
            for(auto& dir : proc.path) {
                proc.selected_pos.move(dir);
                dir.reverse();
            }
            reverse(all(proc.path));
        }
        reverse(all(procs));
    }
}

void transition(const TransitionType& tt, State& state) {
    if (tt.has_buddy) move_to_correct_pos(tt.target, tt.buddy, tt.free_dir, state);
    else move_to_correct_pos(tt.target, state);
}
void revert_transition(const TransitionType& tt, State& state, const int prev_path_size) {
    state.revert_move_selected_pos(prev_path_size);
    if (tt.has_buddy) {
        const Pos target = tt.target;
        const Pos buddy = tt.buddy;
        const Pos orig_target = state.orig_pos[target.y][target.x];
        const Pos orig_buddy = state.orig_pos[buddy.y][buddy.x];
        state.board_state[orig_target.y][orig_target.x] = State::ordinary;
        state.board_state[orig_buddy.y][orig_buddy.x] = State::ordinary;
    } else {
        Pos target = tt.target;
        Pos destination = state.orig_pos[target.y][target.x];
        state.board_state[destination.y][destination.x] = State::ordinary;
    }
}
void sort_last_2x2(State& state, int max_select_times, bool minimize_cost = false) {
    if (max_select_times < 1) EXIT_DEBUG("");
    constexpr array<int, 4> offset{16,64,256, 1024};
    Pos p;
    int selected_idx;
    if (p = state.selected_pos.get_moved_toraly_pos(Pos(-1,-1), div_num); state.board_state[p.y][p.x] == State::ordinary) selected_idx = 2;
    else if (p = state.selected_pos.get_moved_toraly_pos(Pos( 1,-1), div_num); state.board_state[p.y][p.x] == State::ordinary) {
        p = state.selected_pos.get_moved_toraly_pos(Pos(0,-1), div_num);
        selected_idx = 3;
    }
    else if (p = state.selected_pos.get_moved_toraly_pos(Pos(-1, 1), div_num); state.board_state[p.y][p.x] == State::ordinary) {
        p = state.selected_pos.get_moved_toraly_pos(Pos(-1,0), div_num);
        selected_idx = 1;
    }
    else if (p = state.selected_pos.get_moved_toraly_pos(Pos( 1, 1), div_num); state.board_state[p.y][p.x] == State::ordinary) {
        p = state.selected_pos.get_moved_toraly_pos(Pos(0,0), div_num);
        selected_idx = 0;
    }
    else EXIT_DEBUG("");

    vector<int> order;
    {
        vector<Pos> tmp;
        Direction dir = Direction::R;
        rep(i,4) {
            tmp.push_back(p);
            p.move_toraly(dir, div_num);
            dir.rotate_cw();
        }
        swap(tmp[2], tmp[3]);
        rep(i,4) {
            rep(j,4) if (state.orig_pos[p.y][p.x] == tmp[j]) order.push_back(j);
            p.move_toraly(dir, div_num);
            dir.rotate_cw();
        }
    }
    int idx = 4*selected_idx;
    rep(i,4) idx += order[i]*offset[i];
    if (minimize_cost) {
        idx += min(3, max_select_times);
    } else {
        repr(i,1,min(3,max_select_times)+1) if (!StrictSorter::best_procs_for_2x2[idx + i].empty()) {
            idx += i;
            break;
        }
    }



    auto& procs = StrictSorter::best_procs_for_2x2[idx];
    if (procs.empty()) EXIT_DEBUG("");

    {
        Path path = procs.front().path;
        state.move_selected_pos(path);
    }

    repr(i,1,procs.size()) {
        Path path = procs[i].path;
        state.selected_pos = p.get_moved_toraly_pos(procs[i].selected_pos, div_num);
        state.proc.emplace_back(state.selected_pos, Path());
        state.move_selected_pos(path);
    }
    {
        Direction dir = Direction::R;
        rep(i,4) {
            state.board_state[p.y][p.x] = State::sorted;
            p.move_toraly(dir, div_num);
            dir.rotate_cw();
        }
    }
}

int StrictSorter::calc_cost(const State& state) {
    int cost = (int)state.proc.size() * select_cost;
    for(const auto& proc : state.proc) cost += (int)proc.path.size() * swap_cost;
    return cost;
}

void StrictSorter::init() {
    calc_best_procs_for_2x2();
    neighborhood_pos = get_all_neighborhood_pos();
}

void StrictSorter::sort_start2finish(const Pos& first_selected_pos, const Pos& first_target, State& state) {

    if (state.selected_pos != first_selected_pos) {
        state.selected_pos = first_selected_pos;
        state.proc.emplace_back(first_selected_pos, Path());
    }

    state.first_sorted_pos = state.orig_pos[first_target.y][first_target.x];
    move_to_correct_pos(first_target, state);

    State best_state, tmp_state;
    while(true) {
        auto transition_types = get_all_possible_transition_types(state);
        if (transition_types.empty()) break;
        double min_cost = 1e20;
        for(const auto& transition_type : transition_types) {
            tmp_state = state;
            transition(transition_type, tmp_state);
            double cost = calc_cost(tmp_state);
            if (min_cost > cost) {
                min_cost = cost;
                best_state = tmp_state;
            }
        }
        state = best_state;
    }

    sort_last_2x2(state, max(2, selectable_times - (int)state.proc.size()), true);
}
void StrictSorter::sort_partially(const Pos& first_selected_pos, const Pos& UL, const Pos& dp, State& state) {

    State orig_state = state;

    // UL~UL+dpの範囲内が元座標の奴らが含まれるような最小の四角形を探索しそれ以外をsortedで埋める
    Pos exUL(0,0), exdp(div_num);
    array<array<Pos, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> rev_now_orig_pos{};
    {
        array<bool, MAX_DIV_NUM_Y> exists_within_range_y{};
        array<bool, MAX_DIV_NUM_X> exists_within_range_x{};

        rep(i,div_num.y) rep(j,div_num.x) {
            const Pos p = state.orig_pos[i][j];
            rev_now_orig_pos[p.y][p.x] = Vec2(j,i);
        }

        rep(dy, dp.y) rep(dx, dp.x) {
            const Pos tmp_dp(dx, dy);
            const Pos p = UL.get_moved_toraly_pos(tmp_dp, div_num);
            const Pos pos = rev_now_orig_pos[p.y][p.x];
            exists_within_range_x[p.x] = true;
            exists_within_range_y[p.y] = true;
            exists_within_range_x[pos.x] = true;
            exists_within_range_y[pos.y] = true;
        }
        rep(i,div_num.x) if (!exists_within_range_x[i]) {
            rep(space,div_num.x) {
                const int j = (i+space+1) % div_num.x;
                if (exists_within_range_x[j]) {
                    if (div_num.x-exdp.x < space+1) {
                        exUL.x = j;
                        exdp.x = div_num.x-(space+1);
                    }
                    break;
                }
            }
        }
        rep(i,div_num.y) if (!exists_within_range_y[i]) {
            rep(space,div_num.y) {
                const int j = (i+space+1) % div_num.y;
                if (exists_within_range_y[j]) {
                    if (div_num.y-exdp.y < space+1) {
                        exUL.y = j;
                        exdp.y = div_num.y-(space+1);
                    }
                    break;
                }
            }
        }
        rep(i,div_num.y) rep(j,div_num.x) state.board_state[i][j] = State::sorted;
        rep(dy, exdp.y) rep(dx, exdp.x) {
            const Pos tmp_dp(dx, dy);
            const Pos p = exUL.get_moved_toraly_pos(tmp_dp, div_num);
            state.board_state[p.y][p.x] = State::ordinary;
        }
    }

    // state.now_orig_posを操作しやすいように改変する
    // 具体的にはUL~UL+dpの範囲内にあるorig_posが全てUL~UL+dpの座標に収まるようにする
    vector<pair<Pos, Pos>> replace, replace_log;
    {
        array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> within_range{};
        array<array<bool, MAX_DIV_NUM_X>, MAX_DIV_NUM_Y> targeted{};
        rep(dy, exdp.y) rep(dx, exdp.x) {
            const Pos tmp_dp(dx, dy);
            const Pos pos = exUL.get_moved_toraly_pos(tmp_dp, div_num);
            const Pos orig_pos = state.orig_pos[pos.y][pos.x];
            within_range[pos.y][pos.x] = true;
            targeted[orig_pos.y][orig_pos.x] = true;
        }
        vector<Pos> not_within_range, not_targeted;
        rep(dy, exdp.y) rep(dx, exdp.x) {
            const Pos tmp_dp(dx, dy);
            const Pos pos = exUL.get_moved_toraly_pos(tmp_dp, div_num);
            const Pos orig_pos = state.orig_pos[pos.y][pos.x];
            if (!targeted[pos.y][pos.x]) not_targeted.push_back(pos);
            if (!within_range[orig_pos.y][orig_pos.x]) not_within_range.push_back(pos);
        }

        vector<ComparableData<double, pair<Pos, Pos>>> v;
        for(const auto& p1 : not_within_range) {
            for(const auto& p2 : not_targeted) {
                int mh_dist = p1.calc_mh_dist_toraly(p2, div_num);
                v.emplace_back(mh_dist*mh_dist, make_pair(p1,p2));
            }
        }
        sort(all(v));
        for(const auto& x : v) {
            const auto& p1 = x.data.first;
            const auto& p2 = x.data.second;
            const auto orig_p1 = state.orig_pos[p1.y][p1.x];
            if (!within_range[orig_p1.y][orig_p1.x] && !targeted[p2.y][p2.x]) {
                replace.emplace_back(p1, p2);
                targeted[p2.y][p2.x] = true;
                within_range[orig_p1.y][orig_p1.x] = true;
            }
        }
    }
    for(const auto& [p1, p2] : replace) {
        Pos p = rev_now_orig_pos[p2.y][p2.x];
        replace_log.emplace_back(p, state.orig_pos[p.y][p.x]);
        replace_log.emplace_back(p2, state.orig_pos[p1.y][p1.x]);
        state.orig_pos[p.y][p.x] = state.orig_pos[p1.y][p1.x];
        state.orig_pos[p1.y][p1.x] = p2;
    }

    if (state.selected_pos != first_selected_pos) state.select(first_selected_pos);
    state.first_sorted_pos = exUL.get_moved_toraly_pos(exdp, div_num);
    if (state.orig_pos[state.selected_pos.y][state.selected_pos.x].x == state.first_sorted_pos.x) state.first_sorted_pos.move_toraly(Direction::R, div_num);
    if (state.orig_pos[state.selected_pos.y][state.selected_pos.x].y == state.first_sorted_pos.y) state.first_sorted_pos.move_toraly(Direction::D, div_num);
    if (state.board_state[state.first_sorted_pos.y][state.first_sorted_pos.x] != State::sorted) move_to_correct_pos(rev_now_orig_pos[state.first_sorted_pos.y][state.first_sorted_pos.x], state);

    {
        //            State best_state;
        State best_state, tmp_state;
        while(true) {

            auto transition_types = get_all_possible_transition_types(state);
            if (transition_types.empty()) break;

            // target == destinationのやつがあれば遷移
            {
                bool transitioned = false;
                for(const auto& transition_type : transition_types) {
                    if (!transition_type.has_buddy) {
                        const auto& target = transition_type.target;
                        const auto& destination = state.orig_pos[target.y][target.x];
                        if (target == destination) {
                            //                            transition(transition_type, state);
                            state.board_state[destination.y][destination.x] = State::sorted;
                            transitioned = true;
                            break;
                        }
                    }
                }
                if (transitioned) continue;
            }

            // costが最も小さい次のstateを探す
            double min_cost = 1e20;
            for(const auto& transition_type : transition_types) {
                tmp_state = state;
                tmp_state = state;
                transition(transition_type, tmp_state);
                double cost = calc_cost(tmp_state);
                if (min_cost > cost) {
                    min_cost = cost;
                    best_state = tmp_state;
                }
            }
            state = best_state;
        }
    }

    sort_last_2x2(state, 2);

    // 変更したところを戻していく
    rep(i,div_num.y) rep(j,div_num.x) state.board_state[i][j] = orig_state.board_state[i][j];
    rep(dy, exdp.y) rep(dx, exdp.x) {
        const Pos tmp_dp(dx, dy);
        const Pos pos = exUL.get_moved_toraly_pos(tmp_dp, div_num);
        state.board_state[pos.y][pos.x] = State::sorted;
    }
    for(const auto& [p1, p2] : replace_log) {
        state.orig_pos[p1.y][p1.x] = p2;
        state.board_state[p1.y][p1.x] = State::ordinary;
    }
}

bool StrictSorter::sort_by_roughly_sort(State& state) {
    while(true) {
        const auto p = RoughSorter::get_max_penalty_pos(state);
        if (!p) break;
        if ((int)state.proc.size() >= selectable_times) return false;
        state.select(p.value());
        RoughSorter::sort_roughly_greedily(state);
        if (state.proc.back().path.empty()) return false;
    }
    optimize_procedures(state.proc);
    return true;
}
