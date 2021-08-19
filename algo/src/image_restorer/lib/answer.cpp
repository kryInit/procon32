#include "answer.hpp"

Answer::Answer() : rotations(), orig_idx() {}

std::ostream& operator << (std::ostream& os, const Answer& ans) {
    for(const auto& rotation : ans.rotations) os << rotation;
    os << std::endl;
    for(size_t i=0; i<ans.orig_idx.size(); ++i) {
        for(size_t j=0; j<ans.orig_idx[i].size(); ++j) {
            os << std::hex << std::uppercase << ans.orig_idx[i][j].x << ans.orig_idx[i][j].y;
            if (j+1 != ans.orig_idx[i].size()) os << " ";
        }
        if (i+1 != ans.orig_idx.size()) os << std::endl;
    }
    return os;
}
