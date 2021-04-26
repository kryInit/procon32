#include<bits/stdc++.h>

#define CHECK(call) {                                                      \
    const cudaError_t error = call;                                        \
    if (error != cudaSuccess) {                                            \
        printf("Error: %s%d, ", __FILE__, __LINE__);                       \
        printf("code:%d, reason: %s\n", error, cudaGetErrorString(error)); \
        exit(1);                                                           \
    }                                                                      \
}

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)

template<class T = int>
struct Vec2 {
    T x, y;

    template<class S> inline Vec2& operator =(const Vec2<S>& other) {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }
    template<class S> inline bool operator<(const Vec2<S>& another) const {
        return x == another.x ? y < another.y : x < another.x;
    }
    template<class S> inline bool operator==(const Vec2<S>& another) const {
        return x == another.x && y == another.y;
    }
    template<class S> inline bool operator!=(const Vec2<S>& another) const {
        return x != another.x || y != another.y;
    }
    template<class S> inline Vec2 operator+(const Vec2<S>& other) const {
        return { x + other.x, y + other.y };
    }
    template<class S> inline Vec2& operator +=(const Vec2<S>& other) {
        *this = *this+other;
        return *this;
    }
    template<class S> inline Vec2 operator -(const Vec2<S>& other) const {
        return { x - other.x, y - other.y };
    }
    template<class S> inline Vec2& operator -=(const Vec2<S>& other) {
        *this = *this - other;
        return *this;
    }
    template<class S> inline Vec2 operator *(S s) const {
        return { x*s, y*s };
    }
    template<class S> inline Vec2& operator *=(S s) {
        *this = *this*s;
        return *this;
    }
    template<class S> inline Vec2 operator /(S s) const {
        return { x/s, y/s };
    }
    template<class S> inline Vec2& operator /=(S s) {
        *this = *this/s;
        return *this;
    }


    Vec2() : x(0), y(0) {};
    template<class N, class M> Vec2(N _x, M _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) {}

    void dump() const { cout << '(' << x << ", " << y << ')' << endl; }
};

template<class T>
std::ostream &operator<<(std::ostream &out, const Vec2<T> &v) {
    out << '(' << v.x << ", " << v.y << ')';
    return out;
}

template<class T = int>
struct Vec3 {
    union {
        struct { T x,y,z; };
        struct { T r,g,b; };
    };

    template<class S> inline Vec3& operator =(const Vec3<S>& other) {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        return *this;
    }
    template<class S> inline bool operator<(const Vec3<S>& another) const {
        return x == another.x ? (y == another.y ? z < another.z : y < another.y) : x < another.x;
    }
    template<class S> inline bool operator==(const Vec3<S>& another) const {
        return x == another.x && y == another.y && z == another.z;
    }
    template<class S> inline bool operator!=(const Vec3<S>& another) const {
        return x != another.x || y != another.y || z != another.z;
    }
    template<class S> inline Vec3 operator+(const Vec3<S>& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    template<class S> inline Vec3& operator +=(const Vec3<S>& other) {
        *this = *this+other;
        return *this;
    }
    template<class S> inline Vec3 operator -(const Vec3<S>& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    template<class S> inline Vec3& operator -=(const Vec3<S>& other) {
        *this = *this-other;
        return *this;
    }
    template<class S> inline Vec3 operator *(S s) const {
        return { x*s, y*s, z*s };
    }
    template<class S> inline Vec3& operator *=(S s) {
        *this = *this*s;
        return *this;
    }
    template<class S> inline Vec3 operator /(S s) const {
        return { x/s, y/s, z/s };
    }
    template<class S> inline Vec3& operator /=(S s) {
        *this = *this/s;
        return *this;
    }

    Vec3() : x(), y(), z() {}
    template<class N, class M, class L> Vec3(N _x, M _y, L _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) {}

    void dump() { cout << '(' << x << ", " << y << ", " << z << ')' << endl; }
    double mag() {
        auto X = static_cast<double>(x);
        auto Y = static_cast<double>(y);
        auto Z = static_cast<double>(z);
        return sqrt(X*X+Y*Y+Z*Z);
    }
    template<class S>
    Vec3<T> mul_each_other(Vec3<S>& other) const {
        return { x*static_cast<T>(other.x), y*static_cast<T>(other.y), z*static_cast<T>(other.z) };
    }
    T sum() const { return x + y + z; }
};

template<class T>
std::ostream &operator<<(std::ostream &out, const Vec3<T> &v) {
    out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return out;
}

using RGB = Vec3<unsigned char>;

template <>
void RGB::dump() {
    cout << static_cast<unsigned>(r) << ", " << static_cast<unsigned>(g) << ", " << static_cast<unsigned>(b) << endl;
}

ostream& operator <<(ostream& os, const RGB& rgb) {
    os << '(' << static_cast<unsigned>(rgb.r) << ", " << static_cast<unsigned>(rgb.g) << ", " << static_cast<unsigned>(rgb.b)<< ')';
    return os;
}

class ImageFragment {
    RGB *img_data;
public:
    const unsigned int FRAG_SIZE;

    // ImageFragment() : PIX_SIZE(0), img_data(nullptr) {}
    ImageFragment() = delete;
    explicit ImageFragment(unsigned int frag_size) : FRAG_SIZE(frag_size) {
        img_data = (RGB*)malloc(FRAG_SIZE*FRAG_SIZE*sizeof(RGB));
    }
    ~ImageFragment() {
        if (img_data != nullptr) free(img_data);
    }

    void input(const string& file_name) {
        ifstream ifs(file_name, ios::binary);
        ifs.read((char*)img_data, FRAG_SIZE*FRAG_SIZE*sizeof(RGB));
    }
    void dump() {
        rep(i,FRAG_SIZE) {
            cout << "[";
            rep(j,FRAG_SIZE) {
                cout << *(img_data+i*FRAG_SIZE+j) << ", ";
            }
            cout << "]" << endl;
        }
    }
    void pdump() {
        rep(i,FRAG_SIZE) {
            cout << "[";
            rep(j,FRAG_SIZE) {
                cout << (img_data+i*FRAG_SIZE+j) << ", ";
            }
            cout << "]" << endl;
        }

    }

    unsigned int area() const { return FRAG_SIZE*FRAG_SIZE; }

    void copy_img_data_for_cuda(RGB* img_data_cuda) const {
        cudaMemcpy(img_data_cuda, img_data, FRAG_SIZE*FRAG_SIZE*sizeof(RGB), cudaMemcpyHostToDevice);
    }
    void simple_ave_conv(const unsigned int HALF_FILTER_SIZE, const unsigned int W_FOR_VAR, Vec3<float> *result) const {
        // result[4][FRAG_SIZE][W_FOR_VAR]
        rep(k,4) rep(i,FRAG_SIZE) rep(j,W_FOR_VAR) {
            int cy,cx;
            if (k == 0) cy = j, cx = i;
            else if (k == 1) cy = i, cx = FRAG_SIZE-j-1;
            else if (k == 2) cy = FRAG_SIZE-j-1, cx = FRAG_SIZE-i-1;
            else  cy = FRAG_SIZE-i-1, cx = j;

            Vec3<double> sum;
            int ly = max(0, cy-(int)HALF_FILTER_SIZE), ry = min(FRAG_SIZE-1, cy+HALF_FILTER_SIZE);
            int lx = max(0, cx-(int)HALF_FILTER_SIZE), rx = min(FRAG_SIZE-1, cx+HALF_FILTER_SIZE);
            for (int y = ly; y<=ry; y++) {
                for (int x = lx; x<=rx; ++x) {
                    sum += *(img_data + y*FRAG_SIZE + x);
                }
            }
            *(result + k*FRAG_SIZE*W_FOR_VAR + i*W_FOR_VAR + j) = sum / ((ry-ly+1)*(rx-lx+1));
        }
    }
};

class Image {
    ImageFragment *img_frags;

public:
    const Vec2<> DIV_NUM;
    const unsigned int FRAG_SIZE;

    Image(unsigned int w_div, unsigned int y_div, unsigned int frag_size, const string& frags_dir_path) : Image(Vec2<>(w_div, y_div), frag_size, frags_dir_path) {}
    Image(Vec2<> div_num, unsigned int frag_size, const string& frags_dir_path) : DIV_NUM(div_num), FRAG_SIZE(frag_size) {
        img_frags = (ImageFragment*)malloc(DIV_NUM.x*DIV_NUM.y*sizeof(ImageFragment));
        for (int i=0; i<DIV_NUM.y; ++i) {
            for (int j=0; j<DIV_NUM.x; ++j) {
                new(img_frags+i*DIV_NUM.x+j) ImageFragment(frag_size);
            }
        }

        for (int y=0; y<DIV_NUM.y; ++y) {
            for (int x=0; x<DIV_NUM.x; ++x) {
                stringstream ss;
                ss << frags_dir_path << hex << uppercase << x << y << ".ppm.bin";
                (img_frags+y*DIV_NUM.x+x)->input(ss.str());
            }
        }
    }

    unsigned int frag_num() const { return DIV_NUM.y * DIV_NUM.x; }

    void dump() {
        for(int i=0; i<DIV_NUM.y; ++i) {
            for (int j=0; j<DIV_NUM.x; ++j) {
                cout << "img_flags[" << i << "][" << j << "]: " << endl;
                (img_frags+i*DIV_NUM.x+j)->dump();
                cout << endl;
            }
        }
    }
    void copy_img_data_for_cuda(RGB* img_data_cuda) const {
        for (int idx=0; idx<DIV_NUM.y*DIV_NUM.x; ++idx) {
            (img_frags + idx)->copy_img_data_for_cuda(img_data_cuda + idx*((img_frags+idx)->area()));
        }
    }
    void simple_ave_conv(const unsigned int HALF_FILTER_SIZE, const unsigned int W_FOR_VAR, Vec3<float> *result) const {
        // result[4][FRAG_SIZE][W_FOR_VAR]
        // result[DIV_NUM.y][DIV_NUM.x][4][FRAG_SIZE][W_FOR_VAR]
        printf("result[%u][%u][4][%u][%u]\n", DIV_NUM.y, DIV_NUM.x, FRAG_SIZE, W_FOR_VAR);
        int one_result_size = 4*FRAG_SIZE*W_FOR_VAR;
        rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) {
            int idx = i*DIV_NUM.x + j;
            cout << idx << endl;
            (img_frags + idx)->simple_ave_conv(HALF_FILTER_SIZE, W_FOR_VAR, result + idx*one_result_size);
        }
    }

    ~Image() {
        if (img_frags != nullptr) free(img_frags);
    }
};

class AdjacencyEvaluator {
public:
    const unsigned int HALF_FILTER_SIZE, W_FOR_VAR;

    AdjacencyEvaluator() : HALF_FILTER_SIZE(4), W_FOR_VAR(2) {}
    AdjacencyEvaluator(unsigned int half_filter_size, unsigned int w_for_var) : HALF_FILTER_SIZE(half_filter_size), W_FOR_VAR(w_for_var) {}

    void calc_similarity_simply(const Image &img) {
        const unsigned int ave_conv_result_size = img.frag_num()*4*img.FRAG_SIZE*W_FOR_VAR;
        auto *ave_conv_result = (Vec3<float>*)malloc(ave_conv_result_size*sizeof(Vec3<float>));
        img.simple_ave_conv(HALF_FILTER_SIZE, W_FOR_VAR, ave_conv_result);

        rep(i,img.DIV_NUM.y) rep(j,img.DIV_NUM.x) {
            cout << *(ave_conv_result + (i*img.DIV_NUM.x + j)*4*img.FRAG_SIZE*W_FOR_VAR) << endl;
        }

        const unsigned int similarity_size = img.frag_num()*4*img.frag_num()*4;
        auto similarity = (double*)malloc(similarity_size*sizeof(double));
        auto sum_memo = (Vec3<double>*)malloc(img.FRAG_SIZE*sizeof(Vec3<double>));
        auto s_sum_memo = (Vec3<double>*)malloc(img.FRAG_SIZE*sizeof(Vec3<double>));
        double *now_ptr = similarity;
        rep(i,img.DIV_NUM.y) rep(j,img.DIV_NUM.x) rep(k,4) {
            {
                unsigned int offset = (i*img.DIV_NUM.x*4+j*4+k)*img.FRAG_SIZE*W_FOR_VAR;
                rep(y, img.FRAG_SIZE) {
                    sum_memo[img.FRAG_SIZE-y-1] = {0,0,0};
                    s_sum_memo[img.FRAG_SIZE-y-1] = {0,0,0};
                    rep(x, W_FOR_VAR) {
                        Vec3<double> tmp;
                        tmp += *(ave_conv_result + offset + y*W_FOR_VAR+x);
                        sum_memo[img.FRAG_SIZE-y-1] += tmp;
                        s_sum_memo[img.FRAG_SIZE-y-1] += tmp.mul_each_other(tmp);
                    }
                }
            }
            rep(I,img.DIV_NUM.y) rep(J,img.DIV_NUM.x) {
                if (i == I && j == J) {
                    rep(K,4) {
                        *now_ptr = INFINITY;
                        now_ptr++;
                    }
                    continue;
                }
                rep(K,4) {
                    unsigned int offset = (I*img.DIV_NUM.x*4+J*4+K)*img.FRAG_SIZE*W_FOR_VAR;
                    double var_sum = 0;
                    rep(y, img.FRAG_SIZE){
                        Vec3<double> sum = sum_memo[y];
                        Vec3<double> s_sum = s_sum_memo[y];
                        rep(x, W_FOR_VAR) {
                            Vec3<double> tmp;
                            tmp += *(ave_conv_result + offset + y*W_FOR_VAR+x);
                            sum += tmp;
                            s_sum += tmp.mul_each_other(tmp);
                        }
                        double tmp = ((s_sum - (sum.mul_each_other(sum))/(2*W_FOR_VAR)) / (2*W_FOR_VAR)).sum() / 3.;
                        var_sum += tmp;
                    }
                    double ave_var = var_sum / img.FRAG_SIZE;
                    *now_ptr = ave_var;
                    now_ptr++;
                }
            }
        }
        rep(i,img.DIV_NUM.y) {
            rep(j,img.DIV_NUM.x) {
                rep(k,4) {
                    const unsigned int offset = (i*img.DIV_NUM.x*4+j*4+k)*img.frag_num()*4;
                    double best_similarity = 1e10;
                    int best_i, best_j, best_k;
                    rep(I,img.DIV_NUM.y) {
                        rep(J,img.DIV_NUM.x) {
                            if (I == i && J == j) continue;
                            rep(K,4) {
                                double tmp = *(similarity+offset + I*img.DIV_NUM.x*4 + J*4 + K);
                                if (best_similarity > tmp) {
                                    best_similarity = tmp;
                                    best_i = I;
                                    best_j = J;
                                    best_k = K;
                                }
                            }
                        }
                    }
                    cout << "(" << i << ", " << j << ", " << k <<  ") <=> (" << best_i << ", " << best_j << ", " << best_k << ") : " << best_similarity << endl;
                }
            }
        }
//        const unsigned int offset = (img.DIV_NUM.y*4 + 4);
        const unsigned int offset = 0;
        rep(i,img.DIV_NUM.y) {
            rep(j,img.DIV_NUM.x) {
                cout << "(";
                rep(k,4) cout << setw(2) << *(similarity + offset + i*img.DIV_NUM.x*4 + j*4+k) << ", ";
                cout << "), ";
            }
            cout << endl;
        }
        cout << endl;
    }
};

__global__ void kr(RGB *rgbs, int a);

int main(int argc, char *argv[]) {
#pragma omp parallel
    {
        std::cout << "Hello World!\n";
    }
    string frag_dir_path = string(argv[1]) + "/frags/";

    unsigned int h_div, w_div, frag_size;
    {
        unsigned int h_pix, w_pix, _;
        ifstream ifs("../prob.txt");
        ifs >> w_div >> h_div;
        ifs >> _ >> _ >> _;
        ifs >> w_pix >> h_pix;
        if (h_pix * w_div != w_pix * h_div) {
            cerr << "h_pix / h_div != w_pix / w_div" << endl;
            return -1;
        }
        frag_size = h_pix / h_div;
    }
    Image img(w_div, h_div, frag_size, frag_dir_path);
    img.dump();

    RGB *rgbs_cuda;
    cudaMalloc(&rgbs_cuda, h_div*w_div*frag_size*frag_size*sizeof(RGB));
    img.copy_img_data_for_cuda(rgbs_cuda);
    kr<<<1,1>>>(rgbs_cuda, h_div*w_div*frag_size*frag_size-1);
    cudaFree(rgbs_cuda);

    AdjacencyEvaluator ae(4, 4);
    ae.calc_similarity_simply(img);
}

__global__ void kr(RGB *rgbs, int a) {
    printf("%u %u %u\n", rgbs->r, rgbs->g, rgbs->b);
    rgbs += a;
    printf("%u %u %u\n", rgbs->r, rgbs->g, rgbs->b);
}
