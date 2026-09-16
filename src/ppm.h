#ifndef PPM_H
#define PPM_H

#include <vector>
#include <cstdint>
#include <fstream>

enum class PPMType
{
    P3_ASCII,
    P6_BINARY,
    FAIL,
    INVALID
};

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct YCbCrPixel
{
    uint8_t y;
    uint8_t cb;
    uint8_t cr;
};

struct PPMImage
{
    int width = 0;
    int height = 0;
    std::vector<RGB> pixels; // Flat array of RGB RGB RGB...
    std::vector<YCbCrPixel> ycbcr_image;
};

PPMType detect_ppm_type(std::ifstream &file);
PPMImage load_ppm(std::ifstream &file);
void color_shift(PPMImage &img);
void print_rgb(const PPMImage &img);
void print_ycbcr(const PPMImage &img);

#endif