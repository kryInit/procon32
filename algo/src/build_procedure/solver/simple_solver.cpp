#include "simple_solver.hpp"
#include <bits/stdc++.h>
#include <utility.hpp>
#include <comparable_data.hpp>
#include <macro.hpp>

using namespace std;

/*
 * rotations
 * choice num
 *
 * select img pos   \
 * swap num         | x choice num
 * procedure        /
 *
 */

void join_path(Path &p1, const Path& p2) { p1.insert(p1.end(), p2.begin(), p2.end()); }

void loop_move(Pos &pos, Direction dir, const Vec2<int> div_num) {
    pos.move(dir);
    pos.x = (pos.x+div_num.x) % div_num.x;
    pos.y = (pos.y+div_num.y) % div_num.y;
}

Pos get_pos(const Pos orig_pos, const OriginalPositions& now_orig_pos, const Vec2<int> div_num) {
    rep(i,div_num.y) rep(j,div_num.x) if (now_orig_pos[i][j] == orig_pos) return {j,i};
    EPRINT("[get_pos]", orig_pos);
    exit(-1);
}

Path calc_path(const Pos s, const Pos g, const Vec2<int> border, const vector<Pos> &exclusions, const Vec2<int> div_num) {
    auto keep_out = Utility::gen_vec(false, div_num.y, div_num.x);
    auto prev_dir = Utility::gen_vec<Direction>(Direction::U, div_num.y, div_num.x);
    for(const auto &pos : exclusions) keep_out[pos.y][pos.x] = true;
    repr(i, border.y, div_num.y) rep(j,div_num.x) keep_out[i][j] = true;
    repr(j, border.x, div_num.x) rep(i, div_num.y) keep_out[i][j] = true;
    if (keep_out[s.y][s.x] || keep_out[g.y][g.x]) {
        Utility::exit_with_message(Utility::concat("[calc_path] keep_out[s.y][s.x], keep_out[g.y]g.x]: ",keep_out[s.y][s.x],", ",keep_out[g.y][g.x]));
    }

    // いつもの
    Utility::reverse_priority_queue<ComparableData<int, pair<Pos, Direction>>> pq;
    pq.emplace(0, make_pair(s, Direction::U));
    while(!pq.empty()) {
        int dist = pq.top().rating;
        Pos pos = pq.top().data.first;
        Direction prev_move_dir = pq.top().data.second;
        pq.pop();

        int y = pos.y, x = pos.x;

        if (keep_out[y][x]) continue;
        keep_out[y][x] = true;
        prev_dir[y][x] = prev_move_dir;
        if (pos == g) break;

        Direction dir = Direction::U;
        rep(i,4) {
            Pos tmp_pos = pos;
            loop_move(tmp_pos, dir, div_num);
            int X = (tmp_pos.x + div_num.x) % div_num.x;
            int Y = (tmp_pos.y + div_num.y) % div_num.y;
            pq.emplace(dist+1, make_pair(Pos(X,Y), dir));
            dir.rotate_cw();
        }
    }

    if (!keep_out[g.y][g.x]) {
        // 例外処理に対応するためにからのpathを返している
        return {};
    }

    // 経路復元
    Path rev_path, path;
    Pos now_pos = g;
    while(now_pos != s) {
        Direction dir = prev_dir[now_pos.y][now_pos.x];
        rev_path.push_back(dir);
        loop_move(now_pos, dir.get_dir_reversed(), div_num);
    }
    revrep(i,rev_path.size()) path.push_back(rev_path[i]);
    return path;
}

void loop_move(Pos &selected_pos, const Path &path, OriginalPositions &now_orig_pos, const Vec2<int> div_num) {
    Pos current = selected_pos;
    Pos tmp = now_orig_pos[current.y][current.x];
    for(const auto dir : path) {
        Pos next = current;
        loop_move(next, dir, div_num);
        now_orig_pos[current.y][current.x] = now_orig_pos[next.y][next.x];
        current = next;
    }
    now_orig_pos[current.y][current.x] = tmp;
    selected_pos = current;
}

Path move_frag_by_selected_frag(Pos &selected_pos, Pos target, Pos destination, OriginalPositions &now_orig_pos, const Vec2<int> border, vector<Pos> &exclusions, Vec2<int> div_num) {
    Path path;
    Path targets_path = calc_path(target, destination, border, exclusions, div_num);
    for (const auto &dir : targets_path) {
        Pos next = target;
        loop_move(next, dir, div_num);

        exclusions.push_back(target);
        Path tmp_path = calc_path(selected_pos, next, border, exclusions, div_num);
        exclusions.pop_back();
        if (selected_pos != next && tmp_path.empty()) break;
        tmp_path.push_back(dir.get_dir_reversed());

        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
        target = next;
    }
    return path;
}

/*
 * align outermost columnの場合について述べる
 * 基本的な考えは上の二つ以外を揃えた後上の二つを頑張って揃える
 * 上の二つ以外は例外なく処理できるはず
 * 上の二つに関しては以下の通りに揃える
 *  ------    ------    ------
 *     s | =>    t0| =>    t0|
 *  t1 t0|    t1  s|    s  t1|
 *
 * ただし、上の二つを揃える際に以下の状況になって停止してしまう場合がある
 * ------
 * t1 s |
 *    t0|
 * 適当に対応させた
 *
*/

Path align_outermost_row(OriginalPositions &now_orig_pos, Vec2<int> border, Pos &selected_pos, Vec2<int> div_num) {
    Path path;
    vector<Pos> exclusions;
    for (int j=border.x-1; j>=2; --j) {
        Pos destination = Vec2(j, border.y-1);
        Pos target = get_pos(destination, now_orig_pos, div_num);
        join_path(path, move_frag_by_selected_frag(selected_pos, target, destination, now_orig_pos, border, exclusions, div_num));
        exclusions.push_back(destination);
    }
    rep(j,2) {
        Pos destination = Vec2(1, border.y-1-j);
        Pos target = get_pos(Vec2(j, border.y-1), now_orig_pos, div_num);
        join_path(path, move_frag_by_selected_frag(selected_pos, target, destination, now_orig_pos, border, exclusions, div_num));
        exclusions.push_back(destination);
    }
    if (selected_pos.y == border.y-1 && selected_pos.x == 0 && now_orig_pos[border.y-2][0] == Vec2(1, border.y-1)) {
        // RUULDRDLUURDLDRU
        Path tmp_path = Path::from_string("RUULDRDLUURDLDRU");
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
    } else {
        Path tmp_path = calc_path(selected_pos, Vec2(0, border.y-1), border, exclusions, div_num);
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
        tmp_path.clear(); tmp_path.push_back(Direction::R); tmp_path.push_back(Direction::U);
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
    }
    return path;
}

Path align_outermost_column(OriginalPositions &now_orig_pos, Vec2<int> border, Pos &selected_pos, Vec2<int> div_num) {
    Path path;
    vector<Pos> exclusions;
    for (int i=border.y-1; i>=2; --i) {
        Pos destination = Vec2(border.x-1, i);
        Pos target = get_pos(destination, now_orig_pos, div_num);
        join_path(path, move_frag_by_selected_frag(selected_pos, target, destination, now_orig_pos, border, exclusions, div_num));
        exclusions.push_back(destination);
    }
    rep(i,2) {
        Pos destination = Vec2(border.x-1-i, 1);
        Pos target = get_pos(Vec2(border.x-1, i), now_orig_pos, div_num);
        join_path(path, move_frag_by_selected_frag(selected_pos, target, destination, now_orig_pos, border, exclusions, div_num));
        exclusions.push_back(destination);
    }
    if (selected_pos.x == border.x-1 && selected_pos.y == 0 && now_orig_pos[0][border.x-2] == Vec2(border.x-1,1)) {
        // DLLURDRULLDRURDL
        Path tmp_path = Path::from_string("DLLURDRULLDRURDL");
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
    } else {
        Path tmp_path = calc_path(selected_pos, Vec2(border.x-1, 0), border, exclusions, div_num);
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
        tmp_path.clear(); tmp_path.push_back(Direction::D); tmp_path.push_back(Direction::L);
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
    }
    return path;
}

SingleProcedure get_first_procedure(OriginalPositions &now_orig_pos, Vec2<int> div_num) {
    SingleProcedure proc;
    proc.selected_pos = get_pos(Pos(0, 0), now_orig_pos, div_num);

    Path path;
    Vec2<int> border = div_num;
    Pos selected_pos = proc.selected_pos;

    while(border.x > 2 || border.y > 2) {
        if (border.x >= border.y) { join_path(path, align_outermost_column(now_orig_pos, border, selected_pos, div_num)); border.x--; }
        else { join_path(path, align_outermost_row(now_orig_pos, border, selected_pos, div_num)); border.y--; }
    }
    {
        vector<Pos> exclusions;
        Pos destination = Vec2(1, 0);
        Pos target = get_pos(destination, now_orig_pos, div_num);
        join_path(path, move_frag_by_selected_frag(selected_pos, target, destination, now_orig_pos, border, exclusions, div_num));

        Path tmp_path = calc_path(selected_pos, Vec2(0,0), border, exclusions, div_num);
        loop_move(selected_pos, tmp_path, now_orig_pos, div_num);
        join_path(path, tmp_path);
    }
    proc.path = path;
    return proc;
}

void check_ans(OriginalPositions initial_orig_pos, Procedures &procs, Vec2<int> div_num) {
    auto& now_orig_pos = initial_orig_pos;
    for(const auto &proc : procs) {
        Pos current = proc.selected_pos;
        Pos tmp = now_orig_pos[current.y][current.x];
        for(const auto dir : proc.path) {
            Pos next = current; loop_move(next, dir, div_num);
            now_orig_pos[current.y][current.x] = now_orig_pos[next.y][next.x];
            current = next;
        }
        now_orig_pos[current.y][current.x] = tmp;
    }
    rep(i,div_num.y) rep(j,div_num.x) if (Vec2(j,i) != now_orig_pos[i][j]) {
        EPRINT(i, j, Vec2(j,i), now_orig_pos[i][j]);
        exit(-1);
    }
}

Procedures SimpleSolver::operator()(const OriginalPositions& original_positions, const Settings& settings) {
    // now_orig_pos[i][j] = (x,y): 今の画像で(j,i)の位置にある断片の元座標は(x,y)
    // now_orig_posが並んだら終了

    Vec2<int> div_num = static_cast<Vec2<int>>(settings.div_num);

    Procedures procs;

    OriginalPositions now_orig_pos = original_positions;

    procs.push_back(get_first_procedure(now_orig_pos, div_num));
    if (now_orig_pos[1][0] != Vec2(0,1)) {
        SingleProcedure sproc;
        Path path; path.push_back(Direction::R);
        procs.emplace_back(Vec2(0,1), path);
    }

    check_ans(original_positions, procs, div_num);

    return procs;
}
