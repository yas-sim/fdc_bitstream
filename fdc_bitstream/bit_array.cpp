#include "bit_array.h"

/**
 * @brief Set a bit array data.
 * 
 * @param buf New array (std::vector<uint8_t>)
 */
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

/**
 * @brief Set an empty array. The new bit array will be filled with 0.
 * 
 * @param size_in_bits Size of the new empty bit array.
 */
void bit_array::set_array(size_t size_in_bits) {
    m_array_data.resize(size_in_bits);
    memset(m_array_data.data(), 0, size_in_bits);
    m_stream_pos = 0;
}

/**
 * @brief Get an copy of the bit array data.
 * 
 * @return std::vector<uint8_t> Bit array data.
 */
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

/**
 * @brief Clear bit array data. The length of the array will be 0.
 * 
 */
void bit_array::clear_array(void) {
    m_array_data.clear();
}

/**
 * @brief Get bit length of the bit array data.
 * 
 * @return size_t Buffer length in bit unit.
 */
size_t bit_array::get_length(void) {
    return m_array_data.size();
}

/**
 * @brief Get the size (==length) of the bit array data.
 * 
 * @return size_t Buffer length in bit unit.
 */
size_t bit_array::size(void) {
    return m_array_data.size();
}

/**
 * @brief Resize bit array data.
 * 
 * @param bit_length New length in bit unit.
 */
void bit_array::resize(size_t bit_length) {
    m_array_data.resize(bit_length);
}

/**
 * @brief Reserve bit array capacity for future array extention.
 * 
 * @param bit_length Capacity in bit to reserve.
 */
void bit_array::reserve(size_t bit_length) {
    m_array_data.reserve(bit_length);
}

//inline bool bit_array::is_wraparound(void) { return m_wraparound; }
//inline void bit_array::clear_wraparound_flag(void) { m_wraparound = false; }

/**
 * @brief Set a bit data to the bit array.
 * 
 * @param index Bit position to set the data.
 * @param value Data to set.
 */
void bit_array::set(size_t index, uint8_t value) {
    if (index >= m_array_data.size()) {
        m_array_data.resize(index + 10, 0);      // Extend the buffer
    }
    m_array_data[index] = value;
}

/**
 * @brief Get a bit data from the bit array.
 * 
 * @param index Bit position to read.
 * @return bool Read data.
 */
bool bit_array::get(size_t index) {
    bool res = m_array_data[index];
    return res;
}


/**
 * @brief Set read/write pointer for streaming operations.
 * 
 * @param position Bit position to read and write the bit array as bit stream.
 * @return true 
 * @return false 
 */
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

/**
 * @brief Get stream pointer position.
 * 
 * @return size_t Pointer position.
 */
size_t bit_array::get_stream_pos(void) {
    return m_stream_pos;
}



/**
 * @brief Write a bit data sequentially (stream mode)
 * 
 * @param value Data to write.
 * @param elastic Operation mode (false=non-elastic and ring-buffer mode. The pointer will wrap around at the end of the buffer / true=elastic, buffer will extend if pointer go over the end of the buffer )
 */
void bit_array::write_stream(uint8_t value, bool elastic) {
    set(m_stream_pos++, value);
    if (m_stream_pos >= m_array_data.size() && elastic == false) {     // reached to the end of the bit array
        m_stream_pos = 0;           // wrap around only when elastic==false
        m_wraparound = true;
    }
}

/**
 * @brief Advance the stream read/write pointer for 1 bit (without actual read/write operation)
 * 
 */
void bit_array::advance_stream_pos(void) {
    m_stream_pos++;
    if (m_stream_pos >= m_array_data.size()) {     // wrap around
        m_stream_pos = 0;
        m_wraparound = true;
    }
}

/**
 * @brief Read a bit data sequentially (stream mode). The streaming pointer will wrap around when the last bit is read.
 * 
 * @return uint8_t Read data.
 */
uint8_t bit_array::read_stream(void) {
    uint8_t val = get(m_stream_pos++);
    if (m_stream_pos >= m_array_data.size()) {     // wrap around
        m_stream_pos = 0;
        m_wraparound = true;
    }
    return val;
}

/**
 * @brief Calculate the distance from the current stream position to the next bit 1.
 * 
 * @return size_t Distance to next bit 1 (in bit unit).
 */
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

/**
 * @brief Fill bit array buffer from the current position with the specified value.
 * 
 * @param data Data to fill.
 * @param length Number of bits to fill.
 */
void bit_array::fill_stream(uint8_t data, uint8_t length) {
    for (size_t i = 0; i < length; i++) {
        write_stream(data);
    }
}

/**
 * @brief Dump bit array data.
 * 
 * @param start Start position (default=0).
 * @param size Number of bits to dump (default=size of bit array)
 */
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

/**
 * @brief Save bit arry data to a file.
 * 
 * @param file_name File name.
 */
void bit_array::save(std::string file_name) {
    std::ofstream ofs;
    ofs.open(file_name, std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<char*>(m_array_data.data()), m_array_data.size());       // data
    ofs.close();
}

/**
 * @brief Load bit array from a file.
 * 
 * @param file_name File name.
 */
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
