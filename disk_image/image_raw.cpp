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
    constexpr uint8_t encode_base = ' ';
    constexpr uint8_t max_length = 'z' - encode_base;
    constexpr uint8_t extend_char = '{';
    while (std::getline(ifs, buf))
    {
        if (read_track_mode && buf[0] == '~') {         // decode bit stream data
            for (size_t i = 1; i < buf.size(); i++)
            {
                if(buf[i] != extend_char) {
                    size_t val = buf[i] - encode_base;
                    bit_pos += val;
                    track_data.set(bit_pos, 1);
                } else {
                    size_t val = max_length;
                    bit_pos += val;                 // extend the period but no pulse.
                }
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
                } else if(items[0] == "**BIT_RATE") {
                    m_base_prop.m_data_bit_rate = std::stoi(items[1]);
                } else if(items[0] == "**SAMPLING_RATE") {
                    m_base_prop.m_sampling_rate = std::stoi(items[1]);
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

std::vector<size_t> bitarray_to_dist(bit_array &bary) {
    std::vector<size_t> res;
    res.clear();
    bary.set_stream_pos(0);
    while(!bary.is_wraparound()) {
        size_t dist = bary.distance_to_next_pulse();
        res.push_back(dist);
    }
    return res;
}

void disk_image_raw::write(const std::string file_name) const {
    constexpr uint8_t encode_base = ' ';
    constexpr uint8_t max_length = 'z' - encode_base;
    constexpr uint8_t extend_char = '{';

    std::ofstream ofs(file_name, std::ios::out);        // not binary
    if(ofs.is_open() == false) {
        return;
    }
    // write the header
    ofs << "**BIT_RATE " << m_base_prop.m_data_bit_rate << std::endl;
    ofs << "**TRACK_RANGE 0 " << m_base_prop.m_number_of_tracks - 1 << std::endl;
    ofs << "**MEDIA_TYPE 2D" << std::endl;
    ofs << "**START" << std::endl;
    //ofs << "**DRIVE_TYPE 2D" << std::endl;
    ofs << "**SPIN_SPD " << m_base_prop.m_spindle_time_ns / 1e9 << std::endl;
    double track_time = static_cast<double>(m_track_data[0].get_bit_length()) / static_cast<double>(m_base_prop.m_sampling_rate);     // track time in (sec)
    double track_ratio = track_time / (m_base_prop.m_spindle_time_ns / 1e9);            // track time ratio to spin time 
    int overlap = static_cast<int>((track_ratio - 1.f) * 100.f);
    if(overlap < 0) overlap = 0;
    ofs << "**OVERLAP " << overlap << std::endl;
    ofs << "**SAMPLING_RATE " << m_base_prop.m_sampling_rate << std::endl;

    for(size_t track_n = 0; track_n < m_base_prop.m_number_of_tracks; track_n ++) {
        ofs << "**TRACK_READ " << track_n / 2 << " " << track_n % 2 << std::endl;
        bit_array track_data = m_track_data[track_n];
        size_t count = 0;
        track_data.set_stream_pos(0);
        while(!track_data.is_wraparound()) {
            size_t dist = track_data.distance_to_next_pulse();
            do {
                if(count % 100 == 0) {
                    ofs << "~";         // pulse data line leader
                }
                if(dist > max_length) {
                    ofs << static_cast<char>(extend_char);
                    dist -= max_length;
                } else {
                    ofs << static_cast<char>(dist + encode_base);
                    dist = 0;
                }
                if(++count % 100 == 0) {
                    ofs << std::endl;
                }
            } while(dist>0);
        }
        if(count % 100 != 0) {
            ofs << std::endl;
        }        
        ofs << "**TRACK_END" << std::endl;
    }
    ofs << "**COMPLETED" << std::endl;
    ofs.close();
}
