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
#include "image_mfm.h"

#include "common.h"

void list_sector_ids(fdc_bitstream &fdc, bool sector_dump) {
    std::vector<uint8_t> id_field;
    bool crc_error = false;
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
            if (sector_dump) {
                size_t length = id_field[3];
                std::vector<uint8_t> sect_data;
                bool dam_type = false;
                fdc.read_sector(length, sect_data, crc_error, dam_type);
                std::cout << (dam_type ? "DDAM " : "DAM  ") << (crc_error ? "CRC ERR" : "CRC OK ") << std::endl;
                if (sect_data.size() > 0) {
                    dump_buf(sect_data.data(), sect_data.size());
                    std::cout << std::endl;
                }
            }
        }
    } while (fdc.is_wraparound() == false);
}



int main(void)
{
    std::vector<uint8_t> id_field;
    bool crc_error = false;
    //    read_raw_image("fb30.raw");
    std::vector<uint8_t> tbuf;

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
    std::cout << std::endl;

    list_sector_ids(fdc);
#endif

#if 1
    std::cout << "*** NANDEMO" << std::endl;
    disk_image_hfe image;
    image.read("nandemo.hfe");
    bit_array track_stream = image.get_track_data(0);
    fdc_bitstream fdc1;
    fdc1.set_raw_track_data(track_stream);
    fdc1.set_pos(0);
    tbuf = fdc1.read_track();
    dump_buf(tbuf.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    list_sector_ids(fdc1, false);
#endif


#if 1
    std::cout << "*** CDOS7" << std::endl;
    disk_image_mfm mfm_img;
    mfm_img.read("cdos7.mfm");
    bit_array barray = mfm_img.get_track_data(2);
    fdc_bitstream fdc2;
    fdc2.set_raw_track_data(barray);
    fdc2.set_pos(0);
    tbuf = fdc2.read_track();
    dump_buf(tbuf.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    list_sector_ids(fdc2, true);
#endif


    std::cout << std::endl << std::endl << "*** TAIYO - COROCORO test" << std::endl;
    disk_image_raw imgraw;
    imgraw.read("taiyo0.raw");
    fdc_bitstream fdc5;
    bit_array tdata = imgraw.get_track_data(0);
    fdc5.set_raw_track_data(tdata);
    tbuf = fdc5.read_track();
    //dump_buf(tbuf.data(), tbuf.size());
    //std::cout << std::endl;

    list_sector_ids(fdc5, false);

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
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }



    std::cout << "\nWrite track / Write sector test" << std::endl;
    bit_array track_write_data;
    track_write_data.set(4e6 * 0.2, 0);     // extend track buffer
    fdc_bitstream fdc6;
    fdc6.set_raw_track_data(track_write_data);
    std::vector<uint8_t> write_data = generate_format_data(0,0,1,1);
    fdc6.write_track(write_data);
    fdc6.set_pos(0);
    tbuf = fdc6.read_track();
    dump_buf(tbuf.data(), tbuf.size());
    std::cout << std::endl;

    fdc6.clear_wraparound();
    fdc6.set_pos(0);
    fdc6.read_id(id_field, crc_error);
    if (id_field.size() > 0) {
        std::cout << std::endl << "ID" << std::endl;
        dump_buf(id_field.data(), id_field.size());
        std::cout << std::endl;
        size_t length = id_field[3];
        std::vector<uint8_t> sect_data;
        bool dam_type = false;
        // read sector
        fdc6.read_sector(length, sect_data, crc_error, dam_type);
        std::cout << (crc_error ? "ERROR" : "OK") << std::endl;
        if (sect_data.size() > 0) {
            std::cout << std::endl << "SECTOR DATA" << std::endl;
            dump_buf(sect_data.data(), sect_data.size());
            std::cout << std::endl;
        }
    }

    // write sector
    std::vector<uint8_t> sect_data;
    for (int i = 0; i < 256; i++) sect_data.push_back(i);
    fdc6.clear_wraparound();
    fdc6.set_pos(0);
    fdc6.read_id(id_field, crc_error);
    fdc6.write_sector(sect_data, false);

    fdc6.set_pos(0);
    tbuf = fdc6.read_track();
    dump_buf(tbuf.data(), tbuf.size());
    std::cout << std::endl;

    fdc6.clear_wraparound();
    fdc6.set_pos(0);
    fdc6.read_id(id_field, crc_error);
    if (id_field.size() > 0) {
        std::cout << std::endl << "ID" << std::endl;
        dump_buf(id_field.data(), id_field.size());
        std::cout << std::endl;
        size_t length = id_field[3];
        std::vector<uint8_t> sect_data;
        bool dam_type = false;
        // read sector after a write sector with incremental data
        fdc6.read_sector(length, sect_data, crc_error, dam_type);
        std::cout << (crc_error ? "ERROR" : "OK") << std::endl;
        if (sect_data.size() > 0) {
            std::cout << std::endl << "SECTOR DATA" << std::endl;
            dump_buf(sect_data.data(), sect_data.size());
            std::cout << std::endl;
        }
    }



    std::cout << "\nWrite track - full format test" << std::endl;
    //bit_array track_write_data;
    track_write_data.clear_array();
    track_write_data.set(4e6 * 0.2, 0);     // extend track buffer
    fdc_bitstream fdc7;
    fdc7.set_raw_track_data(track_write_data);
    write_data = generate_format_data(0, 0, 16, 1, 3);
    fdc7.write_track(write_data);
    fdc7.set_pos(0);
    tbuf = fdc7.read_track();
    dump_buf(tbuf.data(), tbuf.size());
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    list_sector_ids(fdc7, false);

    return 0;
}
