#pragma once

#include "dll_export.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <iomanip>

#include "byte_array.h"

class DLL_EXPORT d77img {
public:
	class DLL_EXPORT sector_data {
    public:
		uint8_t		m_C;
		uint8_t		m_H;
		uint8_t		m_R;
		uint8_t		m_N;
		uint16_t	m_num_sectors;
		uint8_t		m_density;			// 0x00:Double density, 0x40:Single density, 0x01:High density
		uint8_t		m_dam_type;			// 0x00:DAM, 0x10:DDAM
		uint8_t		m_status;			// 0x00:Normal, 0x10:DDAM, 0xa0:ID-CRC, 0xb0:DT-CRC, 0xe0:No IDAM, 0xf0:No DAM
		uint16_t	m_sector_data_length;
		byte_array	m_sector_data;
	public:
        sector_data() : m_C(0), m_H(0), m_R(0), m_N(0), m_num_sectors(0), m_density(0), m_dam_type(0), m_status(0), m_sector_data_length(0) {
            m_sector_data.clear();
        }
	};
	
	size_t	    m_image_size;
	byte_array	m_image_data;

	std::string m_disk_name;
	uint8_t		m_write_protect;		// 0x00=No-protect	0x10=Protected
	uint8_t		m_disk_type;			// 0x00:2D, 0x10:2DD, 0x20:2HD, 0x30:1D, 0x40:1DD
	uint32_t	m_disk_size;
	std::vector<uint32_t> m_track_offset_table;

	using track_data = std::vector<sector_data>;
	std::vector<track_data> m_disk_data;
public:
	d77img(void) : m_image_size(0), m_write_protect(0), m_disk_type(0), m_disk_size(0) {
		m_image_data.clear();
		m_disk_name.clear();
		m_disk_data.clear();
	}

	void read(const std::string& file_name);
    void write(std::string file_name);
	size_t media_max_track(void);
	byte_array read_sector(const size_t track, const size_t side, const size_t sect);
	bool write_sector(const size_t track, const size_t side, const size_t sect, const byte_array &sector_data);
};
