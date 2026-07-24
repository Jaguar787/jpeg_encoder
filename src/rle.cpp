#include <vector>

#include "encode.h"
#include "zigzag.h"
#include "rle.h"

EncodedBlockSymbols encode_rle(const ZigZagBlock &z, int16_t prev_dc)
{
    EncodedBlockSymbols enc;

    // 1. Correct Signed Delta DC (Current - Previous)
    enc.dc_diff = z.data[0] - prev_dc;

    int zero_run = 0;

    // 2. Iterate through AC coefficients (indices 1 to 63)
    for (int i = 1; i < 64; i++)
    {
        int16_t val = z.data[i];

        if (val == 0)
        {
            zero_run++;
        }
        else
        {
            while (zero_run >= 16)
            {
                ACSymbol zrl;
                zrl.runlength = 15;
                zrl.value = 0;
                enc.ac_syms.push_back(zrl);
                zero_run -= 16;
            }

            ACSymbol ac;
            ac.runlength = static_cast<uint8_t>(zero_run);
            ac.value = val;
            enc.ac_syms.push_back(ac);

            // Reset zero count
            zero_run = 0;
        }
    }

    // 3. If trailing zeros remain at the end of the block, push EOB (0, 0)
    if (zero_run > 0)
    {
        ACSymbol eob;
        eob.runlength = 0;
        eob.value = 0;
        enc.ac_syms.push_back(eob);
    }

    return enc;
}