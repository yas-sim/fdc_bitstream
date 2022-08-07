#pragma once

#include "bit_array.h"

class disk_image {
private:
public:
    std::vector<bit_array>  m_track_data;

    disk_image() {
        m_track_data.clear();
        bit_array empty_bit_array;
        empty_bit_array.clear_array();
        for (size_t track_number = 0; track_number < 84; track_number++) {
            m_track_data.push_back(empty_bit_array);
        }
    };

    virtual void read(std::string file_name) = 0;
    virtual bit_array& get_track_data(size_t track_number) = 0;
};
