#pragma once

#include "dll_export.h"

#include <vector>
#include <random>

#include "bit_array.h"

//#define DEBUG

class DLL_EXPORT mfm_codec {
private:
    bit_array   m_track;
    bool        m_track_ready;
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
    size_t      m_sampling_rate;
    size_t      m_data_bit_rate;

    size_t      m_current_bit_pos   = 0;          // current bit position to process
    double      m_data_window_size  = 0.f;        // data window size in [bits] unit
    double      m_data_window_ofst  = 0.f;        // data window start offset in the data cell (in [bits] unit)
    double      m_bit_cell_size     = 0.f;        // bit cell size in [bits] unit   (4MHz sample, 500Kbps bit rate, MFM = 4.0e6/500e3 = 8)
    double      m_bit_cell_size_ref = 0.f;
    double      m_distance_to_next_pulse = 0.f;
    double      m_vfo_gain_h        = 10.f;       // VFO gain for SYNC period
    double      m_vfo_gain_l        = 1.f;        // VFO gain for period other than SYNC
    double      m_vfo_gain          = 1.f;        // Current VFO gain

    bool        m_fluctuation = false;
    size_t      m_fluctuator_numerator = 1;
    size_t      m_fluctuator_denominator = 4;    // data separator fluctuation occur rate = numerator/denominator
    std::random_device m_rnd;

public:
    mfm_codec();
    void reset(void);
    void set_cell_size(double cell_size, double window_ratio=0.75f);    // typical window_ratio for actual drive is 0.5 but uses 0.75 as default in this SW
    void update_parameters(void);
    void set_data_bit_rate(size_t data_bit_rate);
    void set_sampling_rate(size_t sampling_rate);

    void set_track_data(bit_array track);
    bit_array get_track_data(void);
    void unset_track_data(void);
    inline bool is_track_ready(void) { return m_track_ready; }
    inline void set_track_status_forcibly(bool status) { m_track_ready = status; };

    inline bool is_wraparound(void) { return m_wraparound; }
    inline void clear_wraparound(void) { m_wraparound = false; }
    inline size_t get_track_length(void) { return m_track.get_length(); }       /** unit = bit */

    int read_bit_ds(void);
    bool mfm_read_byte(uint8_t& data, bool& missing_clock, bool ignore_missing_clock = true, bool ignore_sync_field = true);
    void set_gain(double gain);
    uint16_t mfm_encoder(uint8_t data, bool mode = false);
    void mfm_write_byte(uint8_t data, bool mode = false, bool write_gate = true);
    void set_pos(size_t bit_pos);
    size_t get_pos(void);

    inline void reset_sync_mode(void) { m_sync_mode = false; }
    inline void set_sync_mode(bool sync_mode) { m_sync_mode = sync_mode; }
    inline void set_vfo_gain(double low, double high) { m_vfo_gain_h = high; m_vfo_gain_l = low; }

    void enable_fluctuator(size_t numerator, size_t denominator);
    void disable_fluctuator(void);
};
