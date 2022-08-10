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

#include "image_raw.h"
#include "image_hfe.h"
#include "image_mfm.h"

#include "fdc_misc.h"
#include "common.h"


int main(void)
{
#if 1
    std::cout << "*** HFE format read test - NANDEMO" << std::endl;
    fdc_bitstream fdc1;
    disk_image_hfe image1;
    image1.read("nandemo.hfe");
    bit_array track_stream = image1.get_track_data(0);
    fdc1.set_raw_track_data(track_stream);
    fdc1.set_pos(0);
    std::vector<uint8_t> track1 = fdc1.read_track();
    dump_buf(track1.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    std::vector<fdc_bitstream::id_field> id1;
    id1 = fdc1.read_all_idam();
    display_id_list(id1);
    std::cout << std::endl;
#endif


#if 1
    std::cout << "*** MFM format data read test - CDOS7" << std::endl;
    fdc_bitstream fdc2;
    disk_image_mfm image2;
    image2.read("cdos7.mfm");
    bit_array barray = image2.get_track_data(0);
    fdc2.set_raw_track_data(barray);
    fdc2.set_pos(0);
    std::vector<uint8_t> track2 = fdc2.read_track();
    dump_buf(track2.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    std::vector<fdc_bitstream::id_field> ids = fdc2.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;
#endif


#if 1
    std::cout << std::endl << std::endl << "*** TAIYO - COROCORO protect test" << std::endl;
    fdc_bitstream fdc3;
    disk_image_raw image3;
    image3.read("taiyo0.raw");
    bit_array tdata = image3.get_track_data(0);

    // Set fluctuation parameters for COROCORO protect
    fdc3.set_raw_track_data(tdata);
    fdc3.enable_fluctuator(2, 5);

    std::cout << "Track dump" << std::endl;
    std::vector<uint8_t> track3 = fdc3.read_track();
    dump_buf(track3.data(), track3.size());
    std::cout << std::endl;

    std::cout << "ID dump" << std::endl;
    std::vector<fdc_bitstream::id_field> id3 = fdc3.read_all_idam();
    display_id_list(id3);
    std::cout << std::endl;

    std::cout << "COROCORO Sector read test" << std::endl;
    fdc_bitstream::sector_data sect_data3;
    for (int i = 0; i < 10; i++) {
        sect_data3 = fdc3.read_sector(0, 0, 0xf7);
        dump_buf(sect_data3.data.data(), sect_data3.data.size(), true);
    }
    std::cout << std::endl;
#endif

#if 1
    std::cout << "\nWrite track, write sector, read sector test" << std::endl;
    fdc_bitstream fdc4;
    bit_array track_b4;
    track_b4.resize(4e6 * 0.2);               // extend track buffer (spindle 1 spin == 0.2sec)
    fdc4.set_raw_track_data(track_b4);        // Set unformatted track data
    std::vector<uint8_t> write_data4 = generate_format_data(0,0,1,1);       // trk=0, sid=0, #sec=1, len=1
    fdc4.write_track(write_data4);

    std::cout << "Track dump - after formatting" << std::endl;
    fdc4.set_pos(0);
    std::vector<uint8_t> track4 = fdc4.read_track();
    dump_buf(track4.data(), track4.size());
    std::cout << std::endl;

    fdc4.clear_wraparound();
    fdc4.set_pos(0);

    // write sector
    std::vector<uint8_t> sect_data_w4;
    for (int i = 0; i < 256; i++) sect_data_w4.push_back(i);
    fdc4.clear_wraparound();
    fdc4.set_pos(0);
    fdc4.write_sector(0, 0, 1, false, sect_data_w4);

    std::cout << "Track dump - after sector write" << std::endl;
    fdc4.set_pos(0);
    track4 = fdc4.read_track();
    dump_buf(track4.data(), track4.size());
    std::cout << std::endl;

    // verify written data by reading the sector
    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sect_data_r4;
    sect_data_r4 = fdc4.read_sector(0, 0, 1);
    dump_buf(sect_data_r4.data.data(), sect_data_r4.data.size());
    std::cout << std::endl;
#endif


#if 1
    std::cout << "\nWrite track - full format test" << std::endl;
    fdc_bitstream fdc5;
    bit_array track_write_data5;
    track_write_data5.clear_array();
    track_write_data5.set(4e6 * 0.2, 0);     // extend track buffer
    fdc5.set_raw_track_data(track_write_data5);
    std::vector<uint8_t> write_data5 = generate_format_data(0, 0, 16, 1, 3);
    fdc5.write_track(write_data5);
    
    fdc5.set_pos(0);
    std::vector<uint8_t> track5 = fdc5.read_track();
    dump_buf(track5.data(), track5.size());
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    ids = fdc5.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sdata;
    sdata = fdc5.read_sector(0, 0, 1);
    dump_buf(sdata.data.data(), sdata.data.size());
    std::cout << std::endl;

    std::cout << "Write to all sectors (0-16)" << std::endl;
    std::vector<uint8_t> sect_data_w5;
    for (int i = 1; i <= 16; i++) {
        sect_data_w5.clear();
        sect_data_w5.resize(256, ((i - 1) << 4 | (i - 1)));
        bool sts = fdc5.write_sector(0, 0, i, false, sect_data_w5, true);  // enable fluctuator
        if (sts == false) {
            std::cout << i << "record-not-found error" << std::endl;
        }
    }

    std::cout << "Track dump - after all sector write" << std::endl;
    fdc5.set_pos(0);
    track5 = fdc5.read_track();
    dump_buf(track5.data(), track5.size());
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    for (int i = 1; i <= 16; i++) {
        std::cout << "Sector " << i << std::endl;
        fdc_bitstream::sector_data sdata;
        sdata = fdc5.read_sector(0, 0, i);
        dump_buf(sdata.data.data(), sdata.data.size());
    }
    std::cout << std::endl;

#endif

    return 0;
}
