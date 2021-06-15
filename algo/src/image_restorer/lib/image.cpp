#include "image.hpp"
#include <sstream>
#include <fstream>
using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)
#define repo(i,o,n) for(int i=static_cast<int>(o); i < static_cast<int>(n); ++i)

ImageFragment::ImageFragment() : FRAG_SIZE(0), img_data(nullptr) {}
ImageFragment::~ImageFragment() {
    if (img_data != nullptr) {
        delete[] *img_data;
        delete[] img_data;
        img_data = nullptr;
    }
}

[[nodiscard]] const unsigned int& ImageFragment::FRAGMENT_SIZE() const { return FRAG_SIZE; }
void ImageFragment::load(const string& frag_path, const Settings& settings) {
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
void ImageFragment::dump() const {
    rep(i,FRAG_SIZE) {
        cout << "[";
        rep(j,FRAG_SIZE) {
            cout << img_data[i][j] << ", ";
        }
        cout << "]" << endl;
    }
}
void ImageFragment::pdump() const {
    rep(i,FRAG_SIZE) {
        cout << "[";
        rep(j,FRAG_SIZE) {
            cout << &img_data[i][j] << ", ";
        }
        cout << "]" << endl;
    }
}
void ImageFragment::get_perimeter(unsigned int size, RGB *perimeter) const {
    // perimeter: RGB[4][FRAG_SIZE][size]
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

Image::Image() : DIV_NUM(0, 0), img_frags(nullptr) {}
Image::~Image() {
    if (img_frags != nullptr) {
        delete[] *img_frags;
        delete[] img_frags;
        img_frags = nullptr;
    }
}

void Image::get_image_fragments(ImageFragment **frags)
{
    frags = Image::img_frags;
}

void Image::load(const string& frags_dir_path, const Settings& settings) {
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
        stringstream ss;
        ss << frags_dir_path << hex << uppercase << x << y << ".ppm.bin";
        img_frags[y][x].load(ss.str(), settings);
    }
}
void Image::dump() const {
    rep(i, DIV_NUM.y) rep(j,DIV_NUM.x) {
        cout << "img_flags[" << i << "][" << j << "]: " << endl;
        img_frags[i][j].dump();
        cout << endl;
    }
}

void Image::dump_perimeter(Vec2<int> idx) const {
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
void Image::get_all_perimeter(unsigned int size, RGB *perimeters) const {
    unsigned int offset = 4*img_frags[0][0].FRAGMENT_SIZE()*size;
    rep(i,DIV_NUM.y) rep(j,DIV_NUM.x) {
        img_frags[i][j].get_perimeter(size, perimeters + (i*DIV_NUM.x + j)*offset);
    }
}

