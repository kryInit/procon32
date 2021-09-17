#include <procedures.hpp>

SingleProcedure::SingleProcedure() = default;
SingleProcedure::SingleProcedure(Pos selected_pos, Path path) : selected_pos(selected_pos), path(path) {}

std::ostream& operator << (std::ostream& os, const SingleProcedure& sp) {
    os << std::hex << std::uppercase << sp.selected_pos.x << sp.selected_pos.y << std::endl;
    os << std::dec << sp.path.size() << std::endl;
    for(const auto &dir : sp.path) os << dir;
    return os;
}
std::ostream& operator << (std::ostream& os, const Procedures& procs) {
    os << procs.size() << std::endl;
    for(size_t i = 0; i < procs.size(); ++i) {
        os << procs[i];
        if (i+1 != procs.size()) os << std::endl;
    }
    return os;
}
