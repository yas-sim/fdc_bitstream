#pragma once

/**
* @file fdc_bitstream.h
*/

#include <vector>

#include "fdc_crc.h"
#include "mfm_codec.h"

//#define DEBUG

class fdc_bitstream {
private:
    enum class fdc_state {
        IDLE = 0,
        CHECK_MARK,
        READ_IDAM,
        READ_SECT
    } m_state;

    mfm_codec m_codec;              /** MFM codec object */
    fdc_crc m_crcgen;               /** CRC generator object */

    size_t m_sampling_rate;
    size_t m_data_bit_rate;
public:
    fdc_bitstream();
    void set_fdc_params(size_t sampling_rate, size_t data_bit_rate);
    std::vector<uint8_t> read_track(void);
    void write_track(const std::vector<uint8_t>& track_buf);
    void read_id(std::vector<uint8_t>& id_field, bool& crc_error);
    void read_sector(size_t sect_length_code, std::vector<uint8_t>& sect_data, bool& crc_error, bool& dam_type);
    void write_sector(std::vector<uint8_t> write_data, bool dam_type, bool write_crc=true);
    void set_pos(size_t bit_pos);
    size_t get_pos(void);
    void set_raw_track_data(bit_array track_data);

    inline bool is_wraparound(void) { return m_codec.is_wraparound(); }
    inline void clear_wraparound(void) { m_codec.clear_wraparound(); }

    void write_data(uint8_t data, bool mode = false, bool write_gate = true);
    void fdc_bitstream::read_data(uint8_t& data, bool& missing_clock, bool ignore_missing_clock=true, bool ignore_sync_field=true);
};
