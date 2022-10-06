#include <string>

#include "disk_images.h"
#include "fdc_bitstream.h"


const unsigned int FORMAT_PREGAP_LENGTH=32;
const unsigned int FORMAT_SYNC_LENGTH=12;
const unsigned int FORMAT_GAP2_LENGTH=0x16;
const unsigned int FORMAT_GAP3_LENGTH=0x36;
const unsigned int FORMAT_POST_GAP_LENGTH=0x300;

const unsigned char FORMAT_GAP=0x4E;
const unsigned char FORMAT_SYNC=0;
const unsigned char FORMAT_CRC=0xF7;
const unsigned char FORMAT_TEMP_DATA=0xE5;


std::vector <uint8_t> MakeFormatData(int C,int H)
{
	std::vector <uint8_t> format;

	for(unsigned int i=0; i<FORMAT_PREGAP_LENGTH; ++i)
	{
		format.push_back(FORMAT_GAP);
	}

	const uint8_t N=1; // 256 bytes

	for(uint8_t R=1; R<=16; ++R)  // 16 sectors per track.
	{
		for(int i=0; i<FORMAT_SYNC_LENGTH; ++i)
		{
			format.push_back(FORMAT_SYNC);
		}

		// ID Mark
		format.push_back(0xF5);
		format.push_back(0xF5);
		format.push_back(0xF5);
		format.push_back(0xFE);

		format.push_back(C);
		format.push_back(H);
		format.push_back(R);
		format.push_back(N);

		format.push_back(FORMAT_CRC);

		for(int i=0; i<FORMAT_GAP2_LENGTH; ++i)
		{
			format.push_back(FORMAT_GAP);
		}
		for(int i=0; i<FORMAT_SYNC_LENGTH; ++i)
		{
			format.push_back(FORMAT_SYNC);
		}

		// Data Mark
		format.push_back(0xF5);
		format.push_back(0xF5);
		format.push_back(0xF5);
		format.push_back(0xFB);  // F8 if Deleted Data Mark

		for(int i=0; i<(128<<N); ++i)
		{
			format.push_back(FORMAT_TEMP_DATA);
		}

		format.push_back(FORMAT_CRC);

		for(int i=0; i<FORMAT_GAP3_LENGTH; ++i)
		{
			format.push_back(FORMAT_GAP);
		}
	}

	while(format.size()<0x1800)
	{
		format.push_back(FORMAT_GAP);
	}

	return format;
}



int main(void)
{
	disk_image_raw img;

	disk_image_base_properties prop;
    prop.m_number_of_tracks=80;
    prop.m_spindle_time_ns=200000000;
    prop.m_sampling_rate=4000000;
    prop.m_data_bit_rate=500000;
	img.set_property(prop);

	std::cout << "Formatting..." << std::endl;
	img.create_empty_track_data(80);
	for(int trk=0; trk<80; ++trk)
	{
		uint8_t C=trk/2;
		uint8_t H=trk%2;

		fdc_bitstream fdc;

        fdc.set_fdc_params(prop.m_sampling_rate,prop.m_data_bit_rate);
        fdc.set_vfo_type(VFO_TYPE_DEFAULT);
        fdc.set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);

		bit_array bits;
		bits.resize(prop.m_sampling_rate/5);
        fdc.set_track_data(bits);

        fdc.set_pos(0);
		fdc.write_track(MakeFormatData(C,H)); // Memo to myself.  It does not stop at the index hole.  Extra bytes may wipe early sectors.

		fdc.set_pos(0);
		auto readBack=fdc.read_track();

		if(16!=fdc.read_all_idam().size())
		{
			std::cout << "Wrong sector count." << std::endl;
			return 1;
		}

		img.set_track_data(trk,fdc.get_track_data());
	}

	for(unsigned int trk=0; trk<img.get_number_of_tracks(); ++trk)
	{
		std::cout << "Track " << trk << std::endl;

		fdc_bitstream fdc;
        fdc.set_pos(0);
        fdc.set_fdc_params(prop.m_sampling_rate,prop.m_data_bit_rate);
        fdc.set_vfo_type(VFO_TYPE_DEFAULT);
        fdc.set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);

		fdc.set_track_data(img.get_track_data(trk));

		auto ids=fdc.read_all_idam();
		if(16!=ids.size())
		{
			std::cout << "Wrong sector count." << std::endl;
			std::cout << "  " << ids.size() << std::endl;
			return 1;
		}

		bool sector[16];
		for(auto &s : sector)
		{
			s=false;
		}
		for(auto id : ids)
		{
			if(1<=id.R && id.R<=16)
			{
				sector[id.R-1]=true;
			}
			else
			{
				std::cout << "Wrong sector number." << std::endl;
				return 1;
			}
		}
		for(auto s : sector)
		{
			if(s!=true)
			{
				std::cout << "Sector does not exist." << std::endl;
				return 1;
			}
		}
	}

	std::cout << "Pass!" << std::endl;
	return 0;
}
