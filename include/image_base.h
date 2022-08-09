#pragma once

#include "fdc_defs.h"
#include "bit_array.h"

class disk_image {
private:
protected:
    bool        m_track_data_is_set;            /** true=track data is set and ready */
    size_t      m_max_track_number;
    uint64_t    m_sampling_frequency;           /** Disk image sampling frequency [Hz] */
    uint64_t    m_fdc_data_rate;                /** FDC bit data rate [bit/sec] */

    std::vector<bit_array>  m_track_data;

public:

    disk_image();

    virtual void read(std::string file_name) = 0;

    virtual bit_array& get_track_data(size_t track_number) {
        return m_track_data[track_number];
    }

    size_t media_max_track_number(media_type mtype);

    inline bool is_ready(void) { return m_track_data_is_set; }
};
