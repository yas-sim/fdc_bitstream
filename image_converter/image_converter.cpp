#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <stdlib.h>

#include "disk_images.h"

#include "bit_array.h"
#include "fdc_vfo_def.h"
#include "fdc_misc.h"

void usage(std::string cmd_name) {
    std::cout << cmd_name << "-i input_file -o output_file [-n]" << std::endl;
    std::cout << "Input file     : mfm, raw, d77, hfe, fdx" << std::endl;
    std::cout << "Output file    : mfm, raw, d77, hfe, fdx" << std::endl;
    std::cout << "-n             : Normalize pulse pitch. Get statistic data of pulse-to-pulse distance \n"
                 "                 distribution in a track and align bit position with standard bit cell pitch.\n"
                 "                 This will make the disk image data easy to read." << std::endl;
    std::cout << "-vfo vfo_type  : Select type of VFO (" VFO_TYPE_DESC_STR "). Effective only for D77." << std::endl;
    std::cout << "-gain low high : VFO gain setting. Effective only for D77.  e.g. -gain 1.0 2.0" << std::endl;
    std::cout << "-v             : Verbose mode." << std::endl;
	std::cout << "-raw           : Export RAW mode.  Effective only for FDX." << std::endl;
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
    std::vector<std::string> allowed = { "hfe", "mfm", "raw", "d77", "fdx" };
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
    if(ext == "fdx") obj = new disk_image_fdx();
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

/**
 * @brief Inspect all tracks. The inspect result has num_ids, num_good, num_bad. num_ids == number of IDs found in the tack, num_good=number of sectors that could read without error. num_bad=number of sectors with error.
 * 
 * @param track_data 
 * @param inspect_result 
 * @param vfo_type 
 * @param gain_l 
 * @param gain_h 
 * @param sampling_rate 
 * @param data_bit_rate 
 */
void inspect_track(std::vector<bit_array> &track_data, std::vector<std::unordered_map<std::string, std::size_t>> &inspect_result, 
                const size_t vfo_type, const double gain_l, const double gain_h, 
                const size_t sampling_rate, const size_t data_bit_rate, bool verbose) {
    fdc_bitstream fdc;

    size_t cell_size_ref = sampling_rate / data_bit_rate;
    size_t sect_pos_ofst = (cell_size_ref * 16) * 16;       // sector read start position needs to be ahead a bit from the sector ID start position.

    inspect_result.resize(track_data.size());
    for(size_t track_n = 0; track_n < track_data.size(); track_n++) {
        fdc.set_track_data(track_data[track_n]);
        fdc.set_vfo_type(vfo_type);
        fdc.set_vfo_gain_val(gain_l, gain_h);
        fdc.set_vfo_gain_mode(fdc_bitstream::gain_state::low);
        std::vector<fdc_bitstream::id_field> id_list = fdc.read_all_idam();     // Read all IDAMs
        inspect_result[track_n]["num_ids"] = id_list.size();
        size_t sector_good = 0;
        size_t sector_bad = 0;
        for(auto id = id_list.begin(); id != id_list.end(); id++) {             // Read all sectors
            // read and inspect a sector
            size_t sect_pos = (*id).pos;
            sect_pos = (sect_pos < sect_pos_ofst) ? 0 : sect_pos - sect_pos_ofst;
            fdc.set_pos(sect_pos);
            fdc_bitstream::sector_data read_sect = fdc.read_sector((*id).C, (*id).R);
            if (!read_sect.record_not_found && !read_sect.crc_sts && !(*id).crc_sts) {
                sector_good++;
            } else {
                sector_bad++;
            }
        }
        inspect_result[track_n]["num_good"] = sector_good;
        inspect_result[track_n]["num_bad"]  = sector_bad;
        if(verbose) std::cout << "." << std::flush;
    }
}

/**
 * @brief 
 * Returns AM positions in MFM byte position
 * @param mfm_data 
 * @param mc_data 
 * @param mfm_pos 
 * @return std::vector<size_t> 
 */
std::vector<size_t> find_address_marks(std::vector<size_t> &mfm_data, std::vector<size_t> &mc_data, std::vector<size_t> &mfm_pos) {
    std::vector<size_t> res;
    size_t pos = 0;
    size_t prev_mc = 0;
    while(pos < mfm_data.size()) {
        if(prev_mc==1 && mc_data[pos]==0) {
            if((mfm_data[pos] & 0xfc) >= 0xfc) {      // found an address mark field
                res.push_back(pos);
            }
        }
        prev_mc = mc_data[pos];
        pos++;
    }
    return res;
}

/**
 * @brief Get the pulse dist data object
 * Creates and returns a list of pulse distance data
 * @param track_data 
 * @param start_pos 
 * @param num 
 * @return std::vector<size_t> 
 */
std::vector<size_t> get_pulse_dist_buf(bit_array &track_data, size_t start_pos, size_t num = (16*16)) {
    std::vector<size_t> res;
    track_data.set_stream_pos(start_pos);
    for(size_t i=0; i<num; i++) {
        res.push_back(track_data.distance_to_next_pulse());
    }
    return res;
}

/**
 * @brief Calculate the correlation metric for two vectors.
 * @param dist_buf0 
 * @param dist_buf1 
 * @return size_t 
 */
size_t calc_dist_buf_correlation(std::vector<size_t> dist_buf0, std::vector<size_t> dist_buf1) {
    size_t len = std::min(dist_buf0.size(), dist_buf1.size());
    size_t metric = 0;
    for(size_t i = 0; i < len; i++) {
        int diff = static_cast<int>(dist_buf0[i]) - static_cast<int>(dist_buf1[i]);
        metric += diff * diff;             // SSD
        //metric += std::abs(diff);                    // SAD
    }
    return metric;
}

size_t compare_data(std::vector<size_t> data, size_t pos1, size_t pos2, size_t size = 0x10) {
    size_t error = 0;
    for(size_t ofst = 0; ofst < size; ofst++) {
        error += std::abs(static_cast<int>(data[pos1 + ofst]) - static_cast<int>(data[pos2 + ofst]));
    }
    return error;
}

void dump(std::vector<size_t> data, size_t pos, size_t size = 0x10) {
    std::cout << std::hex << std::setw(10) << std::setfill('0') << pos << " : ";
    for(size_t i = 0; i < size; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << data[pos + i] << " ";
    }
    std::cout << std::endl;
}

/**
 * @brief 
 * Trim track data with read-overlap to connect the top of the track data and the end of the track data nicely and smoothly.
 * 
 * @param track_data 
 * @return bit_array 
 */
void stitch_track_auto(disk_image *input_image, bool verbose) {
    disk_image_base_properties prop = input_image->get_property();
    size_t pos_top_of_2nd_spin = (prop.m_spindle_time_ns) / (1e9 / prop.m_sampling_rate);

    fdc_bitstream fdc;
    std::vector<std::vector<size_t>> read_data; // (mfm_data, mc_flag, pos, error)

    for(size_t track_num = 0; track_num < prop.m_number_of_tracks; track_num++) {
        bit_array tmp_track_data = input_image->get_track_data(track_num);

#if 0        
        // Remove the 1st pulse data from the track data because the distance to the 1st pulse from the top of the track is unreliable (the pulse might be captured incompletely)
        bit_array track_data;
        tmp_track_data.set_stream_pos(0);
        tmp_track_data.distance_to_next_pulse();                          // read bit data until the 1st pulse is read (1st '1' data)
        while(tmp_track_data.is_wraparound() == false) {
            track_data.write_stream(tmp_track_data.read_stream(), /*elastic=*/true);    // Copy the rest of track data
        }
#else
        bit_array track_data { tmp_track_data };
#endif

        fdc.set_track_data(track_data);
        fdc.set_pos(0);
        read_data = fdc.read_track_ex();                                  // perform read track
        std::vector<size_t> mfm_data, mc_data, mfm_pos;
        for(auto it = read_data.begin(); it != read_data.end(); it++) {   // separate mfm_data, mc_data, and mfm_pos
            mfm_data.push_back((*it)[0]);
            mc_data.push_back((*it)[1]);
            mfm_pos.push_back((*it)[2]);
        }
        std::vector<size_t> am_pos = find_address_marks(mfm_data, mc_data, mfm_pos);

        size_t trim_pos = pos_top_of_2nd_spin;              // default trim position (in bit pos). will applied if no pair of match AM field is detected.
        if(am_pos.size() != 0) {                            // AM fields detected
            size_t i = 0;
            for(i=0; i < am_pos.size(); i++) {
                if(mfm_pos[am_pos[i]] >= pos_top_of_2nd_spin) break;                         // find the 1st AM in the 2nd spin
            }
            if(i != am_pos.size()) {                                                         // AM fields are found in the 2nd spin
                std::vector<size_t> am_pos_1s { am_pos.begin()    , am_pos.begin() + i - 1};    // Vector of AMs in the 1st spin (mfm byte pos)
                std::vector<size_t> am_pos_2s { am_pos.begin() + i, am_pos.end()};              // Vector of AMs in the 2nd spin (mfm byte pos)
                size_t i1 = 0, i2 = 0;
                bool match_flag = true;
                while(compare_data(mfm_data, am_pos_1s[i1], am_pos_2s[i2], 0x10) != 0) {      // find a match AM pair from the 1st spin and the 2nd spin (compare 0x10 bytes)
                    if(++i1 == am_pos_1s.size()) {
                        i1 = 0;
                        if(++i2 == am_pos_2s.size()) {
                            match_flag = false;
                            break;
                        }
                    }
                }
                //     1st spin                                       2nd spin
                //     |   *AM0                                       |   *AM0
                //     ^---^ == top_pos_1s                            ^---^ <- Assuming this is equal to top_pos_1s.
                if(match_flag) {       // Found match AM pair.
                    size_t &am_1s = am_pos_1s[i1];                        // MFM byte position of the matched AM in the 1st spin.
                    size_t &am_2s = am_pos_2s[i2];                        // MFM byte position of the matched AM in the 2nd spin.
                    size_t top_pos_2s = mfm_pos[am_2s - am_1s];           // assumed position (in bit_array pos) of the top of the 2nd spin.
                    if(verbose) {
                        std::cout << "TRACK " << std::dec << track_num << " - AM pair" << std::endl;
                        dump(mfm_data, am_1s);
                        dump(mfm_data, am_2s);
                    }
                    //dump(mfm_data, 0);
                    //dump(mfm_data, am_2s - am_1s);

                    // Search precise trim point
                    const size_t compare_length = 64;
                    std::vector<size_t> dist_buf0 = get_pulse_dist_buf(track_data, 0, compare_length);           // distance buffer for 128 pulses from the top of the track
                    std::vector<size_t> dist_buf1, minbuf0, minbuf1;
                    int bit_cell_size = prop.m_sampling_rate / prop.m_data_bit_rate;
                    size_t min_error = UINT64_MAX;
                    int min_offset = 0;
                    for(int offset = -(bit_cell_size * 32); offset < (bit_cell_size * 32); offset++) {        // search +- 4 bit cell region
                        dist_buf1 = get_pulse_dist_buf(track_data, top_pos_2s + offset, compare_length);
                        size_t error = calc_dist_buf_correlation(dist_buf0, dist_buf1);                       // find least error position
                        //error += offset * offset;                                                           // count the offset into calculation
                        error += std::abs(offset);                                                            // count the offset into calculation
                        if(error < min_error) {
                            min_error = error;
                            min_offset = offset;
                            minbuf0 = dist_buf0;
                            minbuf1 = dist_buf1;
                        }
                    }
                    if(verbose) {
                        std::cout << "Pulse distance buff - matching result" << std::endl;
                        dump(minbuf0, 0, compare_length);
                        dump(minbuf1, 0, compare_length);
                    }
                    if(verbose) {
                        std::cout << "Min error:" << std::dec << min_error << " Avg error:" << min_error/static_cast<double>(compare_length) << ", offset:" << min_offset << std::endl;
                        std::cout << std::endl;
                    }
                    trim_pos = top_pos_2s + min_offset;      // adjust the trim position
                    trim_pos += -1;
                }
            }
        }
        track_data.resize(trim_pos);                // perform track data trimming
        input_image->set_track_data(track_num, track_data);    // replace track data with trimmed one
    }
}

int main(int argc, char* argv[]) {

	std::vector<std::string> cmd_opts;
	for(size_t i=1; i<argc; i++) {
		cmd_opts.push_back(argv[i]);
	}

    std::vector<std::string> input_file_names;
    std::string output_file_name;
    bool normalize = false;
    size_t vfo_type = VFO_TYPE_DEFAULT;
    double gain_l = VFO_GAIN_L_DEFAULT;
    double gain_h = VFO_GAIN_H_DEFAULT;
    bool verbose = false;
    bool raw_flag = false;          // effective only for FDX format
    bool auto_trim = false;         // auto trim track

	for(auto it = cmd_opts.begin(); it != cmd_opts.end(); ++it) {
		if(*it == "-i" && it+1 != cmd_opts.end()) {
            input_file_names.push_back(*(++it));                // accepts multiple input files
        } else if(*it == "-o" && it+1 != cmd_opts.end()) {
            output_file_name = *(++it);
        } else if(*it == "-n") {
            normalize = true;
        } else if(*it == "-vfo" && it+1 != cmd_opts.end()) {
            vfo_type = std::stoi(*(++it));
        } else if(*it == "-gain" && it+1 != cmd_opts.end() && it+2 != cmd_opts.end()) {
            gain_l = std::stod(*(++it));
            gain_h = std::stod(*(++it));
        } else if(*it == "-auto_trim") {
            auto_trim = true;
        } else if(*it == "-v") {
            verbose = true;
        } else if(*it == "-raw") {
            raw_flag = true;
        }
    }


    if (input_file_names.size()==0 || output_file_name.size()==0) {
        usage(argv[0]);
        return -1;
    }

    // Create image objects for the input files
    std::vector<disk_image*> in_images;
    for(auto i = 0; i < input_file_names.size(); i++) {
        const std::string input_ext  = get_file_extension(input_file_names[i]);
        if (check_extension(input_ext)==false) {
            usage(argv[0]);
            std::cout << "A file with wrong extension is given (" << input_ext << ")." << std::endl;
            return -1;
        }
        in_images.push_back(create_object_by_ext(input_ext));
    }

    // Create an image object for the output file
    const std::string output_ext = get_file_extension(output_file_name); 
    if (check_extension(output_ext)==false) {
        usage(argv[0]);
        std::cout << "A file with wrong extension is given (" << output_ext << ")." << std::endl;
        return -1;
    }
    disk_image *out_image = create_object_by_ext(output_ext);

    // Read (multiple) input files
    for(int i=0; i < input_file_names.size(); i++) {
        if(verbose) std::cout << "Reading " << input_file_names[i] << "." << std::endl;
        in_images[i]->read(input_file_names[i]);
        if(!in_images[i]->is_ready()) {
            std::cout << "Failed to read the input file. (Possibly wrong file format)" << std::endl;
            return -1;
        }
        if(auto_trim) {
            if(verbose) std::cout << "Auto trim" << std::endl;
            stitch_track_auto(in_images[i], verbose);
        }
        if(normalize) {
            disk_image_base_properties prop = in_images[i]->get_property();
            std::vector<bit_array> tracks = in_images[i]->get_track_data_all();
            tracks = normalize_track(tracks, prop.m_sampling_rate, prop.m_data_bit_rate, verbose);
            in_images[i]->set_track_data_all(tracks);
        }
    }

    // Obtain image parameters and copy them to output image. Use image0 as the representative of the input disk images.
    disk_image_base_properties prop = in_images[0]->get_property();
    out_image->set_property(prop);

    std::vector<bit_array> chimera_image;
    chimera_image.resize(in_images[0]->get_number_of_tracks());
    std::vector<std::vector<std::unordered_map<std::string, size_t>>> inspect_result; // [img][trk]["keys"]
    if (input_file_names.size() > 1) {
        // In case multiple input files are given. Inspects all tracks track-by-track and generates a new image by compiling the best track from the images.
        std::vector<std::vector<bit_array>> all_trks;
        inspect_result.resize(input_file_names.size());
        for(size_t img_n = 0; img_n < input_file_names.size(); img_n++) {
            if(verbose) std::cout << "Inspecting '" << input_file_names[img_n] << "'";
            all_trks.push_back(in_images[img_n]->get_track_data_all());
            inspect_track(all_trks[img_n], inspect_result[img_n], vfo_type, gain_l, gain_h, prop.m_sampling_rate, prop.m_data_bit_rate, verbose);
            if(verbose) std::cout << std::endl;
        }

        if(verbose) std::cout << "Merging images...";
        for(size_t trk_n = 0; trk_n < in_images[0]->get_number_of_tracks(); trk_n++) {
            size_t best_img_n = 0;
            size_t max_sect = 0;
            std::vector<size_t> max_sct_img_n;
            // search for the best image (track by track)
# if 0
            for(size_t img_n=0; img_n < input_file_names.size(); img_n++) {
                if(inspect_result[img_n][trk_n]["num_ids"] > max_sect) {
                    max_sect = inspect_result[img_n][trk_n]["num_ids"];     // find the maximum number of IDs in the track from images
                }
            }
            for(size_t img_n=0; img_n < input_file_names.size(); img_n++) {
                if(inspect_result[img_n][trk_n]["num_ids"] == max_sect) {
                    max_sct_img_n.push_back(img_n);                         // record which image has the max IDs in the track
                }
            }
            max_sect = 0;
            for(auto it = max_sct_img_n.begin(); it != max_sct_img_n.end(); it++) {
                size_t num_good = inspect_result[(*it)][trk_n]["num_good"];
                size_t num_bad  = inspect_result[(*it)][trk_n]["num_bad"];
                if(num_good > max_sect) {                                   // pick the best track (that has the most good sectors) 
                    max_sect = num_good;
                    best_img_n = (*it);
                }
                if(verbose) std::cout << best_img_n << std::flush;
                chimera_image[trk_n] = all_trks[best_img_n][trk_n];
            }
#else
            max_sect = 0;
            // select the track by the number of good sectors
            for(size_t img_n = 0; img_n < input_file_names.size(); img_n++) {
                size_t num_good = inspect_result[img_n][trk_n]["num_good"];
                size_t num_bad  = inspect_result[img_n][trk_n]["num_bad"];
                if(num_good > max_sect) {                                   // pick the best track (that has the most good sectors) 
                    max_sect = num_good;
                    best_img_n = img_n;
                }
                if(verbose) std::cout << best_img_n << std::flush;
                chimera_image[trk_n] = all_trks[best_img_n][trk_n];
            }
#endif
        }
        if(verbose) std::cout << std::endl;

    } else {
        chimera_image = in_images[0]->get_track_data_all();       // In case # of input file == 1
    }

    out_image->set_track_data_all(chimera_image);
    if(output_ext == "d77") {
        // only for D77 output
        out_image->set_vfo_type(vfo_type);   
        if(verbose) {
            std::cout << "VFO type : " << vfo_type << std::endl;   
            std::cout << "Gain L=" << gain_l << " , Gain H=" << gain_h << std::endl;
        }
        out_image->set_gain(gain_l, gain_h);
    }

    if(output_ext == "fdx") {
        static_cast<disk_image_fdx*>(out_image)->set_conversion_mode(raw_flag);
    }

    out_image->verbose(verbose);
    out_image->write(output_file_name);

    // Display file names
    for(auto it = input_file_names.begin(); it != input_file_names.end(); it++) {
        std::cout << (*it) << ",";
    }
    std::cout << " -> " << output_file_name << std::endl;

    // Delete objects
    delete out_image;
    for(auto it = in_images.begin(); it != in_images.end(); it++) {
        delete (*it);
    }
}
