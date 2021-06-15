#define PY_SSIZE_T_CLEAN
#include <python3.8/Python.h>
#include <opencv2/core.hpp>
#include "convolution_adjacency_evaluator.hpp"

void ConvolutionAdjacencyEvaluator::get_adjacency(const Image& img, const Settings& settings, double *adjacency)
{
    auto frag_size = settings.FRAG_SIZE();
    auto div_num = settings.DIV_NUM();
    cv::Mat images[div_num.y][div_num.x];
    auto frags = img.img_frags;

    for (auto row = 0u; div_num.y > row; row++)
        for (auto col = 0u; div_num.x > col; col++)
        {
            auto fragment = frags[row][col];
            cv::Mat matrix(cv::Size(frag_size, frag_size), CV_8UC3);
            convert_to_mat(fragment, matrix);
            images[row][col] = matrix;
        }
    
}

void convert_to_mat(ImageFragment frag, cv::Mat mat) {
    auto frag_size = frag.FRAG_SIZE;
    for (auto y = 0u; frag_size > y; y++)
        for (auto x = 0u; frag_size > x; x++)
            auto rgb = frag.img_data[y][x];
            mat.at<cv::Vec3b>(x, y)[0] = rgb.b;
            mat.at<cv::Vec3b>(i, j)[1] = rgb.g;
            mat.at<cv::Vec3b>(i, j)[2] = rgb.r;
        }
}