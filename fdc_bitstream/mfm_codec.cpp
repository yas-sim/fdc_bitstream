#pragma once

#include "mfm_codec.h"


void mfm_codec::set_track_data(bit_array track) {
    m_track = track;
    m_track.set_stream_pos(0);
    m_track.clear_wraparound_flag();
}

//inline bool is_wraparound(void) { return m_wraparound; }
//inline void clear_wraparound(void) { m_wraparound = false; }
//inline size_t get_track_length(void) { return m_track.get_length(); }       // unit = bit

mfm_codec::mfm_codec() : m_bit_stream(0),
    m_sync_mode(false), m_wraparound(false),
    m_prev_write_bit(0),
    m_sampling_rate(4e6), m_data_bit_rate(500e3),
    m_fluctuation(false), m_fluctuator_numerator(1), m_fluctuator_denominator(1)
{
    update_parameters();
};

void mfm_codec::update_parameters(void) {
    m_bit_cell_size = m_sampling_rate / m_data_bit_rate;
    m_data_window_size = m_bit_cell_size / 2;
    if (m_data_window_size == 0) {          // Avoid m_data_window_size==0 situation
        m_data_window_size = 1;
    }
    m_data_window_ofst = m_bit_cell_size / 4;
#ifdef DEBUG
    std::cout << "Bit cell size:" << m_bit_cell_size << std::endl;
    std::cout << "Data window size:" << m_data_window_size << std::endl;
    std::cout << "Data window offset:" << m_data_window_ofst << std::endl;
#endif
}

void mfm_codec::set_data_bit_rate(size_t data_bit_rate) {
    m_data_bit_rate = data_bit_rate;
    update_parameters();
}

void mfm_codec::set_sampling_rate(size_t sampling_rate) { 
    m_sampling_rate = sampling_rate; 
    update_parameters(); 
}


// ---------- Data separator

// How data separator work: 
// 
// | WWWW | WWWW | WWWW | WWWW | WWWW | WWWW | WWWW | WWWW |   <- Data window(W), Data cell boundary(|)
//     P           P           P         P         P           <- Data pulses
//     1     0      1      0       0      1      0      0      <- Data reading (only the data pulses within the data window will be considered) 
//
// 01234567
// | WWWW  |  <= In this case, bit cell size = 8, data window ofst = 2, data window size = 4
//
int mfm_codec::read_bit_ds(void) {
    int bit_reading = 0;
    size_t cell_center = m_data_window_ofst + m_data_window_size / 2;
    do {
        // check if the next bit is within the next data window
        if (m_distance_to_next_pulse < m_bit_cell_size) {
            // check pulse position
            if (m_distance_to_next_pulse >= m_data_window_ofst &&
                // regular pulse (within the data window)
                m_distance_to_next_pulse < m_data_window_ofst + m_data_window_size) {
                bit_reading = 1;
            }
            else {
                // irregular pulse
#ifdef DEBUG
                std::cout << '?';
#endif
            }
            // adjust pulse phase (imitate PLL operation)
            // limit the PLL operation frequency and introduce fluctuation with the random generator (certain fluctuation is required to reproduce some copy protection)
            if (m_fluctuation == false || m_rnd() % m_fluctuator_denominator >= m_fluctuator_numerator) {
                if (m_distance_to_next_pulse < cell_center) {
                    m_distance_to_next_pulse++;
#ifdef DEBUG
                    std::cout << '+';
#endif
                }
                else if (m_distance_to_next_pulse > cell_center) {
                    m_distance_to_next_pulse--;
#ifdef DEBUG
                    std::cout << '-';
#endif
                }
            }
            size_t distance = m_track.distance_to_next_bit1();
#if 0
            // give timing fluctuation (intentionally - to immitate the spndle rotation fluctuation)
            size_t rand_num = m_rnd() % 32;
            if (rand_num == 0) {
                distance++;
            }
            else if (rand_num == 1) {
                distance--;
            }
#endif
            m_distance_to_next_pulse += distance;
        }
    } while (m_distance_to_next_pulse < m_bit_cell_size);
    // advance bit cell position
    if (m_distance_to_next_pulse >= m_bit_cell_size) {
        m_distance_to_next_pulse -= m_bit_cell_size;
    }
    return bit_reading;
}



// ----------------------------------------------------------------



void mfm_codec::mfm_read_byte(uint8_t& data, bool& missing_clock, bool ignore_missing_clock, bool ignore_sync_field) {
    size_t decode_count = 0;
    missing_clock = false;
    do {
        int bit_data = read_bit_ds();
        if (m_track.is_wraparound()) {
            m_track.clear_wraparound_flag();
            m_wraparound = true;
        }
        m_bit_stream = (m_bit_stream << 1) | bit_data;

        if (ignore_missing_clock == false) {
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
        }
        if (ignore_sync_field) {
            if (m_bit_stream == m_pattern_00) {
                m_sync_mode = true;
            }
            else {
                m_sync_mode = false;
            }
        }
    } while (++decode_count < 16);
    uint8_t read_data = 0;
    // Extract only 'D' bits (exclude 'C' bits). MFM data is 'CDCDCDCD..CD'.
    for (uint16_t bit_pos = 0x4000; bit_pos > 0; bit_pos >>= 2) {
        read_data = (read_data << 1) | (m_bit_stream & bit_pos ? 1 : 0);
    }
    data = read_data;
    missing_clock = false;
    return;
}


// mode: false=normal, true=special(write track, etc. cares 'F5' and 'F6')
uint16_t mfm_codec::mfm_encoder(uint8_t data, bool mode) {
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
void mfm_codec::mfm_write_byte(uint8_t data, bool mode, bool write_gate) {
    uint16_t bit_pattern = mfm_encoder(data, mode);
    for (uint16_t bit_pos = 0x8000; bit_pos != 0; bit_pos >>= 1) {
        int bit = bit_pattern & bit_pos ? 1 : 0;
        if (write_gate == true) {
            for (size_t i = 0; i < m_bit_width; i++) {
                if (m_bit_width / 2 == i)   m_track.write_stream(bit);    // write data pulse at the center of the bit cell
                else                     m_track.write_stream(0);
            }
        }
        else {
            for (size_t i = 0; i < m_bit_width; i++) {
                m_track.advance_stream_pos();                          // advance stream pointer without actual data write (dummy write)
            }
        }
    }
}

void mfm_codec::set_pos(size_t bit_pos) {
    m_track.set_stream_pos(bit_pos);
}

size_t mfm_codec::get_pos(void) {
    return m_track.get_stream_pos();
}


void mfm_codec::enable_fluctuator(size_t numerator, size_t denominator) {
    m_fluctuation = true;
    m_fluctuator_numerator = numerator;
    m_fluctuator_denominator = denominator;
}

void mfm_codec::disable_fluctuator(void) {
    m_fluctuation = false;
}
