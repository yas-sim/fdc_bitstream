#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "image_hfe.h"
#include "bit_array.h"

#include "image_mfm.h"

size_t align(size_t pos, size_t grain_size = 0x400)
{
    return ((pos / grain_size) + ((pos % grain_size) ? 1 : 0)) * grain_size;
}


int main(int argc, char* argv[]) {

    const std::string input_file_name = argv[1];
    const std::string base_file_name = input_file_name.substr(0, input_file_name.find_last_of("."));
    const std::string output_file_name = base_file_name + ".mfm";

    disk_image_hfe image;
    image.read(argv[1]);
    std::vector<std::vector<uint8_t>> &hfe_disk = image.get_track_data();

    // Create header
    mfm_header header;
    mfm_track_table track_table[84];
    strncpy(reinterpret_cast<char*>(header.id_str), "MFM_IMG ", 8);
    header.track_table_offset = 0x0100;
    header.number_of_tracks = hfe_disk.size();
    header.spindle_time_ns = 2e9;       // 2sec
    header.data_bit_rate = 500e3;       // 500KHz
    header.sampling_rate = 4e6;         // 4MHz

    size_t bit_width = header.sampling_rate / header.data_bit_rate;

    // convert HFE bitstream track data into MFM bitstream track data
    std::vector<std::vector<uint8_t>> mfm_disk;
    for (size_t track_n = 0; track_n < hfe_disk.size(); track_n++) {
        std::vector<uint8_t> hfe_track = hfe_disk[track_n];
        bit_array hfe_stream;
        bit_array track_stream;

        hfe_stream.set_array(hfe_track);
        for (size_t bit = 0; bit < hfe_stream.get_length(); bit++) {
            int bit_data = hfe_stream.read_stream();
            // write a bit cell (data pulse will be placed at the center of the bit cell)
            for (size_t j = 0; j < bit_width; j++) {
                if (j == bit_width / 2) {
                    track_stream.write_stream(bit_data, true);
                }
                else {
                    track_stream.write_stream(0, true);
                }
            }
        }
        mfm_disk.push_back(track_stream.get_array());
    }

    std::ofstream ofs;
    ofs.open(output_file_name, std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<char*>(&header), sizeof(mfm_header));
    ofs.seekp(header.track_table_offset);
    size_t write_ptr = 0x1000;
    size_t track_pos;
    for (size_t track_n = 0; track_n < mfm_disk.size(); track_n++) {
        track_pos = align(write_ptr, 0x100);
        track_table[track_n].offset     = track_pos;
        track_table[track_n].length_bit = mfm_disk[track_n].size() * 8;
        ofs.seekp(track_pos);
        ofs.write(reinterpret_cast<char*>(mfm_disk[track_n].data()), mfm_disk[track_n].size());
        write_ptr = track_pos + mfm_disk[track_n].size();
    }
    ofs.seekp(header.track_table_offset);
    ofs.write(reinterpret_cast<char*>(track_table), sizeof(mfm_track_table) * 84);
    ofs.close();

    std::cout << input_file_name << " -> " << output_file_name << std::endl;
}

