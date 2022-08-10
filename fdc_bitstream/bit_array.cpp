#pragma once

#include "bit_array.h"

void bit_array::set_array(std::vector<uint8_t>& buf) {
    m_array_data.resize(buf.size()*8);
    size_t pos = 0;
    for (auto it = buf.begin(); it != buf.end(); ++it) {
        uint8_t dt = *it;
        for (uint8_t bit_mask = 0x80; bit_mask != 0; bit_mask >>= 1) {
            uint8_t bit = (dt & bit_mask) ? 1 : 0;
            m_array_data[pos++] = bit;
        }
    }
    m_stream_pos = 0;
}

void bit_array::set_array(size_t size_in_bits) {
    m_array_data.resize(size_in_bits);
    memset(m_array_data.data(), 0, size_in_bits);
    m_stream_pos = 0;
}

std::vector<uint8_t> bit_array::get_array(void) {
    std::vector<uint8_t> result;
    result.resize(m_array_data.size()/8 + ((m_array_data.size()%8)?1:0));
    uint8_t bit_ptn = 0x80;
    uint8_t dt = 0;
    size_t pos = 0;
    for (auto it = m_array_data.begin(); it != m_array_data.end(); ++it) {
        if (bit_ptn == 0) {
            bit_ptn = 0x80;
        }
        dt |= *it ? bit_ptn : 0;
        bit_ptn >>= 1;
        result[pos++] = dt;
    }
    if (bit_ptn) {
        result[pos++] = dt;
    }
    return result;
}

void bit_array::clear_array(void) {
    m_array_data.clear();
}

//@ buffer length in bit unit
size_t bit_array::get_length(void) {
    return m_array_data.size();
}

size_t bit_array::size(void) {
    return m_array_data.size();
}

void bit_array::resize(size_t bit_length) {
    m_array_data.resize(bit_length);
}

void bit_array::reserve(size_t bit_length) {
    m_array_data.reserve(bit_length);
}

//inline bool bit_array::is_wraparound(void) { return m_wraparound; }
//inline void bit_array::clear_wraparound_flag(void) { m_wraparound = false; }

void bit_array::set(size_t index, uint8_t value) {
    if (index >= m_array_data.size()) {
        m_array_data.resize(index + 10, 0);      // Extend the buffer
    }
    m_array_data[index] = value;
}

bool bit_array::get(size_t index) {
    bool res = m_array_data[index];
    return res;
}



bool bit_array::set_stream_pos(size_t position) {
    if (position < m_array_data.size()) {
        m_stream_pos = position;
    }
    else {
        m_stream_pos = m_array_data.size() - 1;
    }
    m_wraparound = false;
    return true;
}

size_t bit_array::get_stream_pos(void) {
    return m_stream_pos;
}



// elastic: true=extend the bit array if m_stream_pos go beyond the current length. false=no bit array extend (lap around)
void bit_array::write_stream(uint8_t value, bool elastic) {
    set(m_stream_pos++, value);
    if (m_stream_pos >= m_array_data.size() && elastic == false) {     // reached to the end of the bit array
        m_stream_pos = 0;           // wrap around only when elastic==false
        m_wraparound = true;
    }
}

void bit_array::advance_stream_pos(void) {
    m_stream_pos++;
    if (m_stream_pos >= m_array_data.size()) {     // wrap around
        m_stream_pos = 0;
        m_wraparound = true;
    }
}

uint8_t bit_array::read_stream(void) {
    uint8_t val = get(m_stream_pos++);
    if (m_stream_pos >= m_array_data.size()) {     // wrap around
        m_stream_pos = 0;
        m_wraparound = true;
    }
    return val;
}


size_t bit_array::distance_to_next_bit1(void) {
    size_t distance = 0;
    uint8_t val;
    do {
        val = read_stream();
        if (distance++ >= m_array_data.size()) {       // distance exceeded the entire bit array length
            return 0;
        }
    } while (val == 0);
    return distance;
}

void bit_array::fill_stream(uint8_t data, uint8_t length) {
    for (size_t i = 0; i < length; i++) {
        write_stream(data);
    }
}


void bit_array::dump(size_t start, size_t size) {
    if (size == 0) {
        size = m_array_data.size() - start;
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


void bit_array::save(std::string file_name) {
    std::ofstream ofs;
    ofs.open(file_name, std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<char*>(m_array_data.data()), m_array_data.size());       // data
    ofs.close();
}

void bit_array::load(std::string file_name) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in | std::ios::binary);
    if (ifs.is_open() == false) {
#ifdef DEBUG
        std::cerr << "Failed to open a file'" << file_name << "'." << std::endl;
#endif
        clear_array();
        return;
    }
    ifs.seekg(0, std::ios_base::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios_base::beg);
    size -= sizeof(size_t);
    set_array(size);
    ifs.read(reinterpret_cast<char*>(m_array_data.data()), size);
    m_stream_pos = 0;
}
