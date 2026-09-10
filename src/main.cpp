#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <utility>

#include "ppm.h"
#include "encode.h"

int byte_check(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &data)
{
	int bytes = 0;
	for (const auto &[key, value] : data)
	{
		bytes += value.size() + 2; // Plus 2 for headers
	}
	return bytes;
}

void print_data(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &data)
{
	// Print out everything safely preserving duplicate structural fields
	std::cout << "Beginning execution: " << std::endl;
	for (const auto &[key, value] : data)
	{
		std::cout << "Header: ";
		std::cout << std::hex << static_cast<int>(key) << std::dec << "\n";
		for (const auto &val : value)
		{
			std::cout << "\t" << std::hex << static_cast<int>(val) << std::dec << " ";
		}
		std::cout << "\n";
	}

	std::cout << "Total bytes tracked = " << byte_check(data) << std::endl;
}

void scan_raw_jpeg(std::ifstream &file, std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &headers_and_bytes)
{
	const uint16_t START_BYTES = 0xFFD8;
	uint8_t current_byte = 0;
	uint8_t first_byte = 0x00;
	uint8_t second_byte = 0x00;

	// Read the very first header safely
	file.read(reinterpret_cast<char *>(&first_byte), 1);
	file.read(reinterpret_cast<char *>(&second_byte), 1);

	if (((first_byte << 8) | (second_byte & 0xFF)) != START_BYTES)
	{
		std::cout << "Format not supported" << std::endl;
	}

	while (file)
	{
		uint16_t section_header = (first_byte << 8) | (second_byte & 0xFF);
		std::vector<uint8_t> bytes = {};

		if (section_header == 0xFFD8 || section_header == 0xFFD9 ||
			(section_header >= 0xFFD0 && section_header <= 0xFFD7) ||
			section_header == 0xFF01)
		{
			headers_and_bytes.push_back({section_header, bytes});
			// Read next header bytes directly
			file.read(reinterpret_cast<char *>(&first_byte), 1);
			file.read(reinterpret_cast<char *>(&second_byte), 1);
			continue;
		}
		else if (section_header == 0xFFDA)
		{
			uint8_t b;
			bool found_next_marker = false;

			while (file.read(reinterpret_cast<char *>(&b), 1))
			{
				if (b == 0xFF)
				{
					uint8_t next;
					// Handle potential multiple 0xFF padding bytes
					while (file.read(reinterpret_cast<char *>(&next), 1))
					{
						if (next != 0xFF)
							break;
						// If it is an extra 0xFF, save it into the payload stream
						bytes.push_back(0xFF);
					}

					if (!file)
						break;

					if (next == 0x00)
					{
						// Stuffed byte: save both to payload
						bytes.push_back(0xFF);
						bytes.push_back(0x00);
					}
					else if (next >= 0xD0 && next <= 0xD7)
					{
						// Restart marker: save both to payload and CONTINUE scanning data
						bytes.push_back(0xFF);
						bytes.push_back(next);
					}
					else
					{
						// Structural marker found (like EOI 0xFFD9)!
						// Set up targets for the next iteration of the main loop
						first_byte = 0xFF;
						second_byte = next;

						// Balance out the counter tracking because the next
						// iteration of the outer loop adds 2 for these exact bytes
						found_next_marker = true;
						break;
					}
				}
				else
				{
					bytes.push_back(b);
				}
			}
			headers_and_bytes.push_back({section_header, bytes});

			if (!found_next_marker)
			{
				break;
			}
		}
		else
		{
			uint8_t high_byte = 0x00;
			uint8_t low_byte = 0x00;

			file.read(reinterpret_cast<char *>(&high_byte), 1);
			file.read(reinterpret_cast<char *>(&low_byte), 1);

			if (!file)
				break;
			// Ensure that these bytes are inserted (even if not "helpful" past now)
			bytes.push_back(high_byte);
			bytes.push_back(low_byte);

			uint16_t length_byte = (high_byte << 8) | low_byte;
			int payload_size = length_byte - 2;

			for (int i = 0; i < payload_size; i++)
			{
				if (!file.read(reinterpret_cast<char *>(&current_byte), 1))
					break;
				bytes.push_back(current_byte);
			}
			headers_and_bytes.push_back({section_header, bytes});

			// Read next header bytes directly
			file.read(reinterpret_cast<char *>(&first_byte), 1);
			file.read(reinterpret_cast<char *>(&second_byte), 1);
		}
	}
}

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

	// img = load_ppm(file);
	// color_shift(img);

	// JPEGEncoder j(img);
	// j.headify();

	scan_raw_jpeg(file, headers_and_bytes);
	print_data(headers_and_bytes);

	return 0;
}
