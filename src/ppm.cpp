#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "ppm.h"

/**
 * Detects whether the image file uses ASCII or binary PPM encoding.
 * @param file Input stream positioned at the start of the PPM header.
 * @return The detected PPM format type.
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
 * Loads a PPM image from disk into a memory structure for later JPEG processing.
 * @param file Input stream containing the image file.
 * @return Parsed image data and metadata.
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
        for (size_t i = 0; i < img.pixels.size(); i++)
        {
            if (!(file >> value))
            {
                std::cerr << "Error: Premature end of file or invalid ASCII data at pixel " << i << std::endl;
                break;
            }
            img.pixels[i].r = static_cast<uint8_t>(value);

            if (!(file >> value))
                break;
            img.pixels[i].g = static_cast<uint8_t>(value);

            if (!(file >> value))
                break;
            img.pixels[i].b = static_cast<uint8_t>(value);
        }
    }
    else if (type == PPMType::P6_BINARY) // Has not been tested but "should" work
    {
        file.read(reinterpret_cast<char *>(img.pixels.data()), img.pixels.size() * sizeof(RGB));
    }

    return img;
}

/**
 * Converts an RGB triplet into YCbCr color space using the standard JPEG coefficients.
 * @param r Red channel value.
 * @param g Green channel value.
 * @param b Blue channel value.
 * @param y Output luma component.
 * @param cb Output blue-difference chroma component.
 * @param cr Output red-difference chroma component.
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
 * Converts every RGB pixel in the image to YCbCr and stores the result in the image structure.
 * @param img Image whose pixel data will be transformed in place.
 */
void color_shift(PPMImage &img)
{
    img.ycbcr_image.resize(img.pixels.size());

    for (size_t i = 0; i < img.pixels.size(); i++)
    {
        uint8_t y = 0, cb = 0, cr = 0;
        rgb_to_ycbcr(img.pixels[i].r, img.pixels[i].g, img.pixels[i].b, y, cb, cr);
        img.ycbcr_image[i].y = y;
        img.ycbcr_image[i].cb = cb;
        img.ycbcr_image[i].cr = cr;
    }
}

/**
 * Prints the RGB values of the image for debugging.
 * @param img Image to display.
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
 * Prints the YCbCr values of the image for debugging.
 * @param img Image to display.
 */
void print_ycbcr(const PPMImage &img)
{
    std::cout << "YCbCr" << std::endl;
    for (const auto &i : img.ycbcr_image)
    {
        std::cout << static_cast<int>(i.y) << " " << static_cast<int>(i.cb) << " " << static_cast<int>(i.cr) << std::endl;
    }
}