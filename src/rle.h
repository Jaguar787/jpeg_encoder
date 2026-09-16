#ifndef RLE_H
#define RLE_H

#include <vector>

#include "zigzag.h"

struct EncoderState
{
    int16_t prev_dc_y = 0;
    int16_t prev_dc_cb = 0;
    int16_t prev_dc_cr = 0;

    void reset()
    {
        prev_dc_y = 0;
        prev_dc_cb = 0;
        prev_dc_cr = 0;
    }
};

// Represents a single (runlength, value) pair for AC coefficients
struct ACSymbol
{
    uint8_t runlength;
    int16_t value;
};

// Represents the full 8x8 block ready for bit-packing
struct EncodedBlockSymbols
{
    int16_t dc_diff;
    std::vector<ACSymbol> ac_syms;
};

EncodedBlockSymbols encode_rle(const ZigZagBlock &z, int16_t prev_dc);

#endif