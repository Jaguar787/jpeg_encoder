#ifndef DCT_H
#define DCT_H

#include "ppm.h"

struct Block8x8
{
    float data[8][8];
};

struct QuantBlock8x8
{
    int16_t data[8][8];
};

Block8x8 extract_block(const PPMImage &img, int block_x, int block_y, int channel);
void initialize_table();
float get_alpha(int uv);
Block8x8 perform_dct(const Block8x8 &pixel_block);
QuantBlock8x8 quantize(const Block8x8 &dct, const int q_table[8][8]);
void print_block(const Block8x8 &block);
void print_block(const QuantBlock8x8 &block);

#endif