#pragma once

#include <vector>
#include <random>

#include "bit_array.h"

//#define DEBUG


class mfm_codec {
private:
    bit_array   m_track;
    bool        m_sync_mode;
    uint64_t    m_bit_stream;                         // decoded MFM bit stream from the raw track bit data
    size_t      m_bit_width = ((4e6 / 1e6) * 2);      // == 8  # 4MHz/1MHz * 2
    const uint16_t m_missing_clock_a1 = 0x4489;       //  0100_0100_10*0_1001
    const uint16_t m_missing_clock_c2 = 0x5224;       //  0101_0010_*010_0100
    const uint64_t m_pattern_ff = 0x5555555555555555; // for 4 bytes of sync data(unused)
    const uint64_t m_pattern_00 = 0xaaaaaaaaaaaaaaaa; // for 4 bytes of sync data

    bool        m_wraparound;                         // Flag to indicate that the head position went over the index hole (buffer wrap arounded)

    size_t m_prev_write_bit;        // to preserve the status of previous write data bit for MFM encoding

    // for data separator
    bit_array   m_track_bit_stream;
    size_t      m_current_bit_pos = 0;           // current bit position to process
    size_t      m_data_window_size = 5;          // data window size in [bits] unit
    size_t      m_data_window_ofst = 2;          // data window start offset in the data cell (in [bits] unit)
    size_t      m_bit_cell_size    = 8;          // bit cell size in [bits] unit   (4MHz sample, 500Kbps bit rate, MFM = 4.0e6/500e3 = 8)
    size_t      m_distance_to_next_pulse = 0;
    int         m_phase_adjuster = 0;            // data ceparator phase adjuster
    std::random_device m_rnd;
public:
    mfm_codec() : m_bit_stream(0),
                  m_sync_mode(false), m_wraparound(false),
                  m_prev_write_bit(0) {};

    void set_track_data(bit_array track);

    inline bool is_wraparound(void) { return m_wraparound; }
    inline void clear_wraparound(void) { m_wraparound = false; }
    inline size_t get_track_length(void) { return m_track.get_length(); }       // unit = bit

    int read_bit_ds(void);
    void mfm_read_byte(uint8_t& data, bool& missing_clock, bool ignore_missing_clock = true, bool ignore_sync_field = true);
    uint16_t mfm_encoder(uint8_t data, bool mode = false);
    void mfm_write_byte(uint8_t data, bool mode = false, bool write_gate = true);
    void set_pos(size_t bit_pos);
    size_t get_pos(void);
};
