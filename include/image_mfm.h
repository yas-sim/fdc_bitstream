#pragma once

#ifdef _WIN32
#ifdef DLL_BODY
#define DLL_EXPORT  __declspec(dllexport)
#else
#define DLL_EXPORT  __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif

#include <string.h>

#include "bit_array.h"
#include "image_base.h"

// Header (ofst(byte) = 0)
typedef struct mfm_header_ {
    uint8_t     id_str[8];                  //  "MFM_IMG "
    uint64_t    track_table_offset;         // 
    uint64_t    number_of_tracks;           //
    uint64_t    spindle_time_ns;            //  Time for 1 rotation (ns unit) 
    uint64_t    data_bit_rate;              //  Data bit rate (bit/sec)    MFM=500Kbit/sec = 500,000
    uint64_t    sampling_rate;              //  Sampling rate of the bit stream data     4MHz = 4,000,000
} mfm_header;


// Track offset table (ofst(byte) = header.track_table_offset)
typedef struct track_table_ {
    uint64_t    offset;                     // Offset to the track data (unit=byte, from the top of the file == absolute offset)
    uint64_t    length_bit;                 // Track data length (uint=bits, not bytes)
} mfm_track_table;


// Track data * 84
//   ofst(byte) = track_table[track#].offset
//   size(byte) = track_table[track#].length_bit/8 + (track_table[track#].length%8)?1:0)

class DLL_EXPORT disk_image_mfm : public disk_image {
private:
public:
    disk_image_mfm(void) : disk_image() {};

    void read(const std::string file_name) override;
    void write(const std::string file_name) override;

    inline size_t get_number_of_tracks(void) { return m_base_prop.m_max_track_number; }
    inline size_t get_spindle_time_ns(void)  { return m_base_prop.m_spindle_time_ns; }
    inline size_t get_data_bit_rate(void)    { return m_base_prop.m_data_bit_rate; }
    inline size_t get_sampling_rate(void)    { return m_base_prop.m_sampling_rate; }
};
