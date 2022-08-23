/**
 * @file fdc_misc.cpp
 * @author your name (you@domain.com)
 * @brief Miscellaneous functions for FDC lib (format data generator)
 * @version 0.1
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <algorithm>

#include "fdc_misc.h"

namespace fdc_misc {

/**
 * @brief Generate a list of interleaved sector numbers.
 * 
 * @param num_sector Number of sectors.
 * @param interleave Interleave distance to the next sector. Default=1.
 * @return std::vector<int> Generated sector number.
 */
std::vector<int> generate_interleaved_sector_list(size_t num_sector, int interleave = 1) {
    std::vector<int> sector_list(num_sector, 0);
    int pos = 0;
    for (int i = 1; i <= num_sector; i++) {
        while (sector_list[pos] != 0) {
            if (++pos >= num_sector) pos = 0;
        }
        sector_list[pos] = i;
        pos += interleave;
        if (pos >= num_sector) pos = 0;
    }
    return sector_list;
}

/**
* @brief Format data generator. Generates format data from the given parameters.
* 
* @param[in] track_n Track number.
* @param[in] side_n Side number.
* @param[in] num_sector Number of sectors in the track (starting from 1).
* @param[in] sect_len_code Sector length code (0-3 -> 128/256/512/1024).
* @param[in] interleave Interleave parameter. Meaning the distance to the next sector. 1=no interleave. 1>interleave.
* @param[in] format_type 0:IBM format, 1:ECMA/ISO format (default).
* @return std::vector<uint8_t> Generated format data.
*/
std::vector<uint8_t> generate_format_data(size_t track_n, size_t side_n, size_t num_sector, size_t sect_len_code, size_t interleave, size_t format_type) {
    std::vector<int> sector_list = generate_interleaved_sector_list(num_sector, interleave);
    std::vector<uint8_t> format_data;
    std::vector<size_t> sector_length_table{ 128, 256, 512, 1024 };
    switch (format_type) {
    case 0: { /** IBM format */
        std::vector<size_t> gap3_table{ 54,54,84,116 };
        std::vector<size_t> gap4b_table{ 152,152,182,94 };

        for (size_t i = 0; i < 80; i++) format_data.push_back(0x4e);     //Gap4a
        for (size_t i = 0; i < 12; i++) format_data.push_back(0x00);     //SYNC
        for (size_t i = 0; i < 3; i++) format_data.push_back(0xf6);      // IAM, f6->c2
        format_data.push_back(0xfc);
        for (size_t i = 0; i < 50; i++) format_data.push_back(0x4e);     //Gap1

        for (size_t n = 0; n < num_sector; n++) {
            for (size_t i = 0; i < 12; i++) format_data.push_back(0x00); //SYNC
            for (size_t i = 0; i < 3; i++) format_data.push_back(0xf5);  //IDAM f5->a1
            format_data.push_back(0xfe);
            format_data.push_back(track_n);                 //C
            format_data.push_back(side_n);                  //H
            format_data.push_back(sector_list[n]);          //R
            format_data.push_back(sect_len_code & 0x03u);   //N
            format_data.push_back(0xf7);                    //CRC f7->crc
            for (size_t i = 0; i < 22; i++) format_data.push_back(0x4e);        //Gap2

            for (size_t i = 0; i < 12; i++) format_data.push_back(0x00);        //SYNC
            for (size_t i = 0; i < 3; i++) format_data.push_back(0xf5);  //DAM f5->a1
            format_data.push_back(0xfb);
            for (size_t i = 0; i < sector_length_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x00);  //Sector data
            format_data.push_back(0xf7);                    //CRC f7->crc
            for (size_t i = 0; i < gap3_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x4e);        //Gap3
        }
        for (size_t i = 0; i < gap4b_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x4e);        //Gap4b
        }
        break;
    case 1: { /** ECMA/ISO format */
        std::vector<size_t> gap3_table{ 54,54,84,116 };
        std::vector<size_t> gap4_table{ 226,226,296,208 };

        for (size_t i = 0; i < 32; i++) format_data.push_back(0x4e);     //Gap1

        for (size_t n = 0; n < num_sector; n++) {
            for (size_t i = 0; i < 12; i++) format_data.push_back(0x00); //SYNC
            for (size_t i = 0; i < 3; i++) format_data.push_back(0xf5);  //IDAM f5->a1
            format_data.push_back(0xfe);
            format_data.push_back(track_n);                 //C
            format_data.push_back(side_n);                  //H
            format_data.push_back(sector_list[n]);          //R
            format_data.push_back(sect_len_code & 0x03u);   //N
            format_data.push_back(0xf7);                    //CRC f7->crc
            for (size_t i = 0; i < 22; i++) format_data.push_back(0x4e);        //Gap2

            for (size_t i = 0; i < 12; i++) format_data.push_back(0x00);        //SYNC
            for (size_t i = 0; i < 3; i++) format_data.push_back(0xf5);  //DAM f5->a1
            format_data.push_back(0xfb);
            for (size_t i = 0; i < sector_length_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x00);  //Sector data
            format_data.push_back(0xf7);                    //CRC f7->crc
            for (size_t i = 0; i < gap3_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x4e);        //Gap3
        }
        for (size_t i = 0; i < gap4_table[sect_len_code & 0x03u]; i++) format_data.push_back(0x4e);        //Gap4
        break;
        }
    default:
        /** Wrong type */
        break;
    }
    return format_data;
}

// ------------------------------------------------------------------------------------------

void display_histogram(const std::vector<size_t> &dist_array) {
    size_t max_count = *std::max_element(dist_array.begin(), dist_array.end());
    double scale = 100.f / static_cast<double>(max_count);
    size_t max_item = 0;
    for(size_t i=0; i<dist_array.size(); i++) {
        if (dist_array[i] != 0) max_item = i;
    }

    // display histogram
    for(size_t i=0; i<=max_item; i++) {
        std::cout << std::setw(4) << std::setfill(' ') << i << " : ";
        std::cout << std::setw(8) << std::setfill(' ') << dist_array[i] << " : ";
        size_t bar_length = static_cast<size_t>(static_cast<double>(dist_array[i]) * scale); // normalize
        std::cout << std::string(bar_length, '*') << std::endl;
    }
}

std::vector<size_t> get_frequent_distribution(bit_array barray) {
    barray.clear_wraparound_flag();
    std::vector<size_t> freq_dist;
    do {
        size_t dist = barray.distance_to_next_pulse();
        if(freq_dist.size() <= dist) {
            freq_dist.resize(dist+1);
        }
        freq_dist[dist]++;
    } while(!barray.is_wraparound());
    return freq_dist;
}

std::vector<size_t> find_peaks(const std::vector<size_t> &dist_freq) {
    // LPF
    std::vector<size_t> avg(dist_freq.size());
    const int filt[5] { 1,2,3,2,1 };
    const int flt_sum = 1+2+3+2+1;
    for(int i = 0; i<dist_freq.size(); i++) {
        size_t avg_tmp = 0;
        for(int j=-2; j<=2; j++) {
            int pos=0;
            if (i+j < 0) {
                pos= -(i+j);                            // data mirroring (lower boundary)
            } else if(i+j >= dist_freq.size()) {
                pos = 2*dist_freq.size() - (i+j) - 2;   // data mirroring (upper boundary)
            } else {
                pos = i+j;
            }
            avg_tmp += dist_freq[pos] * filt[j+2];
        }
        avg[i] = avg_tmp / flt_sum;
    }
    //fdc_misc::display_histogram(avg);       // for debug purpose

    // detect peaks
    std::vector<std::pair<size_t, size_t>> peaks;
    bool flat_peak = false;
    size_t lower_edge = 0;
    for(size_t i=1; i<avg.size()-1-1; i++) {
        if(avg[i-1] < avg[i] && avg[i+1] < avg[i]) {    // a sharp peak
            peaks.push_back(std::pair<size_t, size_t>(i, avg[i]));
            flat_peak = false;
        } else if(avg[i-1] < avg[i] && avg[i+1] == avg[i])  { // lower edge of a possible flat peak
            flat_peak = true;
            lower_edge = i;
        } else if(avg[i-1] == avg[i] && avg[i+1] < avg[i] && flat_peak) {   // upper edge of a flat peak
            peaks.push_back(std::pair<size_t, size_t>((i+lower_edge)/2, avg[i]));
            flat_peak = false;
        } else if(avg[i-1] == avg[i] && avg[i+1] == avg[i] && flat_peak) {       // top of flat peak
            // nop
        } else {        // neither sharp peak, top of flat peak, nor edge of flat peak.
            flat_peak = false;
        }
    }

    // sort peaks by value (frequency)
    std::sort(peaks.begin(), peaks.end(), [](const std::pair<size_t, size_t> &left, const std::pair<size_t, size_t> &right) { return left.second > right.second; });

    if (peaks.size()<3) {
        peaks.resize(3);
    }
    std::vector<size_t> result;
    for(auto it=peaks.begin(); it != peaks.begin()+3; ++it) {
        result.push_back((*it).first);
    }

    // sort peaks by index
    std::sort(result.begin(), result.end(), [](const size_t &left, const size_t &right) { return left < right; });

    return result;
}

std::vector<size_t> convert_to_dist_array(bit_array track) {
    track.clear_wraparound_flag();
    std::vector<size_t> dist_array;
    do {
        size_t dist = track.distance_to_next_pulse();
        dist_array.push_back(dist);
    } while(!track.is_wraparound());
    return dist_array;
}


void dump_buf(uint8_t* ptr, size_t size, bool line_feed /*= true*/) {
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

void bit_dump(const uint64_t data, size_t bit_width, size_t spacing /*= 0*/, bool line_feed /*= true*/) {
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

void bit_dump(bit_array &data, size_t bit_width /*= 0*/, size_t spacing /*= 0*/, bool line_feed /*=true*/) {
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

void display_sector_data(const fdc_bitstream::sector_data &sect_data) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    std::cout << std::dec << std::setw(4) << std::setfill(' ');
    std::cout << sect_data.data.size() << " ";
    std::cout << (sect_data.dam_type ? "DDAM " : "DAM  ");
    std::cout << (sect_data.crc_sts ? "DT-CRC_ERR " : "DT-CRC OK  ");
    std::cout << (sect_data.record_not_found ? "RNF_ERR " : "RNF_OK  ");
    std::cout << std::dec << std::setw(8);
    std::cout << "IDAM_POS=" << std::setw(8) << sect_data.id_pos << " ";
    std::cout << "DAM_POS="  << std::setw(8) << sect_data.data_pos << " ";
    std::cout.flags(flags_saved);
}

void display_id(const fdc_bitstream::id_field &id) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(id.C) << " ";
    std::cout << std::setw(2) << static_cast<int>(id.H) << " ";
    std::cout << std::setw(2) << static_cast<int>(id.R) << " ";
    std::cout << std::setw(2) << static_cast<int>(id.N) << " ";
    std::cout << std::setw(4) << static_cast<int>(id.crc_val) << " ";
    std::cout << (id.crc_sts ? "ID-CRC_ERR " : "ID-CRC_OK  ");
    std::cout.flags(flags_saved);
}

void display_id_list(const std::vector<fdc_bitstream::id_field> &id_fields) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (int i = 0; i < id_fields.size(); i++) {
        std::cout << std::dec << std::setw(2) << std::setfill(' ') << i << " ";
        display_id(id_fields[i]);
        std::cout << std::endl;
    }
    std::cout.flags(flags_saved);
}

size_t str2val(const std::string &hexstr) {
    size_t res = 0;
    if (hexstr.size()==0) return 0;
    if (hexstr[0] == '$') {
        for(auto it = hexstr.begin(); it != hexstr.end(); ++it) {
            if (*it >='0' && *it <= '9') {
                res = (res<<4) | ((*it) - '0');
            } else if (*it>='a' && *it <= 'f') {
                res = (res<<4) | ((*it) - 'a' + 10);
            }
        } 
    } 
    else if (hexstr[0] >= '0' && hexstr[0] <= '9') {
        res = std::stoi(hexstr);
    }
    //std::cout << hexstr << " " << res << std::endl;
    return res;
}

}
