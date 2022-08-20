/**
 * @file image_mfm.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief `MFM` floppy image reader
 * @version 
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "image_mfm.h"

void disk_image_mfm::read(const std::string file_name) {
    mfm_header              header;
    mfm_track_table         track_table[164];
    m_track_data_is_set = false;

    std::ifstream ifs = open_binary_file(file_name);
    ifs.read(reinterpret_cast<char*>(&header), sizeof(mfm_header));
    if (memcmp(header.id_str, "MFM_IMG ", 8) != 0) {               // Header signature mismatch
        return;
    }
    m_base_prop.m_max_track_number = header.number_of_tracks;
    m_base_prop.m_spindle_time_ns = header.spindle_time_ns;
    m_base_prop.m_sampling_rate = header.sampling_rate;
    m_base_prop.m_data_bit_rate = header.data_bit_rate;

    ifs.seekg(header.track_table_offset);
    ifs.read(reinterpret_cast<char*>(track_table), header.number_of_tracks * sizeof(mfm_track_table));
    for (size_t track_n = 0; track_n < header.number_of_tracks; track_n++) {
        bit_array barray;
        std::vector<uint8_t> tdata;
        ifs.seekg(track_table[track_n].offset);
        size_t read_length = track_table[track_n].length_bit / 8 + ((track_table[track_n].length_bit % 8) ? 1 : 0);
        tdata.resize(read_length);
        ifs.read(reinterpret_cast<char*>(tdata.data()), read_length);
        barray.set_array(tdata);
        m_track_data[track_n] = barray;
    }
    m_track_data_is_set = true;
}


void disk_image_mfm::write(const std::string file_name) {
    mfm_header header;
    std::vector<mfm_track_table> track_table;

    std::ofstream ofs;
    ofs.open(file_name, std::ios::out | std::ios::binary);

    memcpy(reinterpret_cast<char*>(header.id_str), "MFM_IMG ", 8);
    header.track_table_offset = 0x100;
    header.number_of_tracks = m_base_prop.m_max_track_number;
    header.spindle_time_ns = m_base_prop.m_spindle_time_ns;
    header.data_bit_rate = m_base_prop.m_data_bit_rate;
    header.sampling_rate = m_base_prop.m_sampling_rate;
    ofs.write(reinterpret_cast<char*>(&header), sizeof(mfm_header));

    track_table.resize(header.number_of_tracks);

    size_t track_data_offset = align(header.track_table_offset + header.number_of_tracks * sizeof(mfm_track_table), 0x400);
    for (size_t track_n = 0; track_n < m_base_prop.m_max_track_number; track_n++) {
        std::vector<uint8_t> track = m_track_data[track_n].get_array();
        ofs.seekp(track_data_offset);
        ofs.write(reinterpret_cast<char*>(track.data()), track.size());
        track_table[track_n].length_bit = m_track_data[track_n].get_length();
        track_table[track_n].offset = track_data_offset;

        track_data_offset = align(track_data_offset + track.size(), 0x100);
    }
    ofs.seekp(header.track_table_offset);
    ofs.write(reinterpret_cast<char*>(track_table.data()), sizeof(mfm_track_table) * header.number_of_tracks);
    ofs.close();
}
