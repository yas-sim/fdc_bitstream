#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "image_base.h"

typedef struct picfileformatheader_
{
	uint8_t HEADERSIGNATURE[8];		// "HXCPICFE"
	uint8_t formatrevision;			// Revision 0
	uint8_t number_of_track;		// Number of track in the file
	uint8_t number_of_side;			// Number of valid side (Not used by the emulator)
	uint8_t track_encoding;			// Track Encoding mode
	// (Used for the write support - Please see the list above)
	uint16_t bitRate;				// Bitrate in Kbit/s. Ex : 250=250,000bits/s
	// Max value : 500
	uint16_t floppyRPM;				// Rotation per minute (Not used by the emulator)
	uint8_t floppyinterfacemode;	// Floppy interface mode. (Please see the list above.)
	uint8_t dnu;					// Free
	uint16_t track_list_offset;		// Offset of the track list LUT in block of 512 bytes
	// (Ex: 1=0x200)
	uint8_t write_allowed;			// The Floppy image is write protected ?
	uint8_t single_step;			// 0xFF : Single Step ? 0x00 Double Step mode
	uint8_t track0s0_altencoding;	// 0x00 : Use an alternate track_encoding for track 0 Side 0
	uint8_t track0s0_encoding;		// alternate track_encoding for track 0 Side 0
	uint8_t track0s1_altencoding;	// 0x00 : Use an alternate track_encoding for track 0 Side 1
	uint8_t track0s1_encoding;		// alternate track_encoding for track 0 Side 1
} picfileformatheader;

// floppyinterfacemode
enum floppyinterfacemode_t {
	IBMPC_DD_FLOPPYMODE				= 0x00,
	IBMPC_HD_FLOPPYMODE				= 0x01,
	ATARIST_DD_FLOPPYMODE			= 0x02,
	ATARIST_HD_FLOPPYMODE			= 0x03,
	AMIGA_DD_FLOPPYMODE				= 0x04,
	AMIGA_HD_FLOPPYMODE				= 0x05,
	CPC_DD_FLOPPYMODE				= 0x06,
	GENERIC_SHUGGART_DD_FLOPPYMODE	= 0x07,
	IBMPC_ED_FLOPPYMODE				= 0x08,
	MSX2_DD_FLOPPYMODE				= 0x09,
	C64_DD_FLOPPYMODE				= 0x0A,
	EMU_SHUGART_FLOPPYMODE			= 0x0B,
	S950_DD_FLOPPYMODE				= 0x0C,
	S950_HD_FLOPPYMODE				= 0x0D,
	DISABLE_FLOPPYMODE				= 0xFE
};

// track_encoding
enum track_encoding_t {
	ISOIBM_MFM_ENCODING				= 0x00,
	AMIGA_MFM_ENCODING				= 0x01,
	ISOIBM_FM_ENCODING				= 0x02,
	EMU_FM_ENCODING					= 0x03,
	UNKNOWN_ENCODING				= 0xFF
};

typedef struct pictrack_
{
	uint16_t offset;		// Offset of the track data in block of 512 bytes (Ex: 2=0x400)
	uint16_t track_len;		// Length of the track data in byte.
}pictrack;




class disk_image_hfe : public disk_image {
public:
	disk_image_hfe(void) {}

	void read(std::string file_name) {
		picfileformatheader header;
		std::ifstream ifs;
		ifs.open(file_name, std::ios::in | std::ios::binary);
		ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

		if (header.number_of_track > 84) {
			header.number_of_track = 84;
		}
		pictrack track_offset_table[84];
		ifs.seekg(header.track_list_offset * 0x0200, std::ios_base::beg);
		ifs.read(reinterpret_cast<char*>(track_offset_table), header.number_of_track * sizeof(pictrack));

		for (size_t track = 0; track < header.number_of_track; track++) {
			bit_array side0;
			bit_array side1;
			std::vector<uint8_t> buf;
			size_t blocks   = track_offset_table[track].track_len / 0x0200;
			size_t fraction = track_offset_table[track].track_len % 0x0200;
			size_t read_blocks = blocks + (fraction > 0 ? 1 : 0);
			size_t bit_cell_width = 8;
			buf.resize(read_blocks * 0x0200);
			ifs.seekg(track_offset_table[track].offset * 0x0200, std::ios_base::beg);
			ifs.read(reinterpret_cast<char*>(buf.data()), read_blocks * 0x0200);
			size_t blk_id;
			for (blk_id = 0; blk_id < blocks; blk_id++) {
				uint8_t dt;
				for (size_t ofst = 0; ofst < 0x0100; ofst++) {
					for (uint8_t bit_pos = 0x80; bit_pos > 0; bit_pos >>=1) {
						int bit_data = (buf[blk_id * 0x0200 + ofst     ] & bit_pos) ? 1 : 0;
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
				for (size_t ofst = 0; ofst < fraction/2; ofst++) {
					for (uint8_t bit_pos = 0x80; bit_pos > 0; bit_pos >>= 1) {
						int bit_data = (buf[blk_id * 0x0200 + ofst] & bit_pos) ? 1 : 0;
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
			m_track_data[track*2+0] = side0;
			m_track_data[track*2+1] = side1;
		}
	}
};
