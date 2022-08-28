#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "image_hfe.h"
#include "image_mfm.h"
#include "image_raw.h"
#include "image_d77.h"
#include "bit_array.h"
#include "fdc_vfo_def.h"
#include "fdc_misc.h"

void usage(std::string cmd_name) {
    std::cout << cmd_name << "-i input_file -o output_file [-n]" << std::endl;
    std::cout << "Input file     : hfe, mfm, raw, d77" << std::endl;
    std::cout << "Output file    : mfm, d77" << std::endl;
    std::cout << "-n             : Normalize pulse pitch. Get statistic data of pulse-to-pulse distance \n"
                 "                 distribution in a track and align bit position with standard bit cell pitch.\n"
                 "                 This will make the disk image data easy to read." << std::endl;
    std::cout << "-vfo vfo_type  : Select type of VFO (" VFO_TYPE_DESC_STR "). Effective only for D77." << std::endl;
    std::cout << "-gain low high : VFO gain setting. Effective only for D77.  e.g. -gain 1.0 2.0" << std::endl;
    std::cout << "-v             : Verbose mode." << std::endl;
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



std::vector<bit_array> normalize_track(std::vector<bit_array> tracks, size_t sampling_rate, size_t bit_rate, bool verbose=false) {
    if(verbose) {
        std::cout << "=== Pulse pitch normalize ===" << std::endl;
        std::cout << "Sampling rate : " << sampling_rate / 1e6 << " MHz" << std::endl;
        std::cout << "Data bit rate : " << bit_rate / 1e3 << " Kbits/sec" << std::endl;
    }
    double cell_size_std = static_cast<double>(sampling_rate) / static_cast<double>(bit_rate);

    std::vector<bit_array> res;
    bit_array trk;
    for(size_t i=0; i<tracks.size(); i++) {
        trk = tracks[i];
        bit_array normalized;
        normalized.clear_array();
        if(trk.get_length() > 0) {
            if (verbose) {
                std::cout << "Track " << std::setw(3) << i << " - ";
            }
            std::vector<size_t> freq_dist = fdc_misc::get_frequent_distribution(trk);   // convert track data into frequency-distribution data
            //display_histogram(freq_dist);
            std::vector<size_t> peaks = fdc_misc::find_peaks(freq_dist);                // find peaks from the frequency-distribution data
            double cell_size;
            if(peaks[0] != 0) {
                cell_size = static_cast<double>(peaks[0]) / 2.f;
            } else {
                cell_size = cell_size_std;      // to avoid 0div
            }
            double scale = cell_size_std / cell_size;
            if(verbose) {
                std::cout << "Scale : " << std::fixed << std::setprecision(5) << scale;
                std::cout << ", Peaks : (" << peaks[0] << ", " << peaks[1] << ", " << peaks[2] << ")" << std::endl;
            }
            std::vector<size_t> dist_array = fdc_misc::convert_to_dist_array(trk);
            double curr_pos = 0.f;
            for(auto it = dist_array.begin(); it != dist_array.end(); ++it) {
                curr_pos += (*it) * scale;
                normalized.set(curr_pos, 1);
            }
        }
        res.push_back(normalized);
    }
    return res;
}

int main(int argc, char* argv[]) {

	std::vector<std::string> cmd_opts;
	for(size_t i=1; i<argc; i++) {
		cmd_opts.push_back(argv[i]);
	}

    std::string input_file_name;
    std::string output_file_name;
    bool normalize = false;
    size_t vfo_type = VFO_TYPE_DEFAULT;
    double gain_l = VFO_GAIN_L_DEFAULT;
    double gain_h = VFO_GAIN_H_DEFAULT;
    bool verbose = false;

	for(auto it = cmd_opts.begin(); it != cmd_opts.end(); ++it) {
		if(*it == "-i" && it+1 != cmd_opts.end()) {
            input_file_name = *(++it);
        } else if(*it == "-o" && it+1 != cmd_opts.end()) {
            output_file_name = *(++it);
        } else if(*it == "-n") {
            normalize = true;
        } else if(*it == "-vfo" && it+1 != cmd_opts.end()) {
            vfo_type = std::stoi(*(++it));
        } else if(*it == "-gain" && it+1 != cmd_opts.end() && it+2 != cmd_opts.end()) {
            gain_l = std::stod(*(++it));
            gain_h = std::stod(*(++it));
        } else if(*it == "-v") {
            verbose = true;
        }
    }

    if (input_file_name.size()==0 || output_file_name.size()==0) {
        usage(argv[0]);
        return -1;
    }

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

    size_t sampling_rate = in_image->get_sampling_rate();
    size_t bit_rate = in_image->get_data_bit_rate();
    if(normalize) {
        all_trk = normalize_track(all_trk, sampling_rate, bit_rate, verbose);
    }

    out_image->set_track_data_all(all_trk);
    if(output_ext == "d77") {
        // only for D77 output
        out_image->set_vfo_type(vfo_type);   
        std::cout << "VFO type : " << vfo_type << std::endl;   
        if(verbose) {
            std::cout << "Gain L=" << gain_l << " , Gain H=" << gain_h << std::endl;
        }
        out_image->set_gain(gain_l, gain_h);
        out_image->verbose(verbose);
    }

    out_image->write(output_file_name);

    std::cout << input_file_name << " -> " << output_file_name << std::endl;

    delete out_image;
    delete in_image;
}
