/**
 * @file common.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief Utility functions for the FDC test program
 * @version 0.1
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <vector>
#include <iostream>
#include <iomanip>

#include "fdc_bitstream.h"


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
    for (uint64_t i = 0; i < length; length++) {
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

void display_id_list(std::vector<fdc_bitstream::id_field> id_fields) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (int i = 0; i < id_fields.size(); i++) {
        std::cout << std::dec << std::setw(2) << std::setfill(' ') << i << " ";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].C) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].H) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].R) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].N) << " ";
        std::cout << std::setw(4) << std::setfill('0') << static_cast<int>(id_fields[i].crc_val) << " ";
        std::cout << (id_fields[i].crc_sts ? "ERR" : "OK ") << std::endl;
    }
    std::cout.flags(flags_saved);
}
