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

bit_array disk_image::get_track_data(const size_t track_number) {
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
