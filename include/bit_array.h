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

    void set_array(std::vector<uint8_t>& buf) {
        m_array_data = buf;
        m_bit_length = buf.size() * 8;
    }

    void set_empty_array(size_t size) {
        m_array_data.resize(size);
        memset(m_array_data.data(), 0, size);
        m_bit_length = size * 8;
    }

    std::vector<uint8_t> get_array(void) {
        return m_array_data;
    }

    void clear_array(void) {
        m_array_data.clear();
        m_bit_length = 0;
    }

    //@ buffer length in bit unit
    size_t get_length(void) {
        return m_bit_length;
    }

    size_t size(void) {
        return m_bit_length;
    }

    inline bool is_wraparound(void) { return m_wraparound; }
    inline void clear_wraparound_flag(void) { m_wraparound = false; }



    void set(size_t index, uint8_t value) {
        size_t  byte_pos = to_byte_pos(index);
        uint8_t bit_pos = to_bit_pos(index);

        if (byte_pos >= m_array_data.size()) {
            m_array_data.resize(byte_pos + 10, 0);      // Extend the buffer
        }
        if (m_bit_length <= index) {
            m_bit_length = index + 1;
        }

        if (value != 0) {
            m_array_data[byte_pos] |= bit_pos;
        }
        else {
            m_array_data[byte_pos] &= ~bit_pos;
        }
    }

    bool get(size_t index) {
        size_t  byte_pos = to_byte_pos(index);
        uint8_t bit_pos = to_bit_pos(index);
        bool res = (m_array_data[byte_pos] & bit_pos) ? 1 : 0;
        return res;
    }



    bool set_stream_pos(size_t position) {
        if (position >= m_bit_length) return false;
        m_stream_pos = position;
        m_wraparound = false;
        return true;
    }

    size_t get_stream_pos(void) {
        return m_stream_pos;
    }



    // elastic: true=extend the bit array if m_stream_pos go beyond the current length. false=no bit array extend (lap around)
    void write_stream(uint8_t value, bool elastic=false) {
        if (m_stream_pos >= m_bit_length) {     // lap around
            if (elastic == true) {
                set(m_bit_length, value);       // set() extends the bit array
                m_stream_pos++;
                return;
            }
            else {
                m_stream_pos = 0;
                m_wraparound = true;
            }
        }
        set(m_stream_pos++, value);
    }

    void advance_stream_pos(void) {
        if (m_stream_pos >= m_bit_length) {     // lap around
            m_stream_pos = 0;
            m_wraparound = true;
        }
        m_stream_pos++;
    }

    uint8_t read_stream(void) {
        if (m_stream_pos >= m_bit_length) {     // lap around
            m_stream_pos = 0;
            m_wraparound = true;
        }
        uint8_t val = get(m_stream_pos++);
        return val;
    }


    size_t distance_to_next_bit1(void) {
        size_t distance = 0;
        uint8_t val;
        do {
            val = read_stream();
            if (distance++ >= m_bit_length) {       // distance exceeded the entire bit array length
                return 0;
            }
        } while (val == 0);
        return distance;
    }

    uint8_t fill_stream(uint8_t data, uint8_t length = 1) {
        for (size_t i = 0; i < length; i++) {
            write_stream(data);
        }
    }


    void dump(size_t start = 0, size_t size = 0) {
        if (size == 0) {
            size = m_bit_length - start;
        }
        set_stream_pos(start);
        size_t count = 0;
        while (count++ < size) {
            uint8_t val = read_stream();
            std::cout << (val != 0 ? '1' : '0');
            if (count % 8 == 0) {
                std::cout << ' ';
            }
            if (count % 64 == 0) {
                std::cout << std::endl;
            }
        }
    }


    void save(std::string file_name) {
        std::ofstream ofs(file_name, std::ios::out | std::ios::binary);
        ofs.write(reinterpret_cast<char*>(&m_bit_length), sizeof(size_t));                  // bit length
        ofs.write(reinterpret_cast<char*>(m_array_data.data()), m_array_data.size());       // data
        std::cout << m_bit_length << std::endl;
        ofs.close();
    }

    void load(std::string file_name) {
        std::ifstream ifs(file_name, std::ios::in | std::ios::binary);
        ifs.seekg(0, std::ios_base::end);
        size_t size = ifs.tellg();
        ifs.seekg(0, std::ios_base::beg);
        ifs.read(reinterpret_cast<char*>(&m_bit_length), sizeof(size_t));
        std::cout << m_bit_length << std::endl;
        size -= sizeof(size_t);
        set_empty_array(size);
        ifs.read(reinterpret_cast<char*>(m_array_data.data()), size);
        m_stream_pos = 0;
    }
};
