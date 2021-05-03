#include <bits/stdc++.h>
#include <vec.hpp>
#include <time_manager.hpp>

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

template<class T>
using rev_priority_queue = priority_queue<T, vector<T>, greater<T>>;

enum class Direction { U, R, D, L };
std::ostream& operator << (std::ostream& os, const Direction& dir) {
    if (dir == Direction::U) os << "U";
    else if (dir == Direction::R) os << "R";
    else if (dir == Direction::D) os << "D";
    else if (dir == Direction::L) os << "L";
    return os;
}

class Settings {
    unsigned int frag_size{};
    Vec2<unsigned int> div_num;
    unsigned int selectable_times{};
    unsigned int choice_cost{}, repl_cost{};

public:
    void load(const string& path) {
        Vec2<unsigned int> pix_num;
        ifstream ifs(path);
        ifs >> div_num.x >> div_num.y;
        ifs >> selectable_times >> choice_cost >> repl_cost;
        ifs >> pix_num.x >> pix_num.y;
        if (pix_num.y * div_num.x != pix_num.x * div_num.y) {
            cerr << "h_pix / h_div != w_pix / w_div" << endl;
            exit(-1);
        }
        frag_size = pix_num.y / div_num.y;
    }
    void dump() {
        constexpr unsigned int w = 20;
        cout << left << setw(w) << "frag size" << ": " << frag_size << endl;
        cout << left << setw(w) << "div num"  << ": " << div_num << endl;
        cout << left << setw(w) << "selectable times"  << ": " << selectable_times << endl;
        cout << left << setw(w) << "choice cost"  << ": " << choice_cost << endl;
        cout << left << setw(w) << "replacement cost"  << ": " << repl_cost << endl;
    }
    [[nodiscard]] const unsigned int& FRAG_SIZE() const { return frag_size; }
    [[nodiscard]] const Vec2<unsigned int>& DIV_NUM() const { return div_num; }
    [[nodiscard]] const unsigned int& SELECTABLE_TIMES() const { return selectable_times; }
    [[nodiscard]] const unsigned int& CHOICE_COST() const { return choice_cost; }
    [[nodiscard]] const unsigned int& REPL_COST() const { return repl_cost; }

};

class ImageFragment {
    unsigned int FRAG_SIZE;
    RGB **img_data;

public:
    ImageFragment() : FRAG_SIZE(0), img_data(nullptr) {}
    ~ImageFragment() {
        if (img_data != nullptr) {
            delete[] *img_data;
            delete[] img_data;
            img_data = nullptr;
        }
    }
    [[nodiscard]] const unsigned int& FRAGMENT_SIZE() const { return FRAG_SIZE; }
    void load(const string& frag_path, const Settings& settings) {
        if (img_data != nullptr) {
            delete[] *img_data;
            delete[] img_data;
            img_data = nullptr;
        }

        // allocate memory
        FRAG_SIZE = settings.FRAG_SIZE();
        img_data = new RGB*[FRAG_SIZE];
        img_data[0] = new RGB[FRAG_SIZE*FRAG_SIZE];
        rep(i, FRAG_SIZE) img_data[i] = &img_data[0][i*FRAG_SIZE];

        // load data
        ifstream ifs(frag_path, ios::binary);
        ifs.read((char*)*img_data, FRAG_SIZE*FRAG_SIZE*sizeof(RGB));
    }
    void dump() const {
        rep(i,FRAG_SIZE) {
            cout << "[";
            rep(j,FRAG_SIZE) {
                cout << img_data[i][j] << ", ";
            }
            cout << "]" << endl;
        }
    }
    void pdump() const {
        rep(i,FRAG_SIZE) {
            cout << "[";
            rep(j,FRAG_SIZE) {
                cout << &img_data[i][j] << ", ";
            }
            cout << "]" << endl;
        }
    }
    void get_perimeter(unsigned int size, RGB *perimeter) const {
        // storage: RGB[4][FRAG_SIZE][size]
        /* * * * * * * * *
         * writing order *
         *               *
         *    0 --> 1    *
         *    ^     |    *
         *    |     v    *
         *    3 <-- 2    *
         *               *
         * * * * * * * * */

        const int isize = static_cast<int>(size);
        const int iFRAG_SIZE = static_cast<int>(FRAG_SIZE);
        rep(k,4) rep(i,FRAG_SIZE) rep(j,size) {
            int y,x;
            if (k == 0) y = isize-j-1, x = i;
            else if (k == 1) y = i, x = iFRAG_SIZE-isize + j;
            else if (k == 2) y = iFRAG_SIZE-isize + j, x = iFRAG_SIZE-i-1;
            else  y = iFRAG_SIZE-i-1, x = isize-j-1;
            *(perimeter + k*FRAG_SIZE*size + i*size + j) = img_data[y][x];
        }
    }
};

class Image {
    Vec2<unsigned int> DIV_NUM;
    ImageFragment **img_frags;

public:
    Image() : DIV_NUM(0, 0), img_frags(nullptr) {}
    ~Image() {
        if (img_frags != nullptr) {
            delete[] *img_frags;
            delete[] img_frags;
            img_frags = nullptr;
        }
    }

    void load(const string& frags_dir_path, const Settings& settings) {
        if (img_frags != nullptr) {
            delete[] *img_frags;
            delete[] img_frags;
            img_frags = nullptr;
        }

        // allocate memory
        DIV_NUM = settings.DIV_NUM();
        img_frags = new ImageFragment*[DIV_NUM.y];
        img_frags[0] = new ImageFragment[DIV_NUM.y*DIV_NUM.x];
        rep(i, DIV_NUM.y) img_frags[i] = &img_frags[0][i*DIV_NUM.x];

        // load data
        rep(y, DIV_NUM.y) rep(x, DIV_NUM.x) {
//            cout << y << " " << x << endl;
            stringstream ss;
            ss << frags_dir_path << hex << uppercase << x << y << ".ppm.bin";
            img_frags[y][x].load(ss.str(), settings);
        }
    }
    void dump() const {
        rep(i, DIV_NUM.y) rep(j,DIV_NUM.x) {
            cout << "img_flags[" << i << "][" << j << "]: " << endl;
            img_frags[i][j].dump();
            cout << endl;
        }
    }

    void dump_perimeter(Vec2<int> idx) const {
        unsigned int size = 3;
        const unsigned int FRAG_SIZE = img_frags[0][0].FRAGMENT_SIZE();
        RGB *tmp = new RGB[4*FRAG_SIZE*size];
        img_frags[idx.y][idx.x].get_perimeter(size, tmp);

        rep(k,4) {
            cout << "k: " << k << endl;
            rep(i,FRAG_SIZE) {
                rep(j,size) cout << *(tmp + k*FRAG_SIZE*size + i*size + j) << ", ";
                cout << endl;
            }
            cout << endl;
        }

        delete[] tmp;
    }
    void get_all_perimeter(unsigned int size, RGB *perimeters) const {
        unsigned int offset = 4*img_frags[0][0].FRAGMENT_SIZE()*size;
        rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) {
            img_frags[i][j].get_perimeter(size, perimeters + (i*DIV_NUM.x + j)*offset);
        }
    }
};

constexpr unsigned int MAX_DIV_NUM = 16;

struct ImageFragmentState {
    unsigned char orig_idx, rotation_times;

    unsigned int get_side_idx(Direction dir) const {
        unsigned int result = static_cast<unsigned char>(orig_idx)*4;
//        result += (4+static_cast<unsigned int>(dir) - rotation_times)%4;
        if (dir == Direction::U) result += (4-rotation_times)%4;
        else if (dir == Direction::R) result += (5-rotation_times)%4;
        else if (dir == Direction::D) result += (6-rotation_times)%4;
        else /* if (dir == Direction::L) */ result += (3-rotation_times);
        return result;
    }
    ImageFragmentState() : orig_idx(), rotation_times() {}
    ImageFragmentState(unsigned char _orig_idx, unsigned char _rotation_times) : orig_idx(_orig_idx), rotation_times(_rotation_times) {}
};

struct ImageState {
    Vec2<unsigned int> now_size;
    array<array<ImageFragmentState, MAX_DIV_NUM>, MAX_DIV_NUM> states;
    bitset<MAX_DIV_NUM*MAX_DIV_NUM> placed;

    void rotate_180deg() {

    }
    void place(Vec2<unsigned int> pos, unsigned char orig_idx, unsigned char rotation_times) {
        states[pos.y][pos.x].orig_idx = orig_idx;
        states[pos.y][pos.x].rotation_times = rotation_times;
        placed[orig_idx] = true;
        now_size.x = max(now_size.x, pos.x+1);
        now_size.y = max(now_size.y, pos.y+1);
    }

    vector<ImageFragmentState> get_side(Direction dir) const {

    }

    ImageState() : now_size{}, orig_idx{}, placed{} {}
};

class IAdjacencyEvaluator {
public:
    virtual void get_adjacency(const Image& img, const Settings& settings, double *adjacency) = 0;
};

class ISolver {
public:
    virtual ImageState solve(double *adjacency, const Settings& settings) = 0;
};

class SimpleAdjacencyEvaluator : IAdjacencyEvaluator {
public:
    void get_adjacency(const Image& img, const Settings& settings, double *adjacency) override {

        constexpr unsigned int size = 1;
        auto frag_size = settings.FRAG_SIZE();
        auto div_num = settings.DIV_NUM();
        RGB *perimeters = new RGB[div_num.y*div_num.x*4*frag_size];
        img.get_all_perimeter(size, perimeters);

        auto *memo = new Vec3<double>[frag_size];
        double *now_ptr = adjacency;
        rep(i, div_num.y) rep(j, div_num.x) rep(k, 4) {
            {
                const unsigned int offset = (i*div_num.x*4+j*4+k)*frag_size;
                rep(y, frag_size) {
                    memo[frag_size-y-1] = *(perimeters+offset+y);
                }
            }
            rep(I, div_num.y) rep(J, div_num.x) {
                if (i == I && j == J) {
                    rep(K,4) {
                        *now_ptr = INFINITY;
                        now_ptr++;
                    }
                    continue;
                }
                rep(K,4) {
                    const unsigned int offset = (I*div_num.x*4+J*4+K)*frag_size;
                    double var_sum = 0;
                    rep(y,frag_size) {
                        Vec3<double> tmp = static_cast<Vec3<double>>(*(perimeters+offset+y)) - memo[y];
                        var_sum += tmp.mul_each_other(tmp).sum() / 3. / 2.;
                    }
                    double var_ave = var_sum / frag_size;
                    *now_ptr = sqrt(var_ave);
                    now_ptr++;
                }
            }
        }

        delete[] perimeters;
        delete[] memo;
        cout << "simple adjacency evaluator" << endl;
   }
};

class SimpleSolver : ISolver {
public:
    static void dump_adjacency_info(const double *adjacency, const Settings& settings) {
        auto div_num = settings.DIV_NUM();
        auto frag_num = div_num.y*div_num.x;
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                rep(k,4) {
                    const unsigned int offset = (i*div_num.x*4+j*4+k)*frag_num*4;
                    double best_adjacency = 1e10;
                    int best_i=-1, best_j=-1, best_k=-1;
                    rep(I,div_num.y) {
                        rep(J,div_num.x) {
                            if (I == i && J == j) continue;
                            rep(K,4) {
                                double tmp = *(adjacency+offset + I*div_num.x*4 + J*4 + K);
                                if (best_adjacency > tmp) {
                                    best_adjacency = tmp;
                                    best_i = I;
                                    best_j = J;
                                    best_k = K;
                                }
                            }
                        }
                    }
                    cout << "(" << j << ", " << i << ", " << k <<  ") <=> (" << best_j << ", " << best_i << ", " << best_k << ") : " << best_adjacency << endl;
                }
            }
        }
        const unsigned int offset = (2*div_num.x*4 + 1*4 + 3)*frag_num*4;
//        const unsigned int offset = 0;
        rep(i,div_num.y) {
            rep(j,div_num.x) {
                cout << "(";
                rep(k,4) cout << setw(2) << *(adjacency + offset + i*div_num.x*4 + j*4+k) << ", ";
                cout << "), ";
            }
            cout << endl;
        }
        cout << endl;
    }

    ImageState solve(double *adjacency, const Settings& settings) override {
//        dump_adjacency_info(adjacency, settings);
        cout << "size: " << sizeof(ImageState) << endl;
        cout << sizeof(unsigned char) << endl;
        const Vec2<unsigned int> DIV_NUM = settings.DIV_NUM();
        const unsigned int N = DIV_NUM.y*DIV_NUM.x*4;

        struct adj_idx {
            float adjacency;
            array<unsigned short, 2> idx;

            adj_idx() : adjacency(), idx() {}
            adj_idx(float _adjacency, unsigned short idx0, unsigned short idx1) : adjacency(_adjacency), idx{idx0, idx1} {}
            bool operator<(const adj_idx& another) const {
                return adjacency < another.adjacency;
            }
            bool operator>(const adj_idx& another) const {
                return another.adjacency < adjacency;
            }
        };


        // make ordered adjacency
        vector<vector<adj_idx>> ordered_adjacency(N, vector<adj_idx>(N));
        rep(idx, N) {
            repo(IDX, idx, N) {
                const auto adj = static_cast<float>(*(adjacency + idx*N + IDX));
                ordered_adjacency[idx][IDX] = adj_idx(adj, idx, IDX);
                ordered_adjacency[IDX][idx] = adj_idx(adj, IDX, idx);
            }
            sort(ordered_adjacency[idx].begin(), ordered_adjacency[idx].end());
        }
/*
        rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) rep(k,4) {
            adj_idx tmp = ordered_adjacency[i*DIV_NUM.x*4 + j*4 + k].front();
            cout << '(' << j << ", " << i << ", " << k << ") <=> ";
            cout << '(' << tmp.idx[1]/4 % DIV_NUM.x << ", " << tmp.idx[1]/4 / DIV_NUM.y << ", " << (int)tmp.idx[1]%4 << ") : " << tmp.adjacency << endl;
        }
*/

        constexpr unsigned int MAX_STATE_NUM = 8192;
        vector<ImageState> now_states;
//        now_states.reserve(MAX_STATE_NUM);
        // make initial state (two image fragment)
        {
            rev_priority_queue<adj_idx> pq;
            rep(i,N) repo(j,i+1,N) if (ordered_adjacency[i][j].adjacency != INFINITY) pq.push(ordered_adjacency[i][j]);
            rep(i, MAX_STATE_NUM) {
                adj_idx tmp = pq.top(); pq.pop();
                ImageState state;
                {
                    const unsigned int idx = tmp.idx[0] / 4, rotation_times = (5-tmp.idx[0]%4)%4;
                    state.place(Vec2<unsigned int>(0,0), idx, rotation_times);
                }
                {
                    const unsigned int idx = tmp.idx[1] / 4, rotation_times = (3-tmp.idx[1]%4);
                    state.place(Vec2<unsigned int>(0, 1), idx, rotation_times);
                }
                now_states.push_back(state);
                if (pq.empty()) break;
            }
        }

        // beam search
        const unsigned int STEP_NUM = DIV_NUM.y + DIV_NUM.x - 3;
        rep(_, STEP_NUM) {
            vector<ImageState> next_states;
            rev_priority_queue<adj_idx> pq;
            rep(i, min(MAX_STATE_NUM, (unsigned int)now_states.size())) {
                const ImageState& tmp_state = now_states[i];
                const Vec2<unsigned int>& now_size = tmp_state.now_size;
                rep(j,4) {
                    vector<ImageFragmentState> side = tmp_state.get_side(static_cast<Direction>(j));

                    rep(k, side.size()) {
                        // calc diff adjacency
                        float diff_adjacency = 0;
                        {
                            auto tmp_placed = tmp_state.placed;
//                            unsigned int first_adjacent_idx = side[k].orig_idx*4 + (6-side[k].rotation_times)%4;
                            unsigned int first_adjacent_idx = side[k].get_side_idx(Direction::D);
                            unsigned int best_first_idx = 0;
                            rep(l, N) {
                                if (tmp_placed[ordered_adjacency[first_adjacent_idx][l].idx[0]]/4) {
                                    best_first_idx = ordered_adjacency[first_adjacent_idx][l].idx[0];
                                    break;
                                }
                            }
                            tmp_placed[best_first_idx/4] = true;
                            unsigned int tmp_idx = best_first_idx;
                            for(int l=k-1; l>=0; --l) {
                                float min_adjacency = 1e20;
                                unsigned int best_idx = -1;

                                const unsigned int u_idx = side[l].get_side_idx(Direction::D);
                                const unsigned int r_idx = tmp_idx;
                                rep(I,N) {
                                    if (tmp_placed[I/4]) {
                                        I += 3;
                                        break;
                                    }

                                }
                                tmp_idx = best_idx;
                            }
                            for (int l=k+1; l<side.size(); ++l) {

                            }
                        }

                        pq.emplace(diff_adjacency, i, j*MAX_DIV_NUM+k);
                    }
                }
            }
            next_states.swap(now_states);
        }


        cout << "hi" << endl;
        cout << sizeof(unsigned char) << endl;
        cout << sizeof(float) << endl;
        cout << sizeof(adj_idx) << endl;
        cout << sizeof(pair<unsigned char, float>) << endl;
        return ImageState();
    }
};

template<class ADJACENCY_EVALUATOR, class SOLVER>
ImageState solver(const Image& img, const Settings& settings) {
    // img : RGB[DIV_NUM.y][DIV_NUM.x][FRAG_SIZE][FRAG_SIZE]
    // adjacency : double[DIV_NUM.y][DIV_NUM.x][4][DIV_NUM.y][DIV_NUM.x][4]
    // ans : ImageFragmentState[DIV_NUM.y][DIV_NUM.x]

    stopwatch sw;

    ADJACENCY_EVALUATOR adjacency_evaluator;
    SOLVER solver;

    const unsigned int N = settings.DIV_NUM().x*settings.DIV_NUM().y*4;

    sw.start();
    auto *adjacency = new double[N*N];
    adjacency_evaluator.get_adjacency(img, settings, adjacency);
    sw.print_ms();
    sw.print_sec();

    sw.start();
    ImageState ans = solver.solve(adjacency, settings);
    cout << "end solver" << endl;
    sw.print_ms();
    sw.print_sec();

    delete[] adjacency;
    return ans;
}

int main(int argc, char *argv[]) {

#pragma omp parallel
    {
        std::cout << "Hello World!\n";
    }
    string frag_dir_path = string(argv[1]) + "/frags/";
    string settings_path = string(argv[1]) + "/prob.txt";
    ofstream ofs(frag_dir_path + "tmp.txt");
    ofs << "hi" << endl;

    Settings settings;
    settings.load(settings_path);
    settings.dump();

    Image img;
    img.load(frag_dir_path, settings);
    cout << "finish load" << endl;
//    img.dump();
//    img.dump_perimeter(Vec2<int>(0,0));
    solver<SimpleAdjacencyEvaluator, SimpleSolver>(img, settings);
}
