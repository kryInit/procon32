#include "one_pixel_adjacency_evaluator.hpp"

using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

void OnePixelAdjacencyEvaluator::get_adjacency(const Image& img, const Settings& settings, double *adjacency) {

    constexpr unsigned int size = 1;
    auto frag_size = settings.FRAG_SIZE(); // 断片画像のpixel数
    auto div_num = settings.DIV_NUM(); // 縦と横の分割数
    auto dived_img_num = div_num.x * div_num.y;
    RGB *perimeters = new RGB[dived_img_num * 4 * frag_size]; // Vec3と内部的に同じ
    img.get_all_perimeter(size, perimeters); // perimeterに指定のフォーマットで端1pxを取得

    auto *memo = new Vec3<double>[frag_size];
    double *now_ptr = adjacency;
    unsigned int offset;

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
                    memo[frag_size-y-1] = *(perimeters+offset+y);
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
                                Vec3<double> tmp = static_cast<Vec3<double>>(*(perimeters+offset+y)) - memo[y];
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
}
