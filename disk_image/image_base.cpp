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
