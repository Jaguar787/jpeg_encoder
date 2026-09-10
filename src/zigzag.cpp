#include <cmath>
#include <iostream>

#include "dct.h"
#include "zigzag.h"

const int ZIGZAG_MAP[64][2] = {
    {0, 0}, {0, 1}, {1, 0}, {2, 0}, {1, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 1}, {3, 0}, {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4}, {0, 5}, {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0}, {6, 0}, {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6}, {0, 7}, {1, 6}, {2, 5}, {3, 4}, {4, 3}, {5, 2}, {6, 1}, {7, 0}, {7, 1}, {6, 2}, {5, 3}, {4, 4}, {3, 5}, {2, 6}, {1, 7}, {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2}, {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {7, 5}, {6, 6}, {5, 7}, {6, 7}, {7, 6}, {7, 7}};

ZigZagBlock zigzag_scan(const QuantBlock8x8 &q_block)
{
    ZigZagBlock z_block;

    for (int x = 0; x < 64; x++)
    {
        int row = ZIGZAG_MAP[x][0];
        int col = ZIGZAG_MAP[x][1];

        z_block.data[x] = q_block.data[row][col];
    }

    return z_block;
}

ZigZagBlock zigzag_scan(const int block[8][8])
{
    ZigZagBlock z_block;

    for (int x = 0; x < 64; x++)
    {
        int row = ZIGZAG_MAP[x][0];
        int col = ZIGZAG_MAP[x][1];

        z_block.data[x] = block[row][col];
    }

    return z_block;
}