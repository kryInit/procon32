#include "answer.hpp"

void Answer::dump(std::ostream& os) {
    for(const auto& rotation : rotations) os << rotation;
    os << std::endl;
    for(const auto & i : orig_idx) {
        for(int j=0; j<i.size(); ++j) {
            os << std::hex << std::uppercase << i[j].x << i[j].y;
            if (j+1 != i.size()) os << " ";
        }
        os << std::endl;
    }
}
void Answer::dump() {
    dump(std::cout);
}

Answer::Answer() : rotations(), orig_idx() {}
