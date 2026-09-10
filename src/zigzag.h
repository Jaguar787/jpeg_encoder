#ifndef ZIGZAG_H
#define ZIGZAG_H

#include "dct.h"

struct ZigZagBlock
{
    int16_t data[64];
};

ZigZagBlock zigzag_scan(const QuantBlock8x8 &q_block);
ZigZagBlock zigzag_scan(const int block[8][8]);

#endif