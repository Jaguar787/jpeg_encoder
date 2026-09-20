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

/**
 * Writes a JPEG Huffman table segment (DHT) with the provided code counts and values.
 * @param file Output stream for the JPEG file.
 * @param tableClass DC or AC class selector.
 * @param tableID Target Huffman table number.
 * @param counts Number of codes for each bit length.
 * @param values Huffman code symbols in order.
 */
void JPEGEncoder::write_dht(std::fstream &file, uint8_t tableClass, uint8_t tableID,
                            const uint8_t *counts, const uint8_t *values)
{
    uint16_t numValues = 0;
    for (int i = 0; i < 16; ++i)
        numValues += counts[i];

    uint16_t length = 2 + 1 + 16 + numValues; // length field itself + TC/TH byte + counts + values

    file.put(0xFF);
    file.put(0xC4);
    file.put((length >> 8) & 0xFF);
    file.put(length & 0xFF);
    file.put((tableClass << 4) | tableID);
    file.write(reinterpret_cast<const char *>(counts), 16);
    file.write(reinterpret_cast<const char *>(values), numValues);
}

/**
 * Writes the JPEG Start of Scan (SOS) marker and component table mapping.
 * @param file Output stream for the JPEG file.
 */
void JPEGEncoder::write_sos(std::fstream &file)
{
    file.put(0xFF);
    file.put(0xDA);
    file.put(0x00);
    file.put(0x0C); // length = 12

    file.put(0x03); // NS = 3 components

    // component 1: Y  -> DC table 0, AC table 0
    file.put(0x01);
    file.put(0x00);

    // component 2: Cb -> DC table 1, AC table 1
    file.put(0x02);
    file.put(0x11);

    // component 3: Cr -> DC table 1, AC table 1
    file.put(0x03);
    file.put(0x11);

    file.put(0x00); // Ss
    file.put(0x3F); // Se
    file.put(0x00); // Ah/Al
}

/**
 * Builds a JPEG header with quantization tables, Huffman tables, and the scan metadata.
 */
void JPEGEncoder::headify()
{
    std::fstream file("Image.jpeg", std::ios::out | std::ios::binary);

    // SOI
    file.put(0xFF);
    file.put(0xD8);
    // APP0  (NULL currently)

    // Quant Table
    ZigZagBlock lum = zigzag_scan(LUMINANCE_TABLE);
    ZigZagBlock chrom = zigzag_scan(CHROMINANCE_TABLE);

    // --- Luminance DQT ---
    file.put(0xFF);
    file.put(0xDB); // marker
    file.put(0x00);
    file.put(0x43); // length = 67
    file.put(0x00); // precision=0, table ID=0
    for (const auto &val : lum.data)
    {
        uint8_t byte = static_cast<uint8_t>(val);
        file.write(reinterpret_cast<const char *>(&byte), 1);
    }

    // --- Chrominance DQT ---
    file.put(0xFF);
    file.put(0xDB); // marker
    file.put(0x00);
    file.put(0x43); // length = 67
    file.put(0x01); // precision=0, table ID=1
    for (const auto &val : chrom.data)
    {
        uint8_t byte = static_cast<uint8_t>(val);
        file.write(reinterpret_cast<const char *>(&byte), 1);
    }

    // --- Start of Frame S0F0
    file.put(0xFF);
    file.put(0xC0);
    file.put(0x00);
    file.put(0x11);
    file.put(0x08); // precision
    file.put((img.height >> 8) & 0xFF);
    file.put(img.height & 0xFF);
    file.put((img.width >> 8) & 0xFF);
    file.put(img.width & 0xFF);
    file.put(0x03); // Color scale
    // component 1: Y
    file.put(0x01);
    file.put(0x11);
    file.put(0x00); // id, sampling 1x1, DQT 0
    // component 2: Cb
    file.put(0x02);
    file.put(0x11);
    file.put(0x01); // id, sampling, DQT 1
    // component 3: Cr
    file.put(0x03);
    file.put(0x11);
    file.put(0x01); // id, sampling, DQT 1

    HuffmanTable h;
    // --- Huffman Table ---
    write_dht(file, 0, 0, h.STD_LUMA_DC_BITS, h.STD_LUMA_DC_HUFFVAL);
    write_dht(file, 1, 0, h.STD_LUMA_AC_BITS, h.STD_LUMA_AC_HUFFVAL);
    write_dht(file, 0, 1, h.STD_CHROMA_DC_BITS, h.STD_CHROMA_DC_HUFFVAL);
    write_dht(file, 1, 1, h.STD_CHROMA_AC_BITS, h.STD_CHROMA_AC_HUFFVAL);

    // --- SOS ---
    write_sos(file);
    encode_image(file);

    // --- EOI ---
    file.put(0xFF);
    file.put(0xD9);
}

/**
 * Encodes each 8x8 block into JPEG bitstream data, including DCT, quantization, RLE, and Huffman coding.
 * @param file Output file receiving the compressed scan data.
 */
void JPEGEncoder::encode_image(std::fstream &file)
{
    int blocks_across = (img.width + 7) / 8;
    int blocks_down = (img.height + 7) / 8;

    HuffmanTable dc_table_y(true, false);
    dc_table_y.initialize();
    HuffmanTable ac_table_y(false, false);
    ac_table_y.initialize();
    HuffmanTable dc_table_c(true, true);
    dc_table_c.initialize();
    HuffmanTable ac_table_c(false, true);
    ac_table_c.initialize();

    BitWriter bitwriter; // ONE continuous stream for the entire scan

    for (int by = 0; by < blocks_down; by++)
    {
        for (int bx = 0; bx < blocks_across; bx++)
        {
            Block8x8 y_block = extract_block(img, bx, by, 0);
            Block8x8 cb_block = extract_block(img, bx, by, 1);
            Block8x8 cr_block = extract_block(img, bx, by, 2);

            Block8x8 y_dct = perform_dct(y_block);
            Block8x8 cb_dct = perform_dct(cb_block);
            Block8x8 cr_dct = perform_dct(cr_block);

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

            write_block_bits(bitwriter, y_rle, dc_table_y, ac_table_y);
            write_block_bits(bitwriter, cb_rle, dc_table_c, ac_table_c);
            write_block_bits(bitwriter, cr_rle, dc_table_c, ac_table_c);
        }
    }

    bitwriter.flush(); // pad final byte with 1s, stuff if needed

    const auto &bytes = bitwriter.data();
    file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}