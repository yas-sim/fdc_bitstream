/**
 * @file image_raw.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief `RAW` floppy image reader
 * @version 
 * @date 2022-08-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "image_raw.h"

void disk_image_raw::read(const std::string file_name) {
    m_track_data_is_set = false;
    std::ifstream ifs = open_text_file(file_name);

    std::string  buf;
    bool read_track_mode = false;
    size_t cylinder = 0, side = 0;
    bit_array track_data;
    size_t bit_pos = 0;
    while (std::getline(ifs, buf))
    {
        if (read_track_mode && buf[0] == '~') {         // decode bit stream data
            for (size_t i = 1; i < buf.size(); i++)
            {
                size_t val = buf[i] - ' ';
                bit_pos += val;
                track_data.set(bit_pos, 1);
            }
        }
        else {
            std::vector<std::string> items;
            std::stringstream ss{ buf };
            std::string tmpbuf;
            while (std::getline(ss, tmpbuf, ' '))
            {
                items.push_back(tmpbuf);
            }
            if (items.size() >= 1) {
                if (items[0] == "**TRACK_RANGE") {
                }
                else if (items[0] == "**TRACK_READ") {
                    cylinder = std::stoi(items[1]);
                    side     = std::stoi(items[2]);
                    track_data.clear_array();
                    bit_pos = 0;
                    read_track_mode = true;
                }
                else if (items[0] == "**TRACK_END") {
                    m_track_data[cylinder * 2 + side] = track_data;
                    m_base_prop.m_number_of_tracks = cylinder * 2 + side + 1;
                    read_track_mode = false;
                }
                else if (items[0] == "**MEDIA_TYPE") {
                }
                else if (items[0] == "**START") {
                }
                else if (items[0] == "**DRIVE_TYPE") {
                }
                else if (items[0] == "**SPIN_SPD") {
                    m_base_prop.m_spindle_time_ns = std::stod(items[1]) * 1e9;
                }
                else if (items[0] == "**OVERLAP") {
                    m_overlap = std::stoi(items[1]);
                }
            }
        }
    }
    m_track_data_is_set = true;
}
