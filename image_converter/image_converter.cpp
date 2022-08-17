#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "image_hfe.h"
#include "image_mfm.h"
#include "image_raw.h"
#include "image_d77.h"
#include "bit_array.h"


void usage(std::string cmd_name) {
    std::cout << cmd_name << " input_file.[hfe|d77|mfm|raw] output_file.[d77|mfm]" << std::endl;
}

std::string get_file_base(std::string file_name) {
    int period_pos = file_name.find_last_of(".");
    const std::string extension = file_name.substr(0, period_pos);
    return extension;
}

std::string get_file_extension(std::string file_name) {
    int period_pos = file_name.find_last_of(".");
    const std::string extension = file_name.substr(period_pos + 1, file_name.length());
    return extension;
}

bool check_extension(std::string extension) {
    std::vector<std::string> allowed = { "hfe", "mfm", "raw", "d77" };
    bool res = false;
    for(auto it = allowed.begin(); it != allowed.end(); ++it) {
        if(*it == extension) res = true;
    }
    return res;
}

disk_image* create_object_by_ext(std::string ext) {
    disk_image *obj;
    if(ext == "hfe") obj = new disk_image_hfe();
    if(ext == "raw") obj = new disk_image_raw();
    if(ext == "mfm") obj = new disk_image_mfm();
    if(ext == "d77") obj = new disk_image_d77();
    return obj;
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }
    const std::string input_file_name  = argv[1];
    const std::string output_file_name = argv[2];

    const std::string input_ext  = get_file_extension(input_file_name);
    if (check_extension(input_ext)==false) {
        usage(argv[0]);
        std::cout << "A file with wrong extension is given (" << input_ext << ")." << std::endl;
        return -1;
    }
    const std::string output_ext = get_file_extension(output_file_name); 
    if (check_extension(output_ext)==false) {
        usage(argv[0]);
        std::cout << "A file with wrong extension is given (" << output_ext << ")." << std::endl;
        return -1;
    }

    disk_image *in_image = create_object_by_ext(input_ext);
    disk_image *out_image = create_object_by_ext(output_ext);

    in_image->read(input_file_name);
    if(!in_image->is_ready()) {
        std::cout << "Failed to read the input file. (Possibly wrong file format)" << std::endl;
        return -1;
    }
    disk_image_base_properties prop = in_image->get_property();
    out_image->set_property(prop);

    std::vector<bit_array> all_trk = in_image->get_track_data_all();
    out_image->set_track_data_all(all_trk);

    out_image->write(output_file_name);

    std::cout << input_file_name << " -> " << output_file_name << std::endl;

    delete out_image;
    delete in_image;
}
