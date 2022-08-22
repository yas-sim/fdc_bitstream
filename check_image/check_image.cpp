#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <iomanip>

#include "image_raw.h"
#include "image_hfe.h"
#include "image_mfm.h"
#include "image_d77.h"
#include "fdc_bitstream.h"

#include "fdc_misc.h"

void dump_buf(uint8_t* ptr, size_t size, bool line_feed = true) {
	std::ios::fmtflags flags_saved = std::cout.flags();
	for (size_t i = 0; i < size; i++) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[i]) << " ";
		if (i % 64 == 63) {
			std::cout << std::endl;
		}
	}
	if (line_feed) std::cout << std::endl;
	std::cout.flags(flags_saved);
}


void display_id_list(std::vector<fdc_bitstream::id_field> id_fields) {
	std::ios::fmtflags flags_saved = std::cout.flags();
	std::cout << std::hex << std::setw(2) << std::setfill('0');
	for (int i = 0; i < id_fields.size(); i++) {
		std::cout << std::dec << std::setw(2) << std::setfill(' ') << i << " ";
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].C) << " ";
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].H) << " ";
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].R) << " ";
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].N) << " ";
		std::cout << std::setw(4) << std::setfill('0') << static_cast<int>(id_fields[i].crc_val) << " ";
		std::cout << (id_fields[i].crc_sts ? "ERR" : "OK ") << std::endl;
	}
	std::cout.flags(flags_saved);
}

void usage(void) {
	std::cout << "Options:" << std::endl;
	std::cout << "-i input_file        Input file to check" << std::endl;
	std::cout << "-td                  Perform track dump" << std::endl;
	std::cout << "-gain gain_l gain_h  VFO gain (real numbers)" << std::endl;
	std::cout << "-ts track_number     Start track #" << std::endl;
	std::cout << "-te track_number     End track #" << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		std::cerr << "Usage: " << argv[0] << " image_file_name (.hfe|.mfm|.raw)" << std::endl;
		usage();
		return -1;
	}

	std::vector<std::string> cmd_opts;
	for(size_t i=1; i<argc; i++) {
		cmd_opts.push_back(argv[i]);
	}

	disk_image *image;

	double gain_h = 2.f, gain_l = 1.f;
	size_t track_start = 0, track_end = 83;
	std::string file_name;
	bool track_dump_flag = false;
	for(auto it = cmd_opts.begin(); it != cmd_opts.end(); ++it) {
		if(*it == "-i") {
			if(it+1 != cmd_opts.end()) {
				file_name = *++it;
			}
		} else if(*it == "-td") {
			track_dump_flag = true;
		}
		else if (*it == "-gain") {
			if (it + 1 != cmd_opts.end() && it + 2 != cmd_opts.end()) {
				gain_l = std::stod(*(it + 1));
				gain_h = std::stod(*(it + 2));
				it += 2;
			}
		}
		else if (*it == "-ts") {
			if (it + 1 != cmd_opts.end()) {
				track_start = std::stoi(*++it);
			}
		}
		else if (*it == "-te") {
			if (it + 1 != cmd_opts.end()) {
				track_end = std::stoi(*++it);
			}
		}
	}

	if (file_name.size() > 0) {
		size_t ext_pos = file_name.rfind('.');
		std::string ext = file_name.substr(ext_pos+1, file_name.size()-ext_pos-1);
		if( ext == "hfe") image = new disk_image_hfe();
		else if (ext == "mfm") image = new disk_image_mfm();
		else if (ext == "raw") image = new disk_image_raw();
		else if (ext == "d77") image = new disk_image_d77();
		else {
			std::cerr << "Unsupported file extension" << std::endl;
			return -1;
		}
		try {
			image->read(file_name);
		}
		catch (disk_image_exception e) {
			std::cout << e.what() << std::endl;
			return -1;
		}
	} else {
		std::cerr << "Input file name must be provided (with option flag '-i')" << std::endl;
		usage();
		return -1;
	}

	fdc_bitstream fdc;
	bit_array track;

	std::cout << "Gain L = " << gain_l << ", Gain H = " << gain_h << std::endl;
	fdc.set_vfo_gain_val(gain_l, gain_h);

	std::cout << "Start track = " << track_start << ", End track = " << track_end << std::endl;
	for (int track_n = track_start; track_n <= track_end; track_n++) {
		std::cout << std::endl << "=== " << track_n << std::endl;
		track = image->get_track_data(track_n);
		fdc.set_track_data(track);

		if(track_dump_flag) {
			fdc.set_pos(0);
			std::vector<uint8_t> track_data = fdc.read_track();
			dump_buf(track_data.data(), track_data.size());
		}

		fdc.set_pos(0);
		std::vector<fdc_bitstream::id_field> id_data = fdc.read_all_idam();
		display_id_list(id_data);
		double err = 0.f;
		double cnt = 0.f;
		for (auto it = id_data.begin(); it != id_data.end(); ++it) {
			if ((*it).crc_sts) {
				err++;
			}
			cnt++;
		}
		double rate;
		if (cnt == 0) {
			rate = 1.f;
		}
		else {
			rate = err / cnt;
		}
		std::cout << "Error rate: " << rate;
	}
	delete image;

	return 0;
}
