// Utility functions

#include <vector>
#include <iostream>
#include <iomanip>

#include "bit_array.h"


void dump_buf(uint8_t* ptr, size_t size, bool line_feed = true) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    for (size_t i = 0; i < size; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[i]) << " ";
        if (i % 64 == 63) {
            std::cout << std::endl;
        }
    }
    if (line_feed) std::cout << std::endl;
    std::cout.flags(flags_saved);
}

void bit_dump(const uint64_t data, size_t bit_width, size_t spacing = 0, bool line_feed = true) {
    size_t count = 0;
    for (uint64_t bit_pos = 1 << (bit_width - 1); bit_pos != 0; bit_pos >>= 1) {
        std::cout << ((data & bit_pos) ? 1 : 0);
        count++;
        if (spacing > 0) {
            if (count % spacing == 0) {
                std::cout << " ";
            }
        }
    }
    if (line_feed == true) {
        std::cout << std::endl;
    }
}

void bit_dump(bit_array &data, size_t bit_width = 0, size_t spacing = 0, bool line_feed = true) {
    size_t count = 0;
    size_t length = (bit_width == 0) ? data.get_length() : bit_width;
    for (uint64_t i; i < length; length++) {
        std::cout << (data.get(i) ? 1 : 0);
        count++;
        if (spacing > 0) {
            if (count % spacing == 0) {
                std::cout << " ";
            }
        }
    }
    if (line_feed == true) {
        std::cout << std::endl;
    }
}


