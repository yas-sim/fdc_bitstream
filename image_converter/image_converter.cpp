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

void usage(std::string cmd_name) {
    std::cout << cmd_name << "-i input_file -o output_file [-n]" << std::endl;
    std::cout << "Input file : hfe, mfm, raw, d77" << std::endl;
    std::cout << "Output file: mfm, d77" << std::endl;
    std::cout << "-n            : Normalize pulse pitch. Get statistic data of pulse-to-pulse distance \n"
                 "                distribution in a track and align bit position with standard bit cell pitch.\n"
                 "                This will make the disk image data easy to read." << std::endl;
    std::cout << "-vfo vfo_type : Select type of VFO (" VFO_TYPE_DESC_STR ")" << std::endl;
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



void display_histogram(std::vector<size_t> dist_array) {
    size_t max_count = *std::max_element(dist_array.begin(), dist_array.end());
    double scale = 100.f / static_cast<double>(max_count);
    size_t max_item = 0;
    for(size_t i=0; i<dist_array.size(); i++) {
        if (dist_array[i] != 0) max_item = i;
    }

    // display histogram
    for(size_t i=0; i<=max_item; i++) {
        std::cout << std::setw(4) << std::setfill(' ') << i << " : ";
        std::cout << std::setw(8) << std::setfill(' ') << dist_array[i] << " : ";
        size_t bar_length = static_cast<size_t>(static_cast<double>(dist_array[i]) * scale); // normalize
        std::cout << std::string(bar_length, '*') << std::endl;
    }
}


std::vector<size_t> get_frequent_distribution(bit_array barray) {
    barray.clear_wraparound_flag();
    std::vector<size_t> freq_dist;
    do {
        size_t dist = barray.distance_to_next_pulse();
        if(freq_dist.size() <= dist) {
            freq_dist.resize(dist+1);
        }
        freq_dist[dist]++;
    } while(!barray.is_wraparound());
    return freq_dist;
}

std::vector<size_t> find_peaks(std::vector<size_t> vec) {
    // calc moving average
    vec.resize(vec.size()+2);       // increase top boundary for moving average calculation
    std::vector<size_t> avg(vec.size());
    for(size_t i = 2; i<vec.size()-2-1; i++) {
        size_t avg_tmp = 0;
        for(int j=-2; j<=2; j++) {
            avg_tmp += vec[i+j];
        }
        avg[i] = avg_tmp / 5;
    }
    //display_histogram(avg);       // for debug purpose

    // detect peaks
    std::vector<std::pair<size_t, size_t>> peaks;
    for(size_t i=1; i<avg.size()-1-1; i++) {
        if(avg[i-1]<avg[i] && avg[i+1]<avg[i]) {    // peak
            peaks.push_back(std::pair<size_t, size_t>(i, avg[i]));
        }
    }

    // sort peaks
    std::sort(peaks.begin(), peaks.end(), [](const std::pair<size_t, size_t> &left, const std::pair<size_t, size_t> &right) 
                                                { return left.second > right.second; });

    // Display sort result
    //for(auto it=peaks.begin(); it != peaks.end(); ++it) {
    //    std::cout << (*it).first << "," << (*it).second << std::endl;
    //}

    if (peaks.size()<3) {
        peaks.resize(3);
    }
    std::vector<size_t> result;
    for(auto it=peaks.begin(); it != peaks.begin()+3; ++it) {
        result.push_back((*it).first);
    }

    std::sort(result.begin(), result.end(), [](const size_t &left, const size_t &right) 
                                                { return left < right; });

    return result;
}

std::vector<size_t> convert_to_dist_array(bit_array track) {
    track.clear_wraparound_flag();
    std::vector<size_t> dist_array;
    do {
        size_t dist = track.distance_to_next_pulse();
        dist_array.push_back(dist);
    } while(!track.is_wraparound());
    return dist_array;
}


std::vector<bit_array> normalize_track(std::vector<bit_array> tracks, size_t sampling_rate, size_t bit_rate) {
    std::cout << "=== Pulse pitch normalize ===" << std::endl;
    std::cout << "Sampling rate : " << sampling_rate / 1e6 << " MHz" << std::endl;
    std::cout << "Data bit rate : " << bit_rate / 1e3 << " Kbits/sec" << std::endl;
    double cell_size_std = static_cast<double>(sampling_rate) / static_cast<double>(bit_rate);

    std::vector<bit_array> res;
    bit_array trk;
    for(size_t i=0; i<tracks.size(); i++) {
        trk = tracks[i];
        bit_array normalized;
        normalized.clear_array();
        if(trk.get_length() > 0) {
            std::cout << "Track " << std::setw(3) << i << " - ";
            std::vector<size_t> freq_dist = get_frequent_distribution(trk);   // convert track data into frequency-distribution data
            //display_histogram(freq_dist);
            std::vector<size_t> peaks = find_peaks(freq_dist);                // find peaks from the frequency-distribution data
            double cell_size;
            if(peaks[0] != 0) {
                cell_size = static_cast<double>(peaks[0]) / 2.f;
            } else {
                cell_size = cell_size_std;      // to avoid 0div
            }
            double scale = cell_size_std / cell_size;
            std::cout << "Scale : " << std::fixed << std::setprecision(5) << scale;
            std::cout << ", Peaks : (" << peaks[0] << ", " << peaks[1] << ", " << peaks[2] << ")" << std::endl;

            std::vector<size_t> dist_array = convert_to_dist_array(trk);
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

	for(auto it = cmd_opts.begin(); it != cmd_opts.end(); ++it) {
		if(*it == "-i" && it+1 != cmd_opts.end()) {
            input_file_name = *(++it);
        } else if(*it == "-o" && it+1 != cmd_opts.end()) {
            output_file_name = *(++it);
        } else if(*it == "-n") {
            normalize = true;
        } else if(*it == "-vfo" && it+1 != cmd_opts.end()) {
            vfo_type = std::stoi(*(++it));
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
        all_trk = normalize_track(all_trk, sampling_rate, bit_rate);
    }

    out_image->set_track_data_all(all_trk);
    if(output_ext == "d77") {
        out_image->set_vfo_type(vfo_type);      // only for D77 output
    }
    out_image->write(output_file_name);

    std::cout << input_file_name << " -> " << output_file_name << std::endl;

    delete out_image;
    delete in_image;
}
