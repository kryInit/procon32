#define PY_SSIZE_T_CLEAN
#include <python3.8/Python.h>
#include <opencv2/core.hpp>
#include "convolution_adjacency_evaluator.hpp"

using namespace std;
using namespace cv;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

void ConvolutionAdjacencyEvaluator::get_adjacency(const Image& img, const Settings& settings, double *adjacency) {
    auto frag_size = settings.FRAG_SIZE();
    auto div_num = settings.DIV_NUM();
    Mat images[div_num.y * div_num.x];
    rep(i, div_num.y) {
        rep(j, div_num.x) {
            auto fragment = img.img_frags[i][j];
            Mat matrix(Size(frag_size, frag_size), CV_8UC3);
            convert_to_mat(fragment, matrix);
            images[div_num.y * i + div_num.x * j] = matrix;
        }
    }
    
}

void convert_to_mat(ImageFragment frag, Mat mat) {
    auto frag_size = frag.FRAG_SIZE;
    rep(i, frag_size) {
        rep (j, frag_size) {
            auto rgb = frag.img_data[i][j];
            mat.at<Vec3b>(i, j)[0] = rgb.b;
            mat.at<Vec3b>(i, j)[1] = rgb.g;
            mat.at<Vec3b>(i, j)[2] = rgb.r;
        }
    }
}