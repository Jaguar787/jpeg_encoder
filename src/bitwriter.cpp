#include "bitwriter.h"

void BitWriter::write_bits(uint32_t code, uint8_t length)
{
    for (int i = length - 1; i >= 0; --i)
    {
        uint8_t bit = (code >> i) & 1;
        buffer = (buffer << 1) | bit;
        ++bit_count;

        if (bit_count == 8)
        {
            flush_byte();
        }
    }
}

void BitWriter::flush()
{
    if (bit_count > 0)
    {
        buffer <<= (8 - bit_count);
        buffer |= (0xFF >> bit_count); // pad with 1s
        bytes.push_back(buffer);
        if (buffer == 0xFF)
            bytes.push_back(0x00);
        buffer = 0;
        bit_count = 0;
    }
}

void BitWriter::flush_byte()
{
    bytes.push_back(buffer);
    if (buffer == 0xFF)
        bytes.push_back(0x00); // byte-stuffing
    buffer = 0;
    bit_count = 0;
}

const std::vector<uint8_t> &BitWriter::data() const
{
    return bytes;
}