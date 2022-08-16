#pragma once

#include "dll_export.h"

#include <vector>
#include <cstdint>

std::vector<int> DLL_EXPORT generate_interleaved_sector_list(int num_sector, int interleave = 1);
std::vector<uint8_t> DLL_EXPORT generate_format_data(size_t track_n, size_t side_n, size_t num_sector, size_t sect_len_code, size_t interleave = 1, size_t format_type = 1);
