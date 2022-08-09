#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class bit_array
{
private:
    std::vector<uint8_t>    m_array_data;
    size_t                  m_bit_length;
    size_t                  m_stream_pos;
    bool                    m_wraparound;

    inline size_t to_byte_pos(size_t bit_pos) { return bit_pos >> 3; }
    inline uint8_t to_bit_pos(size_t bit_pos) { return 1 << (bit_pos & 0x07); }
public:

    bit_array() : m_bit_length(0), m_stream_pos(0),m_wraparound(false) {
        m_array_data.clear();
    }

    void set_array(std::vector<uint8_t>& buf);
    void set_empty_array(size_t size);
    std::vector<uint8_t> get_array(void);
    void clear_array(void);
    size_t get_length(void);
    size_t size(void);
    inline bool is_wraparound(void) { return m_wraparound; }
    inline void clear_wraparound_flag(void) { m_wraparound = false; }

    void set(size_t index, uint8_t value);
    bool get(size_t index);

    bool set_stream_pos(size_t position);
    size_t get_stream_pos(void);
    void write_stream(uint8_t value, bool elastic = false);
    void advance_stream_pos(void);
    uint8_t read_stream(void);
    size_t distance_to_next_bit1(void);
    void fill_stream(uint8_t data, uint8_t length = 1);

    void dump(size_t start = 0, size_t size = 0);

    void save(std::string file_name);
    void load(std::string file_name);
};
