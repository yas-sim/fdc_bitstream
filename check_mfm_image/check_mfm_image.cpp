#include <cstdint>
#include <vector>
#include <iostream>
#include <iomanip>

#include "image_mfm.h"
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


int main(void) {
	disk_image_mfm disk_image;


	try {
		disk_image.read("image.mfm");

		fdc_bitstream fdc;
		bit_array track;

		fdc.set_vfo_gain(8.f, 12.f);

		for (int track_n = 0; track_n < 69; track_n++) {
			std::cout << std::endl << "=== " << track_n << std::endl;
			track = disk_image.get_track_data(track_n);
			fdc.set_track_data(track);

#if 0
			fdc.set_pos(0);
			std::vector<uint8_t> track_data = fdc.read_track();
			dump_buf(track_data.data(), track_data.size());
#endif
#if 1
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
#endif
		}
	}
	catch (disk_image_exception e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
