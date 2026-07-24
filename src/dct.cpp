#include <cmath>
#include <iostream>

#include "dct.h"
#include "ppm.h"
#include "zigzag.h"

bool table_initialized = false;

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

void initialize_table(float cos_table[8][8])
{
    if (table_initialized)
        return;
    for (int coord = 0; coord < 8; coord++)
    {
        for (int freq = 0; freq < 8; freq++)
        {
            cos_table[coord][freq] = std::cos((2.0f * coord + 1.0f) * freq * M_PI / 16.0f);
        }
    }
    table_initialized = true;
}

float get_alpha(int uv)
{
    return (uv == 0) ? 0.70710678f : 1.0f; // 1 / sqrt(2)
}

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
                    sum += pixel_block.data[y][x] * cos_table[x][u] * cos_table[y][v];
                }
            }
            float alpha_u = get_alpha(u);
            float alpha_v = get_alpha(v);
            freq_block.data[u][v] = 0.25f * alpha_u * alpha_v * sum;
        }
    }
    return freq_block;
}

void print_block(const Block8x8 &b)
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            std::cout << " " << b.data[x][y];
        }
        std::cout << "\n";
    }
}

void print_block(const QuantBlock8x8 &b)
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            std::cout << " " << static_cast<int>(b.data[x][y]);
        }
        std::cout << "\n";
    }
}

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
            q_block.data[u][v] = static_cast<uint16_t>(result);
        }
    }

    return q_block;
}