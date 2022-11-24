/**
 * @file image_base.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief Base class for floppy image reader classes
 * @version 
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "image_base.h"

disk_image::disk_image() : m_verbose(false), m_track_data_is_set(false) {
    m_base_prop.m_data_bit_rate = 500e3;
    m_base_prop.m_number_of_tracks = 0;
    m_base_prop.m_sampling_rate = 4e6;
    m_base_prop.m_spindle_time_ns = 0.2 * 1e9;
    create_empty_track_data(164);
}

void disk_image::clear_track_data(void) {
    m_track_data.clear();
}

void disk_image::create_empty_track_data(size_t num_tracks) {
    clear_track_data();
    for (size_t track_number = 0; track_number < num_tracks; track_number++) {
        m_track_data.push_back(bit_array());
    }
}


std::ifstream disk_image::open_binary_file(const std::string file_name) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in | std::ios::binary);
    if (!ifs) {
        throw disk_image_exception(-1, "Failed to open '" + file_name + "'." );
    }
    return ifs;
}

std::ifstream disk_image::open_text_file(const std::string file_name) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in);
    if (!ifs) {
        throw disk_image_exception(-1, "Failed to open '" + file_name + "'.");
    }
    return ifs;
}

bit_array disk_image::get_track_data(const size_t track_number) const {
    if (track_number < m_track_data.size() && track_number < m_base_prop.m_number_of_tracks) {
        return m_track_data[track_number];
    }
    else {
        return bit_array();
    }
}

void disk_image::set_track_data(const size_t track_number, const bit_array track_data) {
    if (track_number >= 164) {
        assert(false);  return;
    }
    if (track_number >= m_track_data.size()) {
        m_track_data.resize(track_number + 1);
    }
    m_track_data[track_number] = track_data;
    if (m_base_prop.m_number_of_tracks <= track_number) {
        m_base_prop.m_number_of_tracks  = track_number+1;
    }
}


size_t disk_image::media_number_of_tracks(const media_type mtype) {
    size_t number_of_tracks;
    switch (mtype) {
    case media_type::FLOPPY_2D:
        number_of_tracks = 84;
        break;
    case media_type::FLOPPY_2DD:
        number_of_tracks = 164;
        break;
    case media_type::FLOPPY_2HD:
        number_of_tracks = 164;
        break;
    default:
        number_of_tracks = 0;
        break;
    }
    return number_of_tracks;
}

bit_array disk_image::simple_raw_to_mfm(bit_array raw) const {
    bit_array mfm;
    raw.set_stream_pos(0);
    mfm.clear_array();
    size_t bit_cell_size = m_base_prop.m_sampling_rate / m_base_prop.m_data_bit_rate;
    while(!raw.is_wraparound()) {
        size_t dist = raw.distance_to_next_pulse();
        dist = (dist + bit_cell_size / 2) / bit_cell_size;
        if(dist<2) continue;
        for(size_t i = 0; i < dist-1; i++) {
            mfm.write_stream(0, true);
        }
        mfm.write_stream(1, true);
    }
    return mfm;
}

bit_array disk_image::simple_mfm_to_raw(bit_array mfm) const {
    bit_array raw;
    mfm.set_stream_pos(0);
    raw.clear_array();
    size_t cell_width = m_base_prop.m_sampling_rate / m_base_prop.m_data_bit_rate;
    while(!mfm.is_wraparound()) {
        size_t bit = mfm.read_stream();
        for(size_t i = 0; i < cell_width; i++) {
            raw.write_stream((i == cell_width / 2) ? bit : 0, true);   // place the pulse at the center of the each bit cell.
        }
    }
    return raw;
}



#include "fdc_bitstream.h"

void disk_image::filter_for_FDX_export(void)
{
	for(size_t trk=0; trk<m_track_data.size(); ++trk)
	{
		std::cout << "Track " << trk << std::endl;
		filter_for_FDX_export_track(trk);
break;
	}
}
void disk_image::filter_for_FDX_export_track(int trk)
{
	auto &bits=m_track_data[trk];

	fdc_bitstream fdc;

    fdc.set_track_data(bits);
    fdc.set_pos(0);

	bool wraparound=false;

	fdc.clear_wraparound(); // Just in case;
	while(true!=wraparound)
	{
		auto lastPos=fdc.get_real_pos();
printf("Last pos %d\n",lastPos);

		// My problem:
		//   In the end, fdc.set_pos(60651) but then, in here I am getting lastPos=60632.
		// My solution:
		//   Added fdc_bitstream::set_real_pos.

		std::vector <uint8_t> id_field;
		bool id_crc_error;
		auto idStart=fdc.read_id(id_field,id_crc_error);

		if(0==idStart) // No Address Mark
		{
			break;
		}

		if(true==fdc.is_wraparound())
		{
			wraparound=true;
		}

		auto id_end_pos=fdc.get_real_pos();


		std::vector<uint8_t> sect_data;
		std::vector<size_t> byte_pos;
		bool data_crc_error,deleted_data,record_not_found;
		auto dataStart=fdc.read_sector_body_ex(id_field[3],sect_data,byte_pos,data_crc_error,deleted_data,record_not_found);

		auto data_end_pos=fdc.get_real_pos();

		if(0==dataStart) // No Data Mark
		{
			break;
		}

		if(true==fdc.is_wraparound())
		{
			wraparound=true;
		}

		std::cout << (int)id_field[0] << " " << (int)id_field[1] << " " << (int)id_field[2] << " " << (int)id_field[3];
		if(true==data_crc_error)
		{
			std::cout << " crc_error";
		}
		std::cout << std::endl;


		fdc.set_real_pos(lastPos);  // This clears internal is_wraparound flag.
		while(fdc.get_real_pos()<idStart)
		{
			auto byte_start=fdc.get_real_pos();

			uint8_t data;
			bool missing_clock;
			double error;
			fdc.read_byte(data,missing_clock,error,false,false);
			if(16<=error)
			{
				auto byte_end=fdc.get_real_pos();
				for(auto i=byte_start; i<byte_end; ++i)
				{
					bits.set(i,0);
				}
			}
		}

		fdc.set_real_pos(id_end_pos);
		while(fdc.get_real_pos()<dataStart)
		{
			auto byte_start=fdc.get_real_pos();

			uint8_t data;
			bool missing_clock;
			double error;
			fdc.read_byte(data,missing_clock,error,false,false);
			if(16<=error)
			{
				auto byte_end=fdc.get_real_pos();
				for(auto i=byte_start; i<byte_end; ++i)
				{
					bits.set(i,0);
				}
			}
		}


		if(true==data_crc_error)
		{
			for(auto byte_start : byte_pos)
			{
				fdc.set_real_pos(byte_start);

				uint8_t data;
				bool missing_clock;
				double error;
				fdc.read_byte(data,missing_clock,error,true,true);
				if(16<=error)
				{
					auto byte_end=fdc.get_real_pos();
					for(auto i=byte_start; i<byte_end; ++i)
					{
						bits.set(i,0);
					}
				}
			}
		}

printf("Data end %d\n",data_end_pos);
		fdc.set_real_pos(data_end_pos);
	}
}
