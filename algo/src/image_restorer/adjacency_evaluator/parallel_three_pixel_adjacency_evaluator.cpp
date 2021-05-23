#include "parallel_three_pixel_adjacency_evaluator.hpp"

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

void ParallelThreePixelAdjacencyEvaluator::get_adjacency(const Image& img, const Settings& settings, double *adjacency) {
    constexpr unsigned int size = 1;
    auto frag_size = settings.FRAG_SIZE();
    auto div_num = settings.DIV_NUM();
    auto dived_img_num = div_num.x * div_num.y;
    // RGB[div_num.y][div_num.x][4][frag_size, size]
    RGB *perimeters = new RGB[dived_img_num * 4 * frag_size];
    img.get_all_perimeter(size, perimeters);

    auto *memo = new Vec3<double>[frag_size];
    double *now_ptr = adjacency;
    unsigned int offset;
    double bias = 2;

    rep(i, div_num.y) 
        rep(j, div_num.x) 
            rep(k, 4) {
                offset = (i * div_num.x * 4 + j * 4 + k) * frag_size;
                rep(y, frag_size) {
                    memo[frag_size-y-1] = *(perimeters+offset+y);
                }

                rep(I, div_num.y) 
                    rep(J, div_num.x) {
                        if (i == I && j == J) {
                            rep(K,4) {
                                *now_ptr = INFINITY;
                                now_ptr++;
                            }
                            continue;
                        }

                        rep(K,4) {
                            offset = (I*div_num.x*4+J*4+K)*frag_size;
                            double var_sum = 0;
                            rep(y,frag_size) {
                                Vec3<double> tmp;
                                if (y == 0) {
                                    tmp = (static_cast<Vec3<double>>(*(perimeters + offset + y)) * bias
                                    + static_cast<Vec3<double>>(*(perimeters + offset + y + 1)) 
                                    - memo[y] * bias - memo[y + 1]) / (bias + 1);
                                }
                                else if (y == static_cast<int>(frag_size - 1)) {
                                    tmp = (static_cast<Vec3<double>>(*(perimeters + offset + y)) * bias
                                    + static_cast<Vec3<double>>(*(perimeters + offset + y - 1)) 
                                    - memo[y] * bias - memo[y - 1]) / (bias + 1);
                                }
                                else {
                                    tmp = (static_cast<Vec3<double>>(*(perimeters + offset + y - 1))
                                    + static_cast<Vec3<double>>(*(perimeters + offset + y)) * bias
                                    + static_cast<Vec3<double>>(*(perimeters + offset + y + 1))
                                    - memo[y - 1] - memo[y] * bias - memo[y + 1]) / (bias + 2);
                                }
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
}