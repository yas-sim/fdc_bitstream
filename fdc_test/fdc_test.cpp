#include <stdio.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include <thread>

#include "bit_array.h"
#include "mfm_codec.h"
#include "fdc_bitstream.h"







void dump_buf(uint8_t *ptr, size_t size) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    for (size_t i = 0; i < size; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[i]) << " ";
        if (i % 64 == 63) {
            std::cout << std::endl;
        }
    }
    std::cout.flags(flags_saved);
}

void bit_dump(uint64_t data, size_t bit_width, size_t spacing = 0, bool line_feed=false) {
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


#include "image_raw.h"
#include "image_hfe.h"
#include "../hfe2mfm/image_mfm.h"




int main(void)
{
    //    read_raw_image("fb30.raw");
    std::vector<uint8_t> tbuf;
    bool crc_error = false;
    std::vector<uint8_t> id_field;

#if 0
    bit_array test1;
    test1.load("track.bin");
    //test1.dump(0, 256);

    //std::vector<uint8_t> tbuf;
    fdc_bitstream fdc;
    fdc.set_raw_track_data(test1);
    tbuf = fdc.read_track();
    //dump_buf(tbuf.data(), tbuf.size());
    dump_buf(tbuf.data(), 2048);
    std::cout << std::endl;


    fdc.clear_wraparound();
    fdc.set_pos(0);
    fdc.read_id(id_field, crc_error);

    std::vector<uint8_t> write_data = 
    //{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
    { 0x00, 0x00, 0x00, 0x00, 0xf5, 0xf5, 0xf5, 0xfe, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34 };
    for (auto it = write_data.begin(); it != write_data.end(); ++it) {
    //for(int i=0; i<256; i++) {
        fdc.write_data(*it, true);
    }

    fdc.set_pos(0);
    tbuf = fdc.read_track();
    dump_buf(tbuf.data(), 2048);
#endif

#if 0
    fdc.clear_wraparound();
    fdc.set_pos(0);
    do {
        fdc.read_id(id_field, crc_error);
        if (id_field.size() > 0) {
            dump_buf(id_field.data(), id_field.size());
            if (crc_error == true) {
                std::cout << "CRC ERR" << std::endl;
            }
            else {
                std::cout << "CRC OK" << std::endl;
            }
#if 0
            size_t length = id_field[3];
            std::vector<uint8_t> sect_data;
            bool dam_type;
            fdc.read_sector(length, sect_data, crc_error, dam_type);
            if (dam_type) {
                std::cout << "DDAM ";
            }
            else {
                std::cout << "DAM  ";
            }
            if (sect_data.size() > 0) {
                dump_buf(sect_data.data(), sect_data.size());
                std::cout << std::endl;
            }
#endif
        }
    } while (fdc.is_wraparound() == false);
#endif

#if 0
    disk_image_hfe image;
    image.read("nandemo.hfe");
    std::vector<std::vector<uint8_t>> &disk = image.get_track_data();
    std::vector<uint8_t> track = disk[0];
    bit_array hfe_stream;
    bit_array track_stream;

    hfe_stream.set_array(track);
    for (size_t bit = 0; bit < hfe_stream.get_length(); bit++) {
        int bit_data = hfe_stream.read_stream();
        for (size_t j = 0; j < 3; j++) track_stream.write_stream(0, true);
        track_stream.write_stream(bit_data, true);
        for (size_t j = 0; j < 4; j++) track_stream.write_stream(0, true);
    }
    fdc_bitstream fdc1;
    fdc1.set_raw_track_data(track_stream);
    fdc1.set_pos(0);
    tbuf = fdc1.read_track();
    dump_buf(tbuf.data(), 2048);

    fdc1.clear_wraparound();
    fdc1.set_pos(0);
    do {
        fdc1.read_id(id_field, crc_error);
        if (id_field.size() > 0) {
            dump_buf(id_field.data(), id_field.size());
            if (crc_error == true) {
                std::cout << "CRC ERR" << std::endl;
            }
            else {
                std::cout << "CRC OK" << std::endl;
            }
#if 0
            size_t length = id_field[3];
            std::vector<uint8_t> sect_data;
            bool dam_type;
            fdc1.read_sector(length, sect_data, crc_error, dam_type);
            if (dam_type) {
                std::cout << "DDAM ";
            }
            else {
                std::cout << "DAM  ";
            }
            if (sect_data.size() > 0) {
                dump_buf(sect_data.data(), sect_data.size());
                std::cout << std::endl;
            }
#endif
        }
    } while (fdc1.is_wraparound() == false);
#endif

#if 0
    disk_image_mfm mfm_img;
    mfm_img.read("cdos7.mfm");
    bit_array barray = mfm_img.get_track(2);
    fdc_bitstream fdc2;
    fdc2.set_raw_track_data(barray);
    fdc2.set_pos(0);
    tbuf = fdc2.read_track();
    dump_buf(tbuf.data(), 2048);
#endif


    std::cout << std::endl << std::endl << "TAIYO" << std::endl;
    disk_image_raw imgraw;
    imgraw.read("taiyo0.raw");
    fdc_bitstream fdc5;
    bit_array tdata = imgraw.get_track_data(0);
    fdc5.set_raw_track_data(tdata);
    tbuf = fdc5.read_track();
    //dump_buf(tbuf.data(), tbuf.size());
    //std::cout << std::endl;

    fdc5.clear_wraparound();
    fdc5.set_pos(0);
    for (int i = 0; i < 10; i++) {
        do {
            fdc5.read_id(id_field, crc_error);
            if (id_field.size() > 0) {
                size_t length = id_field[3];
                std::vector<uint8_t> sect_data;
                bool dam_type = false;
                if (id_field[2] == 0xf7) {
                    dump_buf(id_field.data(), id_field.size());
                    std::cout << std::endl;
                    fdc5.read_sector(length, sect_data, crc_error, dam_type);
                    if (sect_data.size() > 0) {
                        dump_buf(sect_data.data(), sect_data.size());
                        std::cout << std::endl;
                    }
                }
            }
        } while (fdc5.is_wraparound() == false);
        fdc5.clear_wraparound();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
