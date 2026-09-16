#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <utility>

#include "ppm.h"
#include "encode.h"
#include "jpeg.h"

int main(int argc, char *argv[])
{
	PPMImage img;

	if (argc != 2)
	{
		std::cerr << "Missing image" << std::endl;
		return 0;
	}

	// Changed to a vector of pairs to prevent duplicate headers from overwriting each other
	std::vector<std::pair<uint16_t, std::vector<uint8_t>>> headers_and_bytes;

	std::ifstream file(argv[1], std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		std::cerr << "Error opening file" << std::endl;
		return 0;
	}

	img = load_ppm(file);
	color_shift(img);

	JPEGEncoder j(img);
	j.headify();

	// scan_raw_jpeg(file, headers_and_bytes);
	// print_data(headers_and_bytes);

	return 0;
}
