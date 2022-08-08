#pragma once

#include <vector>

#include "fdc_crc.h"
#include "mfm_codec.h"

//#define DEBUG

class fdc_bitstream {
private:
    mfm_codec m_codec;
    enum fdc_state {
        IDLE = 0,
        CHECK_MARK,
        READ_IDAM,
        READ_SECT
    } m_state;
    fdc_crc m_crcgen;
public:
    fdc_bitstream() : m_state(fdc_state::IDLE) {};

    std::vector<uint8_t> read_track(void) {
        uint8_t read_data;
        bool missing_clock;
        std::vector<uint8_t> track_data;
        m_codec.clear_wraparound();
        while (m_codec.is_wraparound() == false) {
            m_codec.mfm_read_byte(read_data, missing_clock, false, false);
#ifdef DEBUG
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(read_data) << " ";
#endif
            track_data.push_back(read_data);
        }
        return track_data;
    }

    void write_track(const std::vector<uint8_t> &track_buf) {
        uint16_t crc_val;
        if (track_buf.size() == 0) return;
        for(auto it=track_buf.begin(); it!=track_buf.end(); ++it) {
            uint8_t data = *it;
            switch (data) {
            case 0xf5:      // marker A1
                write_data(data, true);
                m_crcgen.data(0xa1);
                break;
            case 0xf6:      // marker C2
                write_data(data, true);
                m_crcgen.data(0xc2);
                break;
            case 0xf7:      // write CRC (two bytes)
                m_crcgen.data(0x00);
                m_crcgen.data(0x00);
                crc_val = m_crcgen.get();
                write_data(crc_val >> 8);
                write_data(crc_val & 0xffu);
                break;
            case 0xf8:      // data address marks (reset CRC)
            case 0xf9:      // data address marks (reset CRC)
            case 0xfa:      // data address marks (reset CRC)
            case 0xfb:      // data address marks (reset CRC)
            case 0xfe:      // ID address marks (reset CRC)
                m_crcgen.reset();
                write_data(data);
                m_crcgen.data(data);
                break;
            default:
                write_data(data);
                m_crcgen.data(data);
                break;
            }
        }
    }

    void read_id(std::vector<uint8_t> &id_field, bool crc_error) {
        uint8_t read_data, mc_byte = 0;
        bool missing_clock;
        size_t read_count = 0;
        id_field.clear();
        crc_error = false;
        size_t total_read_count = 0;
        while(true) {
            switch (m_state) {
            case fdc_state::IDLE:
                m_codec.mfm_read_byte(read_data, missing_clock, false, false);
                if (missing_clock) {
                    m_state = fdc_state::CHECK_MARK;
                    mc_byte = read_data;
                }
                break;
            case fdc_state::CHECK_MARK:
                m_codec.mfm_read_byte(read_data, missing_clock, false, false);
                if (missing_clock) {
                    mc_byte = read_data;
                }
                else if (mc_byte == 0xa1 && (read_data == 0xfc || read_data == 0xfd || read_data == 0xfe || read_data == 0xff /* IDAMs*/)) {
                    id_field.clear();
                    id_field.push_back(read_data);
                    m_crcgen.reset();
                    m_crcgen.data(read_data);
                    read_count = 4 + 2;    // ID+CRC
                    m_state = fdc_state::READ_IDAM;
                }
                else {
                    m_state = fdc_state::IDLE;
                }
                break;
            case fdc_state::READ_IDAM:
                m_codec.mfm_read_byte(read_data, missing_clock, true, true);
                id_field.push_back(read_data);
                m_crcgen.data(read_data);
                if (--read_count == 0) {
                    crc_error = m_crcgen.get() == 0 ? false : true;
                    id_field.erase(id_field.begin());       // remove the ID mark on the top
                    m_state = fdc_state::IDLE;
                    return;
                }
                break;
            default:
                m_state = fdc_state::IDLE;
                break;
            }
            total_read_count++;
            if(total_read_count > m_codec.get_track_length()) {
                return;
            }
        }
    }

    // DDAM = f8, f9
    // DAM = fa, fb
    // sect_length_code = 0~3 
    void read_sector(size_t sect_length_code, std::vector<uint8_t> &sect_data, bool crc_error, bool dam_type) {
        std::vector<size_t> sector_length_table{ 128, 256, 512, 1024 };
        uint8_t read_data, mc_byte = 0;
        bool missing_clock;
        size_t read_count = 0;
        sect_data.clear();
        crc_error = false;
        dam_type  = false;
        size_t total_read_count = 0;
        while (true) {
            switch (m_state) {
            case fdc_state::IDLE:
                m_codec.mfm_read_byte(read_data, missing_clock, false, false);
                if (missing_clock) {
                    m_state = fdc_state::CHECK_MARK;
                    mc_byte = read_data;
                }
                break;
            case fdc_state::CHECK_MARK:
                m_codec.mfm_read_byte(read_data, missing_clock, false, false);
                if (missing_clock) {
                    mc_byte = read_data;
                }
                else if (mc_byte == 0xa1) {
                    if      (read_data == 0xfa || read_data == 0xfb) {     // Normal DAM
                        dam_type = false;
                    }
                    else if (read_data == 0xf8 || read_data == 0xf9) {     // DDAM
                        dam_type = true;
                    }
                    sect_data.clear();
                    sect_data.push_back(read_data);
                    read_count = sector_length_table[sect_length_code & 0b0011] + 2;    // sector field+CRC
                    m_state = fdc_state::READ_SECT;
                }
                else {
                    m_state = fdc_state::IDLE;
                }
                break;
            case fdc_state::READ_SECT:
                m_codec.mfm_read_byte(read_data, missing_clock, true, true);
                sect_data.push_back(read_data);
                if (--read_count == 0) {
                    m_crcgen.reset();
                    m_crcgen.data(sect_data);
                    crc_error = m_crcgen.get() == 0 ? false : true;
                    sect_data.erase(sect_data.begin());       // remove the ID mark on the top
                    sect_data.pop_back();
                    sect_data.pop_back();                    // remove CRC field
                    m_state = fdc_state::IDLE;
                    return;
                }
                break;
            default:
                m_state = fdc_state::IDLE;
                break;
            }
            total_read_count++;
            if (total_read_count > m_codec.get_track_length()) {
                return;
            }
        }
    }

    // dam_type : false=DAM, true=DDAM
    void write_sector(std::vector<uint8_t> write_data, bool dam_type) {
        for (int i = 0; i <  2; i++) m_codec.mfm_write_byte(0x00, false, false);     // WG==false == dummy write
        for (int i = 0; i <  8; i++) m_codec.mfm_write_byte(0x00, false, false);     // WG==false == dummy write
        for (int i = 0; i <  1; i++) m_codec.mfm_write_byte(0x00, false, false);     // WG==false == dummy write
        for (int i = 0; i < 11; i++) m_codec.mfm_write_byte(0x00, false, false);     // WG==false == dummy write
        for (int i = 0; i < 12; i++) m_codec.mfm_write_byte(0x00);                   // 0x00 x 12
        for (int i = 0; i <  3; i++) m_codec.mfm_write_byte(0xf5, true);             // 0xf5 (0xa1*) x 3

                                                                                     // reset CRC
        m_crcgen.reset();

        // write DAM/DDAM
        if (dam_type == false) {
            m_codec.mfm_write_byte(0xfb);     // 0xfb (normal DAM) x 1
            m_crcgen.data(0xfb);
        }
        else {
            m_codec.mfm_write_byte(0xf8);     // 0xf8 (Deleted DAM) x 1
            m_crcgen.data(0xf8);
        }

        // write sector data
        for (auto it = write_data.begin(); it != write_data.end(); ++it) {
            uint8_t data = *it;
            m_codec.mfm_write_byte(data);
            m_crcgen.data(data);
        }

        // write CRC
        m_crcgen.data(0x00);
        m_crcgen.data(0x00);
        uint16_t crc_val = m_crcgen.get();
        m_codec.mfm_write_byte(crc_val >> 8);
        m_codec.mfm_write_byte(crc_val & 0xffu);

        m_codec.mfm_write_byte(0xff);           // write 0xff (or 0x4f)
    }

    void set_pos(size_t bit_pos) {
        m_codec.set_pos(bit_pos);
        clear_wraparound();
    }

    size_t get_pos(void) {
        return m_codec.get_pos();
    }

    void set_raw_track_data(bit_array& track_data) {
        m_codec.set_track_data(track_data);
    }

    inline bool is_wraparound(void) { return m_codec.is_wraparound(); }
    inline void clear_wraparound(void) { m_codec.clear_wraparound(); }

    void write_data(uint8_t data, bool mode=false, bool write_gate=true) {
        m_codec.mfm_write_byte(data, mode, write_gate);
    }
};
