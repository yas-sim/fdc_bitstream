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


int main(void)
{
#if 1
    std::cout << "*** MFM format data, and read test" << std::endl;
    fdc_bitstream fdc1;
    disk_image_mfm image1;
    image1.read("test.mfm");
    bit_array barray = image1.get_track_data(0);
    fdc1.set_raw_track_data(barray);
    fdc1.set_pos(0);
    std::vector<uint8_t> track1 = fdc1.read_track();
    dump_buf(track1.data(), 2048);
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    std::vector<fdc_bitstream::id_field> ids = fdc1.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;
#endif

#if 1
    std::cout << "\nWrite track, write sector, read sector test" << std::endl;
    fdc_bitstream fdc2;
    bit_array track_b2;
    track_b2.resize(4e6 * 0.2);               // extend track buffer (spindle 1 spin == 0.2sec)
    fdc2.set_raw_track_data(track_b2);        // Set unformatted track data
    std::vector<uint8_t> write_data4 = generate_format_data(0,0,1,1);       // trk=0, sid=0, #sec=1, len=1
    fdc2.write_track(write_data4);

    std::cout << "Track dump - after formatting" << std::endl;
    fdc2.set_pos(0);
    std::vector<uint8_t> track2 = fdc2.read_track();
    dump_buf(track2.data(), track2.size());
    std::cout << std::endl;

    fdc2.clear_wraparound();
    fdc2.set_pos(0);

    // write sector
    std::vector<uint8_t> sect_data_w2;
    for (int i = 0; i < 256; i++) sect_data_w2.push_back(i);
    fdc2.clear_wraparound();
    fdc2.set_pos(0);
    fdc2.write_sector(0, 0, 1, false, sect_data_w2);

    std::cout << "Track dump - after sector write" << std::endl;
    fdc2.set_pos(0);
    track2 = fdc2.read_track();
    dump_buf(track2.data(), track2.size());
    std::cout << std::endl;

    // verify written data by reading the sector
    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sect_data_r2;
    sect_data_r2 = fdc2.read_sector(0, 0, 1);
    dump_buf(sect_data_r2.data.data(), sect_data_r2.data.size());
    std::cout << std::endl;
#endif


#if 1
    std::cout << "\nFormat test - format (track write), write all sectors, and read all sectors" << std::endl;
    fdc_bitstream fdc3;
    bit_array track_write_data3;
    track_write_data3.clear_array();
    track_write_data3.set(4e6 * 0.2, 0);     // extend track buffer
    fdc3.set_raw_track_data(track_write_data3);
    std::vector<uint8_t> write_data3 = generate_format_data(0, 0, 16, 1, 3);
    fdc3.write_track(write_data3);
    
    fdc3.set_pos(0);
    std::vector<uint8_t> track3 = fdc3.read_track();
    dump_buf(track3.data(), track3.size());
    std::cout << std::endl;

    std::cout << "ID" << std::endl;
    ids = fdc3.read_all_idam();
    display_id_list(ids);
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    fdc_bitstream::sector_data sect_data_r3;
    sect_data_r3 = fdc3.read_sector(0, 0, 1);
    dump_buf(sect_data_r3.data.data(), sect_data_r3.data.size());
    std::cout << std::endl;

    std::cout << "Write to all sectors (0-16)" << std::endl;
    std::vector<uint8_t> sect_data_w3;
    for (int i = 1; i <= 16; i++) {
        sect_data_w3.clear();
        sect_data_w3.resize(256, ((i - 1) << 4 | (i - 1)));
        bool sts = fdc3.write_sector(0, 0, i, false, sect_data_w3, true);  // enable fluctuator
        if (sts == false) {
            std::cout << i << "record-not-found error" << std::endl;
        }
    }
#if 1
    std::cout << "Track dump - after all sector write" << std::endl;
    fdc3.set_pos(0);
    track3 = fdc3.read_track();
    dump_buf(track3.data(), track3.size());
    std::cout << std::endl;

    std::cout << "Read sector data" << std::endl;
    for (int i = 1; i <= 16; i++) {
        std::cout << "Sector " << i << std::endl;
        fdc_bitstream::sector_data sect_data_r3;
        sect_data_r3 = fdc3.read_sector(0, 0, i);
        dump_buf(sect_data_r3.data.data(), sect_data_r3.data.size());
    }
    std::cout << std::endl;
#endif

#endif

    return 0;
}
