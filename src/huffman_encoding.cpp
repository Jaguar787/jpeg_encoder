#include <cstdint>
#include <vector>
#include <unordered_map>

#include "rle.h"
#include "huffman_encoding.h"
#include "bitwriter.h"

/**
 * Initializes the Huffman lookup table for the selected DC/AC and luminance/chrominance profile.
 */
void HuffmanTable::initialize()
{
    if (is_dc)
    {
        if (is_cb)
            build(STD_CHROMA_DC_BITS, STD_CHROMA_DC_HUFFVAL);
        else
            build(STD_LUMA_DC_BITS, STD_LUMA_DC_HUFFVAL);
    }
    else
    {
        if (is_cb)
            build(STD_CHROMA_AC_BITS, STD_CHROMA_AC_HUFFVAL);
        else
            build(STD_LUMA_AC_BITS, STD_LUMA_AC_HUFFVAL);
    }
}

/**
 * Builds a Huffman lookup table from a canonical bit-length array and symbol list.
 * @param bits Number of codes for each bit length.
 * @param huffval Huffman symbol values ordered by the canonical code assignment.
 */
void HuffmanTable::build(const uint8_t bits[16], const uint8_t *huffval)
{
    uint32_t code = 0;
    int val = 0;

    for (int len = 1; len <= 16; ++len)
    {
        for (int i = 0; i < bits[len - 1]; ++i)
        {
            uint8_t sym = huffval[val++];
            lookup[sym] = {code, static_cast<uint8_t>(len)};
            code++;
        }
        code <<= 1;
    }
}

/**
 * Computes the bit-length category of a value for JPEG Huffman coding.
 * @param val Signed coefficient value.
 * @return Category index corresponding to the magnitude of the value.
 */
int HuffmanTable::get_category(int16_t val) const
{
    uint16_t abs_val = std::abs(val);
    int category = 0;
    while (abs_val > 0)
    {
        category++;
        abs_val >>= 1; // Shift right 1 bit
    }
    return category;
}

/**
 * Produces the bit pattern for a value within its Huffman category.
 * @param val Signed coefficient value.
 * @param category Huffman category index.
 * @return Encoded value bits matching the JPEG category convention.
 */
uint32_t HuffmanTable::get_value_bits(int16_t val, int category) const
{
    if (val >= 0)
    {
        return static_cast<uint32_t>(val);
    }
    else
    {
        return static_cast<uint32_t>(val - 1) & ((1U << category) - 1);
    }
}

/**
 * Encodes the DC and AC symbols of one block into the shared bitstream.
 * @param bitwriter Output bit writer that accumulates JPEG data.
 * @param block RLE-encoded block symbols.
 * @param dc_table Huffman table used for DC coefficients.
 * @param ac_table Huffman table used for AC coefficients.
 */
void write_block_bits(BitWriter &bitwriter, const EncodedBlockSymbols &block, const HuffmanTable &dc_table, const HuffmanTable &ac_table)
{
    int dc_cat = dc_table.get_category(block.dc_diff);

    HuffmanCode dc_huff = dc_table.lookup[dc_cat];
    bitwriter.write_bits(dc_huff.code, dc_huff.length);

    if (dc_cat > 0)
    {
        uint32_t val_bits = dc_table.get_value_bits(block.dc_diff, dc_cat);
        bitwriter.write_bits(val_bits, dc_cat);
    }

    for (const auto &sym : block.ac_syms)
    {
        // Special case: EOB (0,0)
        if (sym.runlength == 0 && sym.value == 0)
        {
            HuffmanCode eob_huff = ac_table.lookup[0x00];
            bitwriter.write_bits(eob_huff.code, eob_huff.length);
            break;
        }

        // Special case: ZRL (15, 0)
        if (sym.runlength == 15 && sym.value == 0)
        {
            HuffmanCode zrl_huff = ac_table.lookup[0xF0];
            bitwriter.write_bits(zrl_huff.code, zrl_huff.length);
            continue;
        }

        // Non-zero AC value
        int ac_cat = ac_table.get_category(sym.value);
        uint8_t composite_sym = (sym.runlength << 4) | (ac_cat & 0x0F);

        // Huffman code for the runlength and category
        HuffmanCode ac_huff = ac_table.lookup[composite_sym];
        bitwriter.write_bits(ac_huff.code, ac_huff.length);

        uint32_t val_bits = ac_table.get_value_bits(sym.value, ac_cat);
        bitwriter.write_bits(val_bits, ac_cat);
    }
}
