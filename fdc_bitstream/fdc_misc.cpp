#include "fdc_misc.h"

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

std::vector<uint8_t> generate_format_data(size_t track_n, size_t side_n, size_t num_sector, size_t sect_len_code, size_t interleave) {
    std::vector<int> sector_list = generate_interleaved_sector_list(num_sector, interleave);
    std::vector<uint8_t> format_data;
    std::vector<size_t> sector_length_table{ 128, 256, 512, 1024 };
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

    return format_data;
}
