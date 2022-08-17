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
#include <fstream>
#include <vector>
#include <map>

#include <thread>

#include "fdc_bitstream.h"

#include "image_mfm.h"
#include "image_hfe.h"

#include "fdc_misc.h"
#include "common.h"


void test1(void) {
    std::cout << "*** MFM format data, and read test" << std::endl;
    fdc_bitstream fdc;
    disk_image_mfm image;
    try {
        image.read("test.mfm");
        bit_array barray = image.get_track_data(0);
        fdc.set_track_data(barray);
        fdc.set_pos(0);
        std::vector<uint8_t> track = fdc.read_track();
        dump_buf(track.data(), track.size() > 2048 ? 2048 : track.size());
        std::cout << std::endl;

        std::cout << "ID" << std::endl;
        std::vector<fdc_bitstream::id_field> ids = fdc.read_all_idam();
        display_id_list(ids);
        std::cout << std::endl;
    } catch(disk_image_exception cause) {
        std::cout << cause.what() << std::endl;
        std::cout << "error code:" << cause.get_error_code() << std::endl;
        return;
    }
    std::cout << "Writing 'test_write.mfm'." << std::endl;
    image.write("test_write.mfm");
}

void test2() {
    std::cout << "\nWrite track, write sector, read sector test" << std::endl;
    fdc_bitstream fdc;
    bit_array track_b;
    track_b.resize(4e6 * 0.2);               // extend track buffer (spindle 1 spin == 0.2sec)
    fdc.set_track_data(track_b);        // Set unformatted track data
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
    fdc.set_track_data(track_write_data);
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
    fdc.set_vfo_gain(2.f, 10.f);
    for (int i = 1; i <= 16; i++) {
        std::cout << "Sector " << i << std::endl;
        fdc_bitstream::sector_data sect_data_r;
        sect_data_r = fdc.read_sector(0, 0, i);
        dump_buf(sect_data_r.data.data(), sect_data_r.data.size());
    }
    std::cout << std::endl;

}

#include <random>

void test4(void) {
    std::cout << "Corocoro (unstable data) protect test" << std::endl;
    bit_array track;
    std::vector<uint8_t> format_data;
    std::vector<std::pair<int, uint8_t>> input_data = { {20, 0x4e}, {12, 0x00}, {3, 0xf5}, {1, 0xfe} };
    repeat_data_generator(std::vector<std::pair<int, uint8_t>>({ {20, 0x4e}, {12, 0x00}, {3, 0xf5}, {1, 0xfb}, {12, 0xe7}, {4, 0x00}, {12,0xe6}, {256, 0x00} }), format_data);
    fdc_bitstream fdc;
    track.set(4e6 * 0.2, 0);
    fdc.set_track_data(track);
    // Format the track
    fdc.write_track(format_data);

    track = fdc.get_track_data();
    int start = 20 + 12 + 3 + 1 + 12;

    // Generate random pulse in the track data to reproduce 'corocoro' (unstable data) protect
    std::random_device rnd_gen;
    for (int i = 0; i < 32; i++) {
        track.set(start * 16*8 + rnd_gen() % (4 * 16*8), 1);
    }
    fdc.set_track_data(track);

    // Read the sector data 32 times to check data fluctuation
    std::cout << "Read sector data" << std::endl;
    fdc.enable_fluctuator(1, 4);
    for (int i = 0; i < 32; i++) {
        fdc.set_pos(0);
        std::vector<uint8_t> sect_data;
        bool crc_error, dam_type, record_not_found;
        fdc.read_sector_body(00, sect_data, crc_error, dam_type, record_not_found);
        std::cout << std::setw(2) <<  i + 1 << " : ";
        dump_buf(sect_data.data(), 32);
    }
}

int main(void)
{
    test1();        // Read a HFM file / Read track / Dump sector IDs
    test2();        // Format a track (1 sector) / Read track / Write sector / Read track / Read sector
    test3();        // Format a track (16 sectors) / Dump sector IDs / Read sector / Write 16 sectors with data / Read Track / Read sector data 
    test4();        // 'Corocoro (unstable data)' protect test

    return 0;
}
