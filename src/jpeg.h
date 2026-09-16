#include <iostream>
#include <vector>
#include <fstream>

int byte_check(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &data);
void print_data(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &data);
void scan_raw_jpeg(std::ifstream &file, std::vector<std::pair<uint16_t, std::vector<uint8_t>>> &headers_and_bytes);