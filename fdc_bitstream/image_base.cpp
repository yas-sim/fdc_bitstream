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
#define DLL_BODY
#include "image_base.h"

disk_image::disk_image() : m_track_data_is_set(false), m_max_track_number(0), m_sampling_rate(0), m_data_bit_rate(0), m_spindle_time_ns(0) {
    create_empty_track_data(84);
};

void disk_image::clear_track_data(void) {
    m_track_data.clear();
}

void disk_image::create_empty_track_data(size_t num_tracks) {
    clear_track_data();
    for (size_t track_number = 0; track_number < num_tracks; track_number++) {
        m_track_data.push_back(bit_array());
    }
}


std::ifstream disk_image::open_binary_file(std::string file_name) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in | std::ios::binary);
    if (!ifs) {
        throw disk_image_exception(-1, "Failed to open '" + file_name + "'." );
    }
    return ifs;
}

bit_array disk_image::get_track_data(size_t track_number) {
    if (track_number < m_track_data.size() && track_number <= m_max_track_number) {
        return m_track_data[track_number];
    }
    else {
        return bit_array();
    }
}


size_t disk_image::media_max_track_number(media_type mtype) {
    size_t max_track_number;
    switch (mtype) {
    case media_type::FLOPPY_2D:
        max_track_number = 84;
        break;
    case media_type::FLOPPY_2DD:
        max_track_number = 164;
        break;
    case media_type::FLOPPY_2HD:
        max_track_number = 164;
        break;
    default:
        max_track_number = 0;
        break;
    }
    return max_track_number;
}

