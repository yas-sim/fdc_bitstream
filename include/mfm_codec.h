#pragma once

#include <vector>

#include "bit_array.h"


class mfm_codec {
private:
    bool        m_sync_mode;
    bit_array   m_track;
    uint64_t    m_bit_stream;                         // decoded MFM bit stream from the raw track bit data
    size_t      m_bit_width = ((4e6 / 1e6) * 2);      // == 8  # 4MHz/1MHz * 2
    const uint16_t m_missing_clock_a1 = 0x4489;       //  0100_0100_10*0_1001
    const uint16_t m_missing_clock_c2 = 0x5224;       //  0101_0010_*010_0100
    const uint64_t m_pattern_ff = 0x5555555555555555; // for 4 bytes of sync data(unused)
    const uint64_t m_pattern_00 = 0xaaaaaaaaaaaaaaaa; // for 4 bytes of sync data

    // Partial decode data (MFM bits)
    size_t m_decoded_bit_length = 0;
    uint8_t m_decoded_bits = 0;

    bool        m_lap_around;                         // Flag to indicate that the head position went over the index hole (buffer wrap arounded)

    size_t m_prev_write_bit;        // to preserve the status of previous write data bit for MFM encoding

    inline void set_lap_around(void) { m_lap_around = true; }

    // bit_width * 2 (1.5-2.5), 3 (2.5-3.5), 4 (3.5-4.5)
    void decode_bits(uint8_t& data, size_t& length) {
        size_t prev_pos = m_track.get_stream_pos();
        size_t distance = m_track.distance_to_next_bit1();
        if (m_track.get_stream_pos() < prev_pos) {
            set_lap_around();                         // stream pointer returned to the top ( 1 rotation )
        }
        if (distance < m_bit_width * 1.5) {
            length = 1;
            data = 0b1;
        }
        else if (distance < m_bit_width * 2.5) {
            data = 0b10;
            length = 2;
        }
        else if (distance < m_bit_width * 3.5) {
            data = 0b100;
            length = 3;
        }
        else if (distance < m_bit_width * 4.5) {
            data = 0b1000;
            length = 4;
        }
        else {
            data = 0b10000;
            length = 5;
        }
    }

public:
    mfm_codec() : m_bit_stream(0),
        m_decoded_bit_length(0), m_decoded_bits(0),
        m_sync_mode(false), m_lap_around(false),
        m_prev_write_bit(0) {};

    void set_track_data(bit_array track) {
        m_track = track;
        m_track.set_stream_pos(0);
    }

    inline bool is_lap_around(void) { return m_lap_around; }
    inline void clear_lap_around(void) { m_lap_around = false; }
    inline size_t get_track_length(void) { return m_track.get_length(); }       // unit = bit

    void mfm_read_byte(uint8_t& data, bool& missing_clock) {
        size_t decode_count = 0;
        missing_clock = false;
        do {
            while (m_decoded_bit_length == 0) {                // !!!! potential dead loop
                decode_bits(m_decoded_bits, m_decoded_bit_length);
            }

            m_bit_stream = (m_bit_stream << 1) | (m_decoded_bits & 0x01);
            m_decoded_bit_length--;
            m_decoded_bits >>= 1;

            if ((m_bit_stream & 0x0ffffu) == m_missing_clock_a1) {      // Missing clock 0xA1 pattern
                data = 0xa1;
                missing_clock = true;
                return;
            }
            if ((m_bit_stream & 0x0ffffu) == m_missing_clock_c2) {       // Missing clock 0xC2 pattern
                data = 0xc2;
                missing_clock = true;
                return;
            }
            if (m_bit_stream == m_pattern_00) {
                m_sync_mode = true;
            }
            else {
                m_sync_mode = false;
            }
        } while (++decode_count < 16);
        uint8_t read_data = 0;
        uint16_t bit_pos = 0x4000;      // Extract only 'D' bits (exclude 'C' bits). MFM data is 'CDCDCDCD..CD'.
        for (int i = 0; i < 8; i++) {
            read_data = (read_data << 1) | (m_bit_stream & bit_pos ? 1 : 0);
            bit_pos >>= 2;
        }
        data = read_data;
        missing_clock = false;
        return;
    }


    // mode: false=normal, true=special(write track, etc)
    uint16_t mfm_encoder(uint8_t data, bool mode = false) {
        // Data swap for special code
        uint8_t write_data = data;
        if (mode == true) { // special coding mode (F5 and F6 will be converted to A1* C2* with missing-clock pattern respectively)
            switch (data) {
            case 0xf5:
                write_data = 0xa1;
                break;
            case 0xf6:
                write_data = 0xc2;
                break;
            }
        }
        // MFM encoding
        uint16_t bit_pattern = 0;
        uint16_t mfm_bit_pattern = 0;
        int current_bit;
        int clock_bit;
        for (size_t bit_pos = 0x80; bit_pos != 0; bit_pos >>= 1) {
            current_bit = (write_data & bit_pos) ? 1 : 0;
            clock_bit = (m_prev_write_bit == 0 && current_bit == 0) ? 1 : 0;
            mfm_bit_pattern = (mfm_bit_pattern << 1) | clock_bit;                 // clock
            mfm_bit_pattern = (mfm_bit_pattern << 1) | current_bit;               // data
            m_prev_write_bit = current_bit;
        }
        // Missing clock operation
        if (mode == true) {
            switch (data) {
            case 0xf5:
                mfm_bit_pattern &= ~0b100000u;              // remove a clock bit
                break;
            case 0xf6:
                mfm_bit_pattern &= ~0b10000000u;            // remove a clock bit
                break;
            }
        }
        return mfm_bit_pattern;
    }

    // mode: false=normal, true=special(write track, etc)
    // write_gate: true=perform actual write false=dummy write (no actual write will be perfored)
    void mfm_write_byte(uint8_t data, bool mode = false, bool write_gate = true) {
        uint16_t bit_pattern = mfm_encoder(data, mode);
        for (uint16_t bit_pos = 0x8000; bit_pos != 0; bit_pos >>= 1) {
            int bit = bit_pattern & bit_pos ? 1 : 0;
            if (write_gate == true) {
                m_track.write_stream(bit);
                for (size_t i = 0; i < m_bit_width-1; i++) {
                    m_track.write_stream(0);
                }
            }
            else {
                for (size_t i = 0; i < m_bit_width; i++) {
                    m_track.advance_stream_pos();   // advance stream pointer without actual data write (dummy write)
                }
            }
        }
    }

    void set_pos(size_t bit_pos) {
        m_track.set_stream_pos(bit_pos);
    }

    size_t get_pos(void) {
        return m_track.get_stream_pos();
    }
};
