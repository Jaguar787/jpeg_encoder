#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include "ppm.h"
#include "dct.h"
#include "zigzag.h"
#include "rle.h"
#include "huffman_encoding.h"
#include "bitwriter.h"
#include "encode.h"

void JPEGEncoder::encode_image()
{
    // Calculate total blocks, rounding up to handle padding
    int blocks_across = (img.width + 7) / 8;
    int blocks_down = (img.height + 7) / 8;
    HuffmanTable dc_table(true, true);
    HuffmanTable ac_table(false, false);
    dc_table.initialize();
    ac_table.initialize();
    for (int by = 0; by < blocks_down; by++)
    {
        for (int bx = 0; bx < blocks_across; bx++)
        {
            // Extract the three blocks for this 8x8 physical patch
            Block8x8 y_block = extract_block(img, bx, by, 0);
            Block8x8 cb_block = extract_block(img, bx, by, 1);
            Block8x8 cr_block = extract_block(img, bx, by, 2);

            // DCT calculations for each block
            Block8x8 y_dct = perform_dct(y_block);
            Block8x8 cb_dct = perform_dct(cb_block);
            Block8x8 cr_dct = perform_dct(cr_block);

            // Quant calculations for each block
            QuantBlock8x8 y_q = quantize(y_dct, LUMINANCE_TABLE);
            QuantBlock8x8 cb_q = quantize(cb_dct, CHROMINANCE_TABLE);
            QuantBlock8x8 cr_q = quantize(cr_dct, CHROMINANCE_TABLE);

            ZigZagBlock y_zz = zigzag_scan(y_q);
            ZigZagBlock cb_zz = zigzag_scan(cb_q);
            ZigZagBlock cr_zz = zigzag_scan(cr_q);

            EncodedBlockSymbols y_rle = encode_rle(y_zz, state.prev_dc_y);
            state.prev_dc_y = y_zz.data[0];
            EncodedBlockSymbols cb_rle = encode_rle(cb_zz, state.prev_dc_cb);
            state.prev_dc_cb = cb_zz.data[0];
            EncodedBlockSymbols cr_rle = encode_rle(cr_zz, state.prev_dc_cr);
            state.prev_dc_cr = cr_zz.data[0];

            // Second false does not do anything yet
            BitWriter bitwriter_y;
            write_block_bits(bitwriter_y, y_rle, dc_table, ac_table);
        }
    }
}
