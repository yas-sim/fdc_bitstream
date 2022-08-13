#include <cstdint>
#include <vector>

#include "image_mfm.h"
#include "fdc_bitstream.h"

#include "fdc_misc.h"

int main(void) {
	disk_image_mfm disk_image;

	fdc_bitstream fdc;
	bit_array track;
	
	for (int track_n = 0; track_n < 84; track_n++) {
		std::cout << std::endl <<  "=== " << track_n << std::endl;
		std::vector<uint8_t> format_data = generate_format_data(track_n / 2, track_n % 2, 16, 1, 1, 0);
		track.clear_array();
		track.resize(0.2 * 4e6);		// Create an empty track data (4e6 (sampling rate) * 0.2 (sipndle time))
		fdc.set_track_data(track);
		fdc.set_pos(0);
		fdc.write_track(format_data);
		track = fdc.get_track_data();

		disk_image.set_track_data(track_n, track);
	}

	disk_image.write("new_image.mfm");

	return 0;
}
