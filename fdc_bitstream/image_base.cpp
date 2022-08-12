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

disk_image::disk_image() : m_track_data_is_set(false), m_max_track_number(0), m_sampling_frequency(0), m_fdc_data_rate(0) {
    m_track_data.clear();
    bit_array empty_bit_array;
    empty_bit_array.clear_array();
    for (size_t track_number = 0; track_number < 84; track_number++) {
        m_track_data.push_back(empty_bit_array);
    }
};

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

