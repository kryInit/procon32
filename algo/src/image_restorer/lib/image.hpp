#pragma once

#include <rgb.hpp>
#include <pos.hpp>
#include <settings.hpp>

class ImageFragment {
    int frag_size;
    RGB **img_data;

public:
    ImageFragment();
    ~ImageFragment();
    void load(const std::string& frag_path, const Settings& settings);
    void dump() const;
    void pdump() const;
    void get_perimeter(unsigned int size, RGB *perimeter) const;

    RGB* operator[](size_t idx) { return img_data[idx]; }
    RGB& operator[](const Pos& p) { return img_data[p.y][p.x]; }
    const RGB* operator[](size_t idx) const { return img_data[idx]; }
    const RGB& operator[](const Pos& p) const { return img_data[p.y][p.x]; }

};

class Image {
    int frag_size;
    Vec2<int> div_num;
    ImageFragment **img_frags;

public:
    Image();
    ~Image();

    void load(const std::string& frags_dir_path, const Settings& settings);
    void dump() const;

    void dump_perimeter(Pos pos) const;
    void get_all_perimeter(unsigned int size, RGB *perimeters) const;

    ImageFragment* operator[](size_t idx) { return img_frags[idx]; }
    ImageFragment& operator[](const Pos& p) { return img_frags[p.y][p.x]; }
    const ImageFragment* operator[](size_t idx) const { return img_frags[idx]; }
    const ImageFragment& operator[](const Pos& p) const { return img_frags[p.y][p.x]; }
};