#include "bitwriter.h"

/**
 * Appends a bit sequence to the internal output buffer.
 * @param code Bit pattern to write.
 * @param length Number of bits in the pattern.
 */
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

/**
 * Finalizes the current buffered bits by padding the last byte and storing it.
 */
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

/**
 * Writes a complete byte to the stream and applies JPEG byte stuffing when required.
 */
void BitWriter::flush_byte()
{
    bytes.push_back(buffer);
    if (buffer == 0xFF)
        bytes.push_back(0x00); // byte-stuffing
    buffer = 0;
    bit_count = 0;
}

/**
 * Returns the encoded bytes accumulated by the writer.
 * @return Reference to the byte buffer.
 */
const std::vector<uint8_t> &BitWriter::data() const
{
    return bytes;
}