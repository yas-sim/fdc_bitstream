/**
 * @file fdc_test.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief FDC-bitstream library test program
 * @version 0.1
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

#include <thread>

#include "fdc_bitstream.h"

#include "image_mfm.h"

#include "fdc_misc.h"
#include "common.h"


void test1(void) {
    std::cout << "*** MFM format data, and read test" << std::endl;
    fdc_bitstream fdc;
    disk_image_mfm image;
    image.read("test.mfm");
    bit_array barray = image.get_track_data(0);
    fdc.set_raw_track_data(barray);
    fdc.set_pos(0);
    std::vector<uint8_t> track = fdc.read_track();
    dump_buf(track.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    std::vector<fdc_bitstream::id_field> ids = fdc.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;
}

void test2() {
    std::cout << "\nWrite track, write sector, read sector test" << std::endl;
    fdc_bitstream fdc;
    bit_array track_b;
    track_b.resize(4e6 * 0.2);               // extend track buffer (spindle 1 spin == 0.2sec)
    fdc.set_raw_track_data(track_b);        // Set unformatted track data
    std::vector<uint8_t> write_data = generate_format_data(0, 0, 1, 1);       // trk=0, sid=0, #sec=1, len=1
    fdc.write_track(write_data);

    std::cout << "Track dump - after formatting" << std::endl;
    fdc.set_pos(0);
    std::vector<uint8_t> track = fdc.read_track();
    dump_buf(track.data(), track.size());
    std::cout << std::endl;

    fdc.clear_wraparound();
    fdc.set_pos(0);

    // write sector
    std::vector<uint8_t> sect_data_w;
    for (int i = 0; i < 256; i++) sect_data_w.push_back(i);
    fdc.clear_wraparound();
    fdc.set_pos(0);
    fdc.write_sector(0, 0, 1, false, sect_data_w);

    std::cout << "Track dump - after sector write" << std::endl;
    fdc.set_pos(0);
    track = fdc.read_track();
    dump_buf(track.data(), track.size());
    std::cout << std::endl;

    // verify written data by reading the sector
    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sect_data_r;
    sect_data_r = fdc.read_sector(0, 0, 1);
    dump_buf(sect_data_r.data.data(), sect_data_r.data.size());
    std::cout << std::endl;

}

void test3(void) {
    std::cout << "\nFormat test - format (track write), write all sectors, and read all sectors" << std::endl;
    fdc_bitstream fdc;
    bit_array track_write_data;
    track_write_data.clear_array();
    track_write_data.set(4e6 * 0.2, 0);     // extend track buffer
    fdc.set_raw_track_data(track_write_data);
    std::vector<uint8_t> write_data = generate_format_data(0, 0, 16, 1, 3);
    fdc.write_track(write_data);

    fdc.set_pos(0);
    std::vector<uint8_t> track = fdc.read_track();
    dump_buf(track.data(), track.size());
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    std::vector<fdc_bitstream::id_field> ids = fdc.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sect_data_r;
    sect_data_r = fdc.read_sector(0, 0, 1);
    dump_buf(sect_data_r.data.data(), sect_data_r.data.size());
    std::cout << std::endl;

    std::cout << "Write to all sectors (0-16)" << std::endl;
    std::vector<uint8_t> sect_data_w;
    for (int i = 1; i <= 16; i++) {
        sect_data_w.clear();
        sect_data_w.resize(256, ((i - 1) << 4 | (i - 1)));
        bool sts = fdc.write_sector(0, 0, i, false, sect_data_w, true);  // enable fluctuator
        if (sts == false) {
            std::cout << i << "record-not-found error" << std::endl;
        }
    }

    std::cout << "Track dump - after all sector write" << std::endl;
    fdc.set_pos(0);
    track = fdc.read_track();
    dump_buf(track.data(), track.size());
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    for (int i = 1; i <= 16; i++) {
        std::cout << "Sector " << i << std::endl;
        fdc_bitstream::sector_data sect_data_r;
        sect_data_r = fdc.read_sector(0, 0, i);
        dump_buf(sect_data_r.data.data(), sect_data_r.data.size());
    }
    std::cout << std::endl;

}

int main(void)
{
    test1();        // Read a HFM file / Read track / Dump sector IDs
    test2();        // Format a track (1 sector) / Read track / Write sector / Read track / Read sector
    test3();        // Format a track (16 sectors) / Dump sector IDs / Read sector / Write 16 sectors with data / Read Track / Read sector data 

    return 0;
}
