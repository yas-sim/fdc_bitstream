#include <cstdint>
#include <vector>
#include <map>

#include "image_mfm.h"
#include "fdc_bitstream.h"

#include "fdc_misc.h"

/**
 * @brief 
 * 
 * @param track_data bit_stream track data.
 * @param sampling_rate sampling rate of the track data (default=4e6).
 * @param bit_rate bit rate of the track data (default=500e3).
 * @param amplitude amplitude of bit rate frequency fluctuator (0.1~0.01 would be reasonable range)
 * @param frequency frequency of bit rate frequency fluctuator [Hz]
 * @return bit_array modified track data
 */
bit_array add_timing_fluctuation(bit_array &track_data, size_t sampling_rate, size_t bit_rate, double amplitude, double frequency) {
    double sampling_freq = static_cast<double>(sampling_rate);
    double bit_cell = static_cast<double>(sampling_rate) / static_cast<double>(bit_rate);

    bit_array output;
    output.resize(track_data.size()*1.5);
    double out_pos = 0.f;
    for(size_t read_pos = 0; read_pos < track_data.size(); read_pos++) {
        double phase = (static_cast<double>(read_pos) / (sampling_rate / frequency)) * 2.f * 3.141592f;
        double displacement = sin(phase) * amplitude;
        //std::cout << phase << ", " << displacement << std::endl;
        out_pos += 1.f + displacement;
        if(out_pos >= output.size()) {
            output.resize(output.size() * 1.5);
        }
        output.set(out_pos, track_data.get(read_pos));
    }
    output.resize(out_pos + 1);

    return output;
}

void help(void) {
	std::cout << "-sr <sampling_rate> : Sampling rate (Hz)\n"
	             "-br <bit_rate>      : Bit rate (bit/sec)\n"
				 "-amp <amplitude>    : Amplitude of bit rate fluctuation (0.1 = +-0.1*sampling_rate, 0.01~0.1 would be reasonable)\n"
				 "-freq <frequency>   : Frequency of bit rate fluctuation (5Hz = 1 fluctuation cycle in 1 FD spin)\n";
}

std::map<std::string, double> parse_args(int argc, char* argv[]) {
	std::map<std::string, double> res;
	res["sampling_rate"] = 4e6;
	res["bit_rate"] = 500e3;
	res["amplitude"] = 0.f;
	res["frequency"] = 1.f;

	for(int i = 1; i < argc; i ++) {
		if(strcmp(argv[i], "-amp") == 0  && argc > i + 1 ) {
			res["amplitude"] = atof(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i], "-freq") == 0  && argc > i + 1 ) {
			res["frequency"] = atof(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i], "-br") == 0  && argc > i + 1 ) {
			res["bit_rate"] = atof(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i], "-sr") == 0  && argc > i + 1 ) {
			res["sampling_rate"] = atof(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i], "-h") == 0) {
			help();
			exit(0);
		}
	}
	return res;
}

int main(int argc, char* argv[]) {

	std::map<std::string, double> args = parse_args(argc, argv);
	if(args["amplitude"] != 0.f) {
	    std::cout << "Sampling rate:" << args["sampling_rate"] << std::endl 
		          << "Bit rate     :" << args["bit_rate"] << std::endl
				  << "Amplitude    :" << args["amplitude"] << std::endl
				  << "Frequency    :" << args["frequency"] << std::endl
				  << std::endl;
	}

	disk_image_mfm disk_image;

	fdc_bitstream fdc;
	bit_array track;

	fdc.set_fdc_params(args["sampling_rate"], args["bit_rate"], 0.9f);
	
	for (int track_n = 0; track_n < 84; track_n++) {
		std::cout << "=== " << track_n << std::endl;
		std::vector<uint8_t> format_data = fdc_misc::generate_format_data(track_n / 2, track_n % 2, 16, 1, 1, 0);
		track.clear_array();
		track.resize(0.2 * args["sampling_rate"]);		// Create an empty track data (4e6 (sampling rate) * 0.2 (sipndle time))
		fdc.set_track_data(track);
		fdc.set_pos(0);
		fdc.write_track(format_data);
		track = fdc.get_track_data();

        track = add_timing_fluctuation(track, args["sampling_rate"], args["bit_rate"], args["amplitude"], args["frequency"]);

		disk_image.set_track_data(track_n, track);
	}

	disk_image.write("new_image.mfm");

	return 0;
}
