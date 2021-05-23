#include "vertical_two_pixel_adjacency_evaluator.hpp"

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

// 面に対して垂直方向にピクセルをとり反映させるのはあまりeffectiveではない
void VerticalTwoPixelAdjacencyEvaluator::get_adjacency(const Image& img, const Settings& settings, double *adjacency) {
    constexpr unsigned int size = 2;
    auto frag_size = settings.FRAG_SIZE();
    auto div_num = settings.DIV_NUM();
    auto dived_img_num = div_num.x * div_num.y;
    RGB *perimeters = new RGB[dived_img_num * 4 * frag_size * size];
    // size分の深さだけピクセルをとる仕様になっている
    // 重複は考慮されていない(下の方法だと重複は問題ない)
    img.get_all_perimeter(size, perimeters);

    // 両者2ピクセル分の平均値をとってそれをもとに類似度を計算する
    auto *average_perimeters = new Vec3<double>[dived_img_num * 4 * frag_size];
    unsigned int offset;

    rep(i, div_num.y) {
        rep(j, div_num.x) {
            rep(k, 4) {
                rep(y, frag_size) {
                    Vec3<double> ave(0, 0, 0);
                    int divc = 0;
                    rep(s, size) {
                        // i * div_num.x * 4 * frag_size * size + j * 4 * frag_size * size + k * frag_size * size + y * size + s
                        offset = (((i * div_num.x + j) * 4 + k) * frag_size + y) * size + s;
                        ave += static_cast<Vec3<double>>(perimeters[offset]) * (s + 1) * (s + 1);
                        divc += (s + 1) * (s + 1);
                    }
                    ave /= divc;
                    offset = ((i * div_num.x + j) * 4 + k) * frag_size + y;
                    average_perimeters[offset] = ave;
                }
            }
        }
    }
        
    auto *memo = new Vec3<double>[frag_size];
    double *now_ptr = adjacency;

    rep(i, div_num.y) 
        rep(j, div_num.x) 
            rep(k, 4) {
                // 計算式1
                // array[i, j, k]
                // 1行: div_num.x * 4 * frag_size [px]
                // i * (div_num.x * 4 * frag_size) + j * (4 * frag_size) + k * (frag_size)
                offset = (i * div_num.x * 4 + j * 4 + k) * frag_size;
                rep(y, frag_size) {
                    // 反転してメモ
                    memo[frag_size-y-1] = *(average_perimeters+offset+y);
                }

                // 1つの辺とそれぞれの辺との類似度を見る
                rep(I, div_num.y) 
                    rep(J, div_num.x) {
                        // 比較対象と同じ断片だった場合はINFINITYをセット
                        if (i == I && j == J) {
                            rep(K,4) {
                                *now_ptr = INFINITY;
                                now_ptr++;
                            }
                            continue;
                        }

                        // それぞれの辺に対して類似度を計算する
                        rep(K,4) {
                            // 計算式1で比較する辺のOffsetを求める
                            offset = (I*div_num.x*4+J*4+K)*frag_size;
                            double var_sum = 0;
                            rep(y,frag_size) {
                                // RGBをVec3にキャストして2辺の差をとる（一方は隣接するときに反転して隣接しなければならないのでmemoを使う）
                                Vec3<double> tmp = *(average_perimeters+offset+y) - memo[y];
                                // 類似度を計算(それぞれの色要素で2乗したものの合計の平均を2で割ったもの)
                                var_sum += tmp.mul_each_other(tmp).sum() / 3. / 2.;
                            }
                            // 類似度の平均を求める
                            double var_ave = var_sum / frag_size;
                            // 結果の格納
                            *now_ptr = sqrt(var_ave);
                            now_ptr++;
                        }
                    }
            }

    delete[] perimeters;
    delete[] memo;
    delete[] average_perimeters;
}