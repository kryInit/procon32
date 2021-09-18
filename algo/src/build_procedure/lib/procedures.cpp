#include <procedures.hpp>

using namespace std;

SingleProcedure::SingleProcedure() = default;
SingleProcedure::SingleProcedure(Pos selected_pos, Path path) : selected_pos(selected_pos), path(path) {}

Procedures input_procedure(const string& path) {
    ifstream ifs(path);
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
