#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "image_hfe.h"
#include "bit_array.h"

#include "image_mfm.h"

inline size_t align(size_t pos, size_t grain_size = 0x400)
{
    return ((pos / grain_size) + ((pos % grain_size) ? 1 : 0)) * grain_size;
}

void usage(std::string cmd_name) {
    std::cout << cmd_name << " input_file.hfm" << std::endl;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        usage(argv[0]);
        return -1;
    }
    const std::string input_file_name = argv[1];
    if (input_file_name.length() == 0) {
        usage(argv[0]);
        return -1;
    }
    int period_pos = input_file_name.find_last_of(".");
    const std::string extension = input_file_name.substr(period_pos + 1, input_file_name.length());
    if (extension != "hfe") {
        usage(argv[0]);
        std::cout << "A file with wrong extension is given (" << extension << ")." << std::endl;
        return -1;
    }
    const std::string base_file_name = input_file_name.substr(0, input_file_name.find_last_of("."));
    const std::string output_file_name = base_file_name + ".mfm";

    disk_image_hfe image;
    image.read(input_file_name);

    // Create header
    mfm_header header;
    mfm_track_table track_table[84];
    strncpy(reinterpret_cast<char*>(header.id_str), "MFM_IMG ", 8);
    header.track_table_offset = 0x0100;
    header.number_of_tracks = 84;
    header.spindle_time_ns = 2e9;       // 2sec
    header.data_bit_rate = 500e3;       // 500KHz
    header.sampling_rate = 4e6;         // 4MHz

    size_t bit_width = header.sampling_rate / header.data_bit_rate;

    std::ofstream ofs;
    ofs.open(output_file_name, std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<char*>(&header), sizeof(mfm_header));
    ofs.seekp(header.track_table_offset);

    size_t write_ptr = align(header.track_table_offset + header.number_of_tracks * sizeof(mfm_track_table), 0x400);
    size_t track_pos;
    for (size_t track_n = 0; track_n < header.number_of_tracks; track_n++) {
        bit_array track_stream = image.get_track_data(track_n);
        std::cout << "TRACK : " << track_n << ", " << write_ptr << ", " << track_stream.get_length() << std::endl;
        track_pos = align(write_ptr, 0x100);
        track_table[track_n].offset     = track_pos;
        track_table[track_n].length_bit = track_stream.get_length();
        ofs.seekp(track_pos);

        std::vector<uint8_t> track_data = track_stream.get_array();
        ofs.write(reinterpret_cast<char*>(track_data.data()), track_data.size());
        write_ptr = track_pos + track_data.size();
    }
    ofs.seekp(header.track_table_offset);
    ofs.write(reinterpret_cast<char*>(track_table), sizeof(mfm_track_table) * 84);
    ofs.close();

    std::cout << input_file_name << " -> " << output_file_name << std::endl;
}

