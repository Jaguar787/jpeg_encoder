#ifndef ENCODE_H
#define ENCODE_H

#include "ppm.h"
#include "dct.h"
#include "zigzag.h"
#include "rle.h"

class JPEGEncoder
{
private:
    EncoderState state;

public:
    const PPMImage &img;

    // Quant tables
    static constexpr int LUMINANCE_TABLE[8][8] = {
        {16, 11, 10, 16, 24, 40, 51, 61},
        {12, 12, 14, 19, 26, 58, 60, 55},
        {14, 13, 16, 24, 40, 57, 69, 56},
        {14, 17, 22, 29, 51, 87, 80, 62},
        {18, 22, 37, 56, 68, 109, 103, 77},
        {24, 35, 55, 64, 81, 104, 113, 92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103, 99}};

    static constexpr int CHROMINANCE_TABLE[8][8] = {
        {17, 18, 24, 47, 99, 99, 99, 99},
        {18, 21, 26, 66, 99, 99, 99, 99},
        {24, 26, 56, 99, 99, 99, 99, 99},
        {47, 66, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99}};

    JPEGEncoder(const PPMImage &img) : img(img) {};
    void encode_image();
    void write_dht(std::fstream &file, uint8_t tableClass, uint8_t tableID,
                   const uint8_t *counts, const uint8_t *values);
    void write_sos(std::fstream &file);
    void headify();
};
#endif