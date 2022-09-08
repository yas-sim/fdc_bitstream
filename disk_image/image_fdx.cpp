/**
 * @file image_fdx.cpp
 * @author Yasunori Shimura (yasu0710@gmail.com)
 * @brief `FDX` floppy image reader
 * @version 
 * @date 2022-09-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "image_fdx.h"

bit_array disk_image_fdx::simple_raw_to_mfm(bit_array &raw) const {
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

bit_array disk_image_fdx::simple_mfm_to_raw(bit_array &mfm) const {
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

void disk_image_fdx::read(const std::string file_name) {
    fdx_header             header;
    m_track_data_is_set = false;

    std::ifstream ifs = open_binary_file(file_name);
    ifs.read(reinterpret_cast<char*>(&header), sizeof(fdx_header));
    if (memcmp(header.signature, "FDX", 3) != 0) {               // Header signature mismatch
        return;
    }
    m_base_prop.m_number_of_tracks = header.cylinders * header.heads;
    m_base_prop.m_spindle_time_ns  = 60e9 / header.rpm;
    m_base_prop.m_data_bit_rate    = header.rate * 1e3;
    m_base_prop.m_sampling_rate    = 4e6;      // 4MHz fixed

    size_t data_type = header.type; // 00:2D MFM, 09:RAW

    ifs.seekg(sizeof(fdx_header));
    for (size_t track_n = 0; track_n < header.cylinders * header.heads; track_n++) {
        fdx_track_header track_header;
        std::vector<uint8_t> track_contents; 
        bit_array mfm_track;
        bit_array raw_track;
        ifs.read(reinterpret_cast<char*>(&track_header), sizeof(fdx_track_header));          // read a track data header
        track_contents.resize(header.track_block_size - sizeof(fdx_track_header));
        ifs.read(reinterpret_cast<char*>(track_contents.data()), header.track_block_size - sizeof(fdx_track_header));
        mfm_track.set_array(track_contents);
        switch(data_type) {
        default:
        case 0:
        case 1:
        case 2:
            raw_track = simple_mfm_to_raw(mfm_track);
            m_track_data[track_n] = raw_track;
            break;
        case 9:
            m_track_data[track_n] = mfm_track;      // no conversion
            break;
        }
    }
    m_track_data_is_set = true;
}


void disk_image_fdx::write(const std::string file_name) {
    fdx_header header;

    std::ofstream ofs;
    ofs.open(file_name, std::ios::out | std::ios::binary);

    memset(reinterpret_cast<char*>(header.signature), 0, sizeof(fdx_header));
    memcpy(reinterpret_cast<char*>(header.signature), "FDX", 3);
    header.revision = 3;
    memcpy(reinterpret_cast<char*>(header.disk_name), "FDX_IMAGE\0", 10);
    header.type = m_conversion_mode ? 9 : 0;                // 0:2D, 1:2DD, 2:2HD, 9:RAW
    header.cylinders = m_base_prop.m_number_of_tracks / 2;
    header.heads = 2;
    header.rate = m_base_prop.m_data_bit_rate / 1e3;        // 500 or 1000
    size_t rpm = 60e9 / m_base_prop.m_spindle_time_ns;
    header.rpm = (rpm < 360*1.05f && rpm > 360*0.95f) ? 360 : 300;
    header.write_protect = 0;
    header.option = 0;

    if(m_verbose) {
        std::cout << "VFX - " << (m_conversion_mode ? "RAW mode" : "MFM mode") << std::endl;
    }

    size_t curr_pos = sizeof(fdx_header);   // must be 256

    size_t track_buf_length = 0;
    track_buf_length = ((m_conversion_mode ? m_base_prop.m_sampling_rate : m_base_prop.m_data_bit_rate) * m_base_prop.m_spindle_time_ns) / 1e9;
    track_buf_length *= 1.1;
    track_buf_length /= 8;
    track_buf_length = ((track_buf_length/0x1000) + 1) * 0x1000;             // align with 0x1000 boundary

    for (size_t track_n = 0; track_n < m_base_prop.m_number_of_tracks; track_n++) {
        fdx_track_header track_header;
        bit_array mfm_track = m_track_data[track_n];
        if(m_conversion_mode==false) mfm_track = simple_raw_to_mfm(mfm_track);
        track_header.cylinder = track_n / 2;
        track_header.head     = track_n % 2;
        track_header.length   = mfm_track.get_length();               // track length is in bit unit
        track_header.index    = 0; //((m_conversion_mode ? m_base_prop.m_sampling_rate : m_base_prop.m_data_bit_rate) * m_base_prop.m_spindle_time_ns) / 1e9;
        if(track_header.index > track_header.length) track_header.length = track_header.index;      // safeguard
        ofs.seekp(curr_pos);
        ofs.write(reinterpret_cast<char*>(&track_header), sizeof(fdx_track_header));
        std::vector<uint8_t> track = mfm_track.get_array();
        track.resize(track_buf_length - sizeof(fdx_track_header));
        ofs.write(reinterpret_cast<char*>(track.data()), track.size());
        curr_pos += track_buf_length;
    }

    // write header
    //header.tracksize = curr_pos - sizeof(fdx_header);
    header.track_block_size = track_buf_length;
    ofs.seekp(0);
    ofs.write(reinterpret_cast<char*>(&header), sizeof(fdx_header));

    ofs.close();
}
