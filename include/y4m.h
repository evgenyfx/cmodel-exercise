#ifndef Y4M_H
#define Y4M_H

#include <cstdint>
#include <string>
#include <vector>

struct MonoImage {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;
};

// Read 8-bit monochrome frame
bool read_y4m(const std::string& path, MonoImage& image, std::string& error);

// Write one 8-bit monochrome frame. Stride is in bytes.
bool write_y4m(const std::string& path, const std::uint8_t* pixels,
               int width, int height, int stride, std::string& error);

#endif
