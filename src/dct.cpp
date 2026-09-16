#include <cmath>
#include <iostream>

#include "dct.h"
#include "ppm.h"
#include "zigzag.h"

/**
 * Extracts a single 8x8 block from a Y/Cb/Cr channel, clamping coordinates at image bounds.
 * @param img Source image.
 * @param block_x Block column index.
 * @param block_y Block row index.
 * @param channel Channel selection: 0=Y, 1=Cb, 2=Cr.
 * @return The pixel block centered around the selected channel values.
 */
Block8x8 extract_block(const PPMImage &img, int block_x, int block_y, int channel)
{
    Block8x8 block;
    int start_x = block_x * 8;
    int start_y = block_y * 8;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            int pixel_x = start_x + x;
            int pixel_y = start_y + y;

            // Check if out of bounds
            if (pixel_x >= img.width)
                pixel_x = img.width - 1;
            if (pixel_y >= img.height)
                pixel_y = img.height - 1;

            // Map 2D coordinates to flat 1D vector index
            int flat_index = pixel_y * img.width + pixel_x;
            const auto &pixel = img.ycbcr_image[flat_index];

            float val = 0.0f;
            if (channel == 0)
                val = static_cast<float>(pixel.y);
            else if (channel == 1)
                val = static_cast<float>(pixel.cb);
            else
                val = static_cast<float>(pixel.cr);
            block.data[y][x] = val - 128.0f;
        }
    }

    return block;
}

/**
 * Builds the DCT cosine lookup table once for all subsequent transforms.
 * @param cos_table Precomputed cosine coefficients for 8x8 DCT evaluation.
 */
void initialize_table(float cos_table[8][8])
{

    for (int coord = 0; coord < 8; coord++)
    {
        for (int freq = 0; freq < 8; freq++)
        {
            cos_table[coord][freq] = std::cos((2.0f * coord + 1.0f) * freq * M_PI / 16.0f);
        }
    }
}

/**
 * Returns the normalization factor used by the DCT basis functions.
 * @param uv Frequency index along one axis.
 * @return 1/sqrt(2) for the DC term and 1 otherwise.
 */
float get_alpha(int uv)
{
    return (uv == 0) ? 0.70710678f : 1.0f; // 1 / sqrt(2)
}

/**
 * Converts an 8x8 pixel block into its frequency-domain DCT coefficients.
 * @param pixel_block Spatial-domain block before transformation.
 * @return DCT coefficients in the frequency domain.
 */
Block8x8 perform_dct(const Block8x8 &pixel_block)
{
    float cos_table[8][8];
    initialize_table(cos_table);

    Block8x8 freq_block;

    for (int u = 0; u < 8; u++)
    {
        for (int v = 0; v < 8; v++)
        {
            float sum = 0.0f;

            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    sum += pixel_block.data[x][y] * cos_table[x][u] * cos_table[y][v];
                }
            }
            float alpha_u = get_alpha(u);
            float alpha_v = get_alpha(v);
            freq_block.data[u][v] = 0.25f * alpha_u * alpha_v * sum;
        }
    }
    return freq_block;
}

/**
 * Prints the values of an 8x8 floating-point block for debugging.
 * @param b Block to display.
 */
void print_block(const Block8x8 &b)
{
    std::cout << "Block:" << std::endl;

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            std::cout << " " << b.data[x][y];
        }
        std::cout << "\n";
    }
}

/**
 * Prints the values of an 8x8 quantized block for debugging.
 * @param b Quantized block to display.
 */
void print_block(const QuantBlock8x8 &b)
{
    std::cout << "Quant Block:" << std::endl;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            std::cout << " " << static_cast<int>(b.data[x][y]);
        }
        std::cout << "\n";
    }
}

/**
 * Divides each DCT coefficient by the matching JPEG quantization value and rounds it.
 * @param dct Frequency-domain block.
 * @param q_table Quantization matrix for the selected channel.
 * @return Block containing quantized coefficients.
 */
QuantBlock8x8 quantize(const Block8x8 &dct, const int q_table[8][8])
{
    QuantBlock8x8 q_block;

    for (int u = 0; u < 8; u++)
    {
        for (int v = 0; v < 8; v++)
        {
            float unquantized = dct.data[u][v];
            float q_factor = static_cast<float>(q_table[u][v]);

            float result = std::round(unquantized / q_factor);
            q_block.data[u][v] = static_cast<int16_t>(result);
        }
    }

    return q_block;
}