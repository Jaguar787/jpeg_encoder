#ifndef BITWRITER_H
#define BITWRITER_H
#include <cstdint>
#include <vector>

class BitWriter
{
public:
    void write_bits(uint32_t code, uint8_t length);
    void flush();
    const std::vector<uint8_t> &data() const;

private:
    void flush_byte();

    std::vector<uint8_t> bytes;
    uint8_t buffer = 0;
    int bit_count = 0;
};

#endif