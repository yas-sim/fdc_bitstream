#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "image_hfe.h"
#include "image_mfm.h"
#include "bit_array.h"


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

    disk_image_hfe in_image;
    in_image.read(input_file_name);
    if(!in_image.is_ready()) {
        std::cout << "Failed to read the input file. (Possibly wrong file format)" << std::endl;
        return -1;
    }

    disk_image_mfm out_image;
    disk_image_base_properties prop = in_image.get_property();
    out_image.set_property(prop);
    for (size_t track_n = 0; track_n < prop.m_max_track_number; track_n++) {
        bit_array track = in_image.get_track_data(track_n);
        std::cout << "TRACK : " << track_n << ", " << track.get_length() << std::endl;
        out_image.set_track_data(track_n, track);
    }

    out_image.write(output_file_name);
    std::cout << input_file_name << " -> " << output_file_name << std::endl;
}

