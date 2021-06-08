#pragma once

#include <vec.hpp>
#include "settings.hpp"

class ImageFragment {
public:
    unsigned int FRAG_SIZE;
    RGB **img_data;

    ImageFragment();
    ~ImageFragment();
    [[nodiscard]] const unsigned int& FRAGMENT_SIZE() const;
    void load(const std::string& frag_path, const Settings& settings);
    void dump() const;
    void pdump() const;
    void get_perimeter(unsigned int size, RGB *perimeter) const;
};

class Image {
    Vec2<unsigned int> DIV_NUM;

public:
    ImageFragment **img_frags;

    Image();
    ~Image();

    void load(const std::string& frags_dir_path, const Settings& settings);
    void dump() const;

    void dump_perimeter(Vec2<int> idx) const;
    void get_all_perimeter(unsigned int size, RGB *perimeters) const;
};

