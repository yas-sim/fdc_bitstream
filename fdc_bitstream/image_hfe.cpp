/**
 * @file image_hfe.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief `HFE` floppy image reader (HxC floppy emulator format)
 * @version 
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#define DLL_BODY
#include "image_hfe.h"


void disk_image_hfe::read(const std::string file_name) {
	m_track_data_is_set = false;
	picfileformatheader header;
	std::ifstream ifs = open_binary_file(file_name);
	ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

	if (memcmp(header.HEADERSIGNATURE, "HXCPICFE", 8) != 0) {								// Header signature mismatch
		return;
	}
	if (header.number_of_track > 84) {
		header.number_of_track = 84;
	}
	m_base_prop.m_max_track_number = header.number_of_track * header.number_of_side;		// HFE(MFM,2D) = 42, 2
	if (header.floppyRPM == 0) {
		m_base_prop.m_spindle_time_ns = 0.2 * 1e9;
	}
	else {
		m_base_prop.m_spindle_time_ns = (60 * 1e9) / header.floppyRPM;
	}
	m_base_prop.m_data_bit_rate = header.bitRate * 2e3;			// HFE(MFM,2D) == 250??
	m_base_prop.m_sampling_rate = header.bitRate * 2e3;

	pictrack track_offset_table[84];
	ifs.seekg(header.track_list_offset * 0x0200, std::ios_base::beg);
	ifs.read(reinterpret_cast<char*>(track_offset_table), header.number_of_track * sizeof(pictrack));

	for (size_t track = 0; track < header.number_of_track; track++) {
		bit_array side0;
		bit_array side1;
		side0.resize(4e6 * 0.2);		// reserve capacity in advance to speed up
		side1.resize(4e6 * 0.2);
		side0.set_stream_pos(0);
		side1.set_stream_pos(0);
		std::vector<uint8_t> buf;
		size_t blocks = track_offset_table[track].track_len / 0x0200;
		size_t fraction = track_offset_table[track].track_len % 0x0200;
		size_t read_blocks = blocks + (fraction > 0 ? 1 : 0);
		size_t bit_cell_width = 8;
		buf.resize(read_blocks * 0x0200);
		ifs.seekg(track_offset_table[track].offset * 0x0200, std::ios_base::beg);
		ifs.read(reinterpret_cast<char*>(buf.data()), read_blocks * 0x0200);
		size_t blk_id;
		for (blk_id = 0; blk_id < blocks; blk_id++) {
			for (size_t ofst = 0; ofst < 0x0100; ofst++) {
				for (uint16_t bit_pos = 0x01; bit_pos < 0x100; bit_pos <<= 1) {
					int bit_data;
					bit_data = (buf[blk_id * 0x0200 + ofst] & bit_pos) ? 1 : 0;
					for (size_t j = 0; j < bit_cell_width; j++) {
						if (j == bit_cell_width / 2) side0.write_stream(bit_data, true);
						else                         side0.write_stream(0, true);
					}
					bit_data = (buf[blk_id * 0x0200 + ofst + 0x0100] & bit_pos) ? 1 : 0;
					for (size_t j = 0; j < bit_cell_width; j++) {
						if (j == bit_cell_width / 2) side1.write_stream(bit_data, true);
						else                         side1.write_stream(0, true);
					}
				}
			}
		}
		if (fraction > 0) {
			for (size_t ofst = 0; ofst < fraction / 2; ofst++) {
				for (uint16_t bit_pos = 0x01; bit_pos < 0x100; bit_pos <<= 1) {
					int bit_data;
					bit_data = (buf[blk_id * 0x0200 + ofst] & bit_pos) ? 1 : 0;
					for (size_t j = 0; j < bit_cell_width; j++) {
						if (j == bit_cell_width / 2) side0.write_stream(bit_data, true);
						else                         side0.write_stream(0, true);
					}

					bit_data = (buf[blk_id * 0x0200 + ofst + 0x0100] & bit_pos) ? 1 : 0;
					for (size_t j = 0; j < bit_cell_width; j++) {
						if (j == bit_cell_width / 2) side1.write_stream(bit_data, true);
						else                         side1.write_stream(0, true);
					}
				}
			}
		}
		m_track_data[track * 2 + 0] = side0;
		m_track_data[track * 2 + 1] = side1;
	}
	m_track_data_is_set = true;
}
