#include <procedures.hpp>

using namespace std;

SingleProcedure::SingleProcedure() = default;
SingleProcedure::SingleProcedure(Pos selected_pos, Path path) : selected_pos(selected_pos), path(path) {}

Procedures input_procedure(const string& path) {
    ifstream ifs(path);
    if (!ifs) {
        cerr << "[input procedure] load failed" << endl;
        exit(-1);
    }

    {
        string rotations;
        ifs >> rotations;
    }

    Procedures procs;
    int n;
    ifs >> n;
    while(n--) {
        string s;
        ifs >> s;
        int idx = stoi(s, nullptr, 16);
        int x = idx / 16, y = idx % 16;
        int m;
        string spath;
        ifs >> m >> spath;
        procs.emplace_back(Pos(x,y), Path::from_string(spath));
    }
    return procs;
}

void reduce_path(Path& path) {
    Path tmp_path; tmp_path.reserve(path.size());
    for(const auto& dir : path) {
        if (tmp_path.empty()) tmp_path.push_back(dir);
        else if (dir != tmp_path.back().get_dir_reversed()) tmp_path.push_back(dir);
        else tmp_path.pop_back();
    }
    path = tmp_path;
}
void optimize_procedures(Procedures& procs) {
    Procedures tmp_procs; tmp_procs.reserve(procs.size());
    for(auto& proc : procs) {
        reduce_path(proc.path);
        if (!proc.path.empty()) tmp_procs.push_back(proc);
    }
    procs = tmp_procs;
}



ostream& operator << (ostream& os, const SingleProcedure& sp) {
    os << hex << uppercase << sp.selected_pos.x << sp.selected_pos.y << endl;
    os << dec << sp.path.size() << endl;
    for(const auto &dir : sp.path) os << dir;
    return os;
}
ostream& operator << (ostream& os, const Procedures& procs) {
    os << procs.size() << endl;
    for(size_t i = 0; i < procs.size(); ++i) {
        os << procs[i];
        if (i+1 != procs.size()) os << endl;
    }
    return os;
}
