#pragma once

#include "image_mfm.h"

void disk_image_mfm::read(std::string file_name) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in | std::ios::binary);
    if (ifs.is_open() == false) {
        std::cerr << "Failed to open file '" << file_name << "'." << std::endl;
        m_track_data.clear();
        return;
    }
    ifs.read(reinterpret_cast<char*>(&m_header), sizeof(mfm_header));
    ifs.seekg(m_header.track_table_offset);
    ifs.read(reinterpret_cast<char*>(&m_track_table), sizeof(mfm_track_table) * 84);
    for (size_t track_n = 0; track_n < m_header.number_of_tracks; track_n++) {
        bit_array barray;
        std::vector<uint8_t> tdata;
        ifs.seekg(m_track_table[track_n].offset);
        size_t read_length = m_track_table[track_n].length_bit / 8 + ((m_track_table[track_n].length_bit % 8) ? 1 : 0);
        tdata.resize(read_length);
        ifs.read(reinterpret_cast<char*>(tdata.data()), read_length);
        barray.set_array(tdata);
        m_track_data[track_n] = barray;
    }
}
