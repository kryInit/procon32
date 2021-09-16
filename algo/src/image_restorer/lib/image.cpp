#include <image.hpp>
#include <sstream>
#include <fstream>
using namespace std;

#define rep(i,n) for(int i=0; i < static_cast<int>(n); ++i)

ImageFragment::ImageFragment() : frag_size(0), img_data(nullptr) {}
ImageFragment::~ImageFragment() {
    if (img_data != nullptr) {
        delete[] *img_data;
        delete[] img_data;
        img_data = nullptr;
    }
}
void ImageFragment::load(const string& frag_path, const Settings& settings) {
    if (img_data != nullptr) {
        delete[] *img_data;
        delete[] img_data;
        img_data = nullptr;
    }

    // allocate memory
    frag_size = settings.frag_size;
    img_data = new RGB*[frag_size];
    img_data[0] = new RGB[frag_size*frag_size];
    rep(i, frag_size) img_data[i] = &img_data[0][i*frag_size];

    // load data
    ifstream ifs(frag_path, ios::binary);
    ifs.read((char*)*img_data, static_cast<streamsize>(frag_size*frag_size*sizeof(RGB)));
}
void ImageFragment::dump() const {
    rep(i,frag_size) {
        cout << "[";
        rep(j,frag_size) {
            cout << "(" << img_data[i][j].value() << "), ";
        }
        cout << "]" << endl;
    }
}
void ImageFragment::pdump() const {
    rep(i,frag_size) {
        cout << "[";
        rep(j,frag_size) {
            cout << &img_data[i][j] << ", ";
        }
        cout << "]" << endl;
    }
}
void ImageFragment::get_perimeter(unsigned int size, RGB *perimeter) const {
    // perimeter: RGB[4][frag_size][size]
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
    rep(k,4) rep(i,frag_size) rep(j,size) {
        int y,x;
        if (k == 0) y = isize-j-1, x = i;
        else if (k == 1) y = i, x = frag_size-isize + j;
        else if (k == 2) y = frag_size-isize + j, x = frag_size-i-1;
        else  y = frag_size-i-1, x = isize-j-1;
        *(perimeter + k*frag_size*size + i*size + j) = img_data[y][x];
    }
}

Image::Image() : frag_size(), div_num(0, 0), img_frags(nullptr) {}
Image::~Image() {
    if (img_frags != nullptr) {
        delete[] *img_frags;
        delete[] img_frags;
        img_frags = nullptr;
    }
}

void Image::load(const string& frags_dir_path, const Settings& settings) {
    if (img_frags != nullptr) {
        delete[] *img_frags;
        delete[] img_frags;
        img_frags = nullptr;
    }

    // allocate memory
    frag_size = settings.frag_size;
    div_num = settings.div_num;
    img_frags = new ImageFragment*[div_num.y];
    img_frags[0] = new ImageFragment[div_num.y*div_num.x];
    rep(i, div_num.y) img_frags[i] = &img_frags[0][i*div_num.x];

    // load data
    rep(y, div_num.y) rep(x, div_num.x) {
        stringstream ss;
        ss << frags_dir_path << hex << uppercase << x << y << ".ppm.bin";
        img_frags[y][x].load(ss.str(), settings);
    }
}
void Image::dump() const {
    rep(i, div_num.y) rep(j,div_num.x) {
        cout << "img_flags[" << i << "][" << j << "]: " << endl;
        img_frags[i][j].dump();
        cout << endl;
    }
}

void Image::dump_perimeter(Pos pos) const {
    unsigned int size = 3;
    RGB *tmp = new RGB[4*frag_size*size];
    img_frags[pos.y][pos.x].get_perimeter(size, tmp);

    rep(k,4) {
        cout << "k: " << k << endl;
        rep(i,frag_size) {
            rep(j,size) cout << *(tmp + k*frag_size*size + i*size + j) << ", ";
            cout << endl;
        }
        cout << endl;
    }

    delete[] tmp;
}
void Image::get_all_perimeter(unsigned int size, RGB *perimeters) const {
    unsigned int offset = 4*frag_size*size;
    rep(i,div_num.y) rep(j,div_num.x) {
        img_frags[i][j].get_perimeter(size, perimeters + (i*div_num.x + j)*offset);
    }
}