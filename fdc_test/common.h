#pragma once

#include <vector>

#include "bit_array.h"

void dump_buf(uint8_t* ptr, size_t size, bool line_feed = true);
void bit_dump(const uint64_t data, size_t bit_width, size_t spacing = 0, bool line_feed = true);
void bit_dump(const bit_array &data, size_t bit_width = 0, size_t spacing = 0, bool line_feed = true);
void display_id_list(std::vector<fdc_bitstream::id_field> id_fields);
void repeat_data_generator(std::vector<std::pair<int, uint8_t>> inbuf, std::vector<uint8_t>& outbuf);
