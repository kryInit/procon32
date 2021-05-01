#include <bits/stdc++.h>
#include <vec.hpp>
#include <time_manager.hpp>

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)

class Settings {
    unsigned int frag_size;
    Vec2<unsigned int> div_num;
    unsigned int selectable_times;
    unsigned int choice_cost, repl_cost;

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
    const unsigned int& FRAG_SIZE() const { return frag_size; }
    const Vec2<unsigned int>& DIV_NUM() const { return div_num; }
    const unsigned int& SELECTABLE_TIMES() const { return selectable_times; }
    const unsigned int& CHOICE_COST() const { return choice_cost; }
    const unsigned int& REPL_COST() const { return repl_cost; }

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
        }
    }
    const unsigned int& FRAGMENT_SIZE() const { return FRAG_SIZE; }
    void load(const string& frag_path, const Settings& settings) {
        if (img_data != nullptr) {
            delete[] *img_data;
            delete[] img_data;
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
        }
    }

    void load(const string& frags_dir_path, const Settings& settings) {
        if (img_frags != nullptr) {
            delete[] *img_frags;
            delete[] img_frags;
        }

        // allocate memory
        DIV_NUM = settings.DIV_NUM();
        img_frags = new ImageFragment*[DIV_NUM.y];
        img_frags[0] = new ImageFragment[DIV_NUM.y*DIV_NUM.x];
        rep(i, DIV_NUM.y) img_frags[i] = &img_frags[0][i*DIV_NUM.x];

        // load data
        rep(y, DIV_NUM.y) rep(x, DIV_NUM.x) {
            cout << y << " " << x << endl;
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


struct ImageFragmentState {
    unsigned int rotation_times;
    Vec2<unsigned int> orig_coord;

    ImageFragmentState() : rotation_times(0), orig_coord() {}
    ImageFragmentState(unsigned int _rotation_times, Vec2<unsigned int> _orig_coord) : rotation_times(_rotation_times), orig_coord(_orig_coord) {}
};

struct ImageState {
    vector<ImageFragmentState> repl_order;
    vector<bool> placed;

    explicit ImageState() : repl_order(), placed() {}
    explicit ImageState(unsigned int n) : repl_order(), placed(vector<bool>(n, false)) {}
    void set_image(unsigned int idx, ImageFragmentState frag_state) {
        placed[idx] = true;
        repl_order.push_back(frag_state);
    }
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
                    int best_i, best_j, best_k;
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
        dump_adjacency_info(adjacency, settings);

        const auto div_num = settings.DIV_NUM();
        const auto frag_num = div_num.y*div_num.x;

        timeManager tm(2000);
        constexpr unsigned int MAX_DIV_NUM = 16;
        constexpr unsigned int MAX_FRAG_NUM = MAX_DIV_NUM*MAX_DIV_NUM;
        constexpr unsigned int MAX_STORAGE_SIZE = 1000;

        int order_table[MAX_DIV_NUM][MAX_DIV_NUM] = {};
        Vec2<unsigned int> rev_order_table[MAX_FRAG_NUM] = {};

        {
            int now = 0;
            int max_div = max(div_num.x, div_num.y);
            rep(i, max_div) {
                if (i < div_num.x) rep(j, min(i, (int)div_num.y)) {
                    order_table[j][i] = now;
                    rev_order_table[now] = Vec2<unsigned int>(i, j);
                    now++;
                }
                if (i < div_num.y) rep(j,min(i+1, (int)div_num.x)) {
                    order_table[i][j] = now;
                    rev_order_table[now] = Vec2<unsigned int>(j, i);
                    now++;
                }
            }
            rep(i,div_num.y) {
                rep(j,div_num.x) cout << order_table[i][j] << " ";
                cout << endl;
            }
        }

        vector<ImageState> storage[MAX_FRAG_NUM];
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq[MAX_FRAG_NUM];
        rep(i, div_num.y) rep(j,div_num.x) rep(k,4) {
            ImageState state(frag_num);
            state.set_image(i*div_num.x+j, ImageFragmentState(k, Vec2<unsigned int>(j, i)));
            pq[0].push(make_pair(0, storage[0].size()));
            storage[0].push_back(state);
        }

        int now_step = 0;
//        while(tm.is_within_time_limit()) {
        int loop_count = 0;
        while(true) {
            loop_count++;
            if (pq[now_step].empty()) {
                if (now_step >= 1) {
                    vector<ImageState>().swap(storage[now_step-1]);
                }
                now_step++;
                loop_count = 0;
                cout << now_step << endl;
                continue;
            }
            double now_cost = pq[now_step].top().first;
            int storage_idx = pq[now_step].top().second;
            pq[now_step].pop();

            ImageState& state = storage[now_step][storage_idx];

            int next_set_idx = state.repl_order.size();
            Vec2<unsigned int> next_set_coord = rev_order_table[next_set_idx];

            rep(i,div_num.y) rep(j,div_num.x) {
                unsigned int my_idx = i*div_num.x+j;
                if (state.placed[my_idx]) continue;
                int best_rotation_times = -1;
                double best_diff_cost = 1e20;
                rep(k,4) {
                    double tmp_diff_cost = 0;
                    if (next_set_coord.x != 0) {
                        unsigned int opp_set_order = order_table[next_set_coord.y][next_set_coord.x-1];
                        auto& opp_coord = state.repl_order[opp_set_order].orig_coord;
                        unsigned int opp_idx = opp_coord.y*div_num.x + opp_coord.x;
                        unsigned int opp_adjacent_side = (5-state.repl_order[opp_set_order].rotation_times)%4;
                        unsigned int opp_offset = (opp_idx*4 + opp_adjacent_side) * frag_num * 4;
                        tmp_diff_cost += *(adjacency + opp_offset + my_idx*4 + 3-k);
                    }
                    if (next_set_coord.y != 0) {
                        unsigned int opp_set_order = order_table[next_set_coord.y-1][next_set_coord.x];
                        auto& opp_coord = state.repl_order[opp_set_order].orig_coord;
                        unsigned int opp_idx = opp_coord.y*div_num.x + opp_coord.x;
                        unsigned int opp_adjacent_side = (6-state.repl_order[opp_set_order].rotation_times)%4;
                        unsigned int opp_offset = (opp_idx*4 + opp_adjacent_side) * frag_num * 4;
                        tmp_diff_cost += *(adjacency + opp_offset + my_idx*4 + (4-k)%4);
                    }
                    if (best_diff_cost > tmp_diff_cost) {
                        best_diff_cost = tmp_diff_cost;
                        best_rotation_times = k;
                    }
                }

                ImageState tmp = state;
                tmp.set_image(my_idx, ImageFragmentState(best_rotation_times, Vec2<unsigned int>(j, i)));
                pq[now_step+1].push(make_pair(now_cost+best_diff_cost, storage[now_step+1].size()));
                storage[now_step+1].push_back(tmp);
            }

//            now_step = (now_step+1)%(frag_num-1);
            if (now_step && loop_count%200 == 0) {
                if (now_step >= 1) {
                    vector<ImageState>().swap(storage[now_step-1]);
                }
                if (now_step == frag_num-2) break;
                else {
                    now_step++;
                    loop_count = 0;
                    cout << now_step << endl;
                }
            }
        }

        rep(i,frag_num) cout << i << ": " << pq[i].size() << endl;
        cout << pq[frag_num-1].top().first << endl;
        {
            int idx = pq[frag_num-1].top().second;
            ImageState state = storage[frag_num-1][idx];
            rep(i, div_num.y) {
                rep(j, div_num.x) {
                    cout << state.repl_order[order_table[i][j]].orig_coord << " " << state.repl_order[order_table[i][j]].rotation_times << ", ";
                }
                cout << endl;
            }
        }


        cout << "simple solver" << endl;
        return ImageState();
    }
};

template<class ADJACENCY_EVALUATOR, class SOLVER>
ImageState solver(const Image& img, const Settings& settings) {
    // img : RGB[DIV_NUM.y][DIV_NUM.x][FRAG_SIZE][FRAG_SIZE]
    // adjacency : double[DIV_NUM.y][DIV_NUM.x][4][DIV_NUM.y][DIV_NUM.x][4]
    // ans : ImageFragmentState[DIV_NUM.y][DIV_NUM.x]

    ADJACENCY_EVALUATOR adjacency_evaluator;
    SOLVER solver;

    const unsigned int N = settings.DIV_NUM().x*settings.DIV_NUM().y*4;
    auto *adjacency = new double[N*N];
    adjacency_evaluator.get_adjacency(img, settings, adjacency);

    ImageState ans = solver.solve(adjacency, settings);

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
    img.dump_perimeter(Vec2<int>(0,0));
    solver<SimpleAdjacencyEvaluator, SimpleSolver>(img, settings);
}
