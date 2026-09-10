#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "ppm.h"

/**
 * Detects the type of PPM image based on the header.
 * @param file The input file stream.
 * @return The detected PPMType.
 */

PPMType detect_ppm_type(std::ifstream &file)
{
    std::string header;
    if (!(file >> header))
        return PPMType::FAIL;
    if (header == "P3")
        return PPMType::P3_ASCII;
    if (header == "P6")
        return PPMType::P6_BINARY;
    std::cerr << "extraction failed: stream bad/eof, header=\"" << header << "\"" << std::endl;
    return PPMType::INVALID;
}

/**
 * Loads a PPM image from the given file stream.
 * @param file The input file stream.
 * @return A PPMImage object containing the image data.
 */

PPMImage load_ppm(std::ifstream &file)
{
    PPMImage img;
    PPMType type = detect_ppm_type(file);
    if (type == PPMType::INVALID)
    {
        std::cerr << "Invalid PPM type" << std::endl;
        return img;
    }
    else if (type == PPMType::FAIL)
    {
        std::cerr << "Failed to read PPM header (stream error or EOF)" << std::endl;
        return img;
    }

    int max_color = 0;

    file >> img.width >> img.height >> max_color;

    img.pixels.resize(img.width * img.height);

    if (type == PPMType::P3_ASCII)
    {
        int value = 0;
        for (uint8_t i = 0; i < img.pixels.size(); i++)
        {
            file >> value;
            img.pixels[i].r = static_cast<uint8_t>(value);
            file >> value;
            img.pixels[i].g = static_cast<uint8_t>(value);
            file >> value;
            img.pixels[i].b = static_cast<uint8_t>(value);
        }
    }
    else if (type == PPMType::P6_BINARY) // Has not been tested and should not work
    {
        file.read(reinterpret_cast<char *>(img.pixels.data()), img.pixels.size() * sizeof(RGB));
    }

    return img;
}

/**
 * Converts rgb to ycbcr
 * Uses the standard as defined in ITU-T Rec. T.871 (05/2011) to four decimal point accuracy
 * Y = 0.299R + 0.587G + 0.114B
 * C_b = -0.1687R - 0.3313G + 0.5B + 128
 * C_r = 0.5R - 0.4187G - 0.0813B + 128
 * Note: this implementation avoids flops opting for the conversion using: 65,536
 * Example: 0.299 * 65,536 = 19595
 * @param r,g,b,y,cb,cr The initial pixel colors and their results
 * @return void: passes y,cb,cr by reference
 */
void rgb_to_ycbcr(uint8_t r, uint8_t g, uint8_t b, uint8_t &y, uint8_t &cb, uint8_t &cr)
{
    int y_tmp = (19595 * r + 38469 * g + 7472 * b) >> 16;
    int cb_tmp = (-11059 * r - 21709 * g + 32768 * b) >> 16;
    int cr_tmp = (32768 * r - 27439 * g - 5329 * b) >> 16;

    y = static_cast<uint8_t>(std::max(0, std::min(255, y_tmp)));
    cb = static_cast<uint8_t>(std::max(0, std::min(255, cb_tmp + 128)));
    cr = static_cast<uint8_t>(std::max(0, std::min(255, cr_tmp + 128)));
}

/**
 * Calls the rgb_to_ycbcr conversion
 * @param img
 * @return void
 */
void color_shift(PPMImage &img)
{
    img.ycbcr_image.resize(img.pixels.size());

    for (uint8_t i = 0; i < img.pixels.size(); i++)
    {
        uint8_t y = 0, cb = 0, cr = 0;
        rgb_to_ycbcr(img.pixels[i].r, img.pixels[i].g, img.pixels[i].b, y, cb, cr);
        img.ycbcr_image[i].y = y;
        img.ycbcr_image[i].cb = cb;
        img.ycbcr_image[i].cr = cr;
    }
}

/**
 * Prints the rgb values of PPMImage
 * @param img
 * @return void
 */
void print_rgb(const PPMImage &img)
{
    std::cout << "RGB: " << std::endl;
    for (const auto &i : img.pixels)
    {
        std::cout << static_cast<int>(i.r) << " " << static_cast<int>(i.g) << " " << static_cast<int>(i.b) << std::endl;
    }
}

/**
 * Prints the ycbcr values of PPMImage
 * @param img
 * @return void
 */
void print_ycbcr(const PPMImage &img)
{
    std::cout << "YCbCr" << std::endl;
    for (const auto &i : img.ycbcr_image)
    {
        std::cout << static_cast<int>(i.y) << " " << static_cast<int>(i.cb) << " " << static_cast<int>(i.cr) << std::endl;
    }
}