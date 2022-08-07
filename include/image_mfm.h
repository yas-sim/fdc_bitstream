#pragma once

#include "../fdc_test/bit_arrray.h"

// Header (ofst=0)
typedef struct mfm_header_ {
    uint8_t     id_str[8];                  //  "MFM_IMG "
    uint64_t    track_table_offset;         // 
    uint64_t    number_of_tracks;           //
    uint64_t    spindle_time_ns;            //  Time for 1 rotation (ns unit) 
    uint64_t    data_bit_rate;              //  Data bit rate (bit/sec)    MFM=500Kbit/sec = 500,000
    uint64_t    sampling_rate;              //  Sampling rate of the bit stream data     4MHz = 4,000,000
} mfm_header;

// Track offset table
typedef struct track_table_ {
    uint64_t    offset;                     // Offset to the track data (unit=byte, from the top of the file == absolute offset)
    uint64_t    length_bit;                 // Track data length (uint=bits, not bytes)
} mfm_track_table;


class disk_image_mfm {
private:
    mfm_header              m_header;
    mfm_track_table         m_track_table[84];
    std::vector<bit_array>  m_track_data;
public:
    disk_image_mfm() {
        memset(&m_header, 0, sizeof(mfm_header));
        memset(&m_track_table, 0, sizeof(mfm_track_table));
    };

    void read(std::string file_name) {
        std::ifstream ifs;
        ifs.open(file_name, std::ios::in | std::ios::binary);
        ifs.read(reinterpret_cast<char*>(&m_header), sizeof(mfm_header));
        ifs.seekg(m_header.track_table_offset);
        ifs.read(reinterpret_cast<char*>(&m_track_table), sizeof(mfm_track_table) * 84);
        for (size_t track_n = 0; track_n < m_header.number_of_tracks; track_n++) {
            bit_array barray;
            std::vector<uint8_t> tdata;
            ifs.seekg(m_track_table[track_n].offset);
            size_t read_length = m_track_table[track_n].length_bit / 8 + ((m_track_table[track_n].length_bit % 8) ? 1 : 0);
            ifs.read(reinterpret_cast<char*>(tdata.data()), read_length);
            barray.set_array(tdata);
            m_track_data.push_back(barray);
        }
    }
    
};
