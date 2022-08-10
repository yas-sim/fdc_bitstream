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
    /** 
    * FDC state machine status definition.
    */
    enum class fdc_state {
        IDLE = 0,
        CHECK_MARK,                 /** Check special mark with missing clock pattern (A1/C2) */
        READ_IDAM,                  /** Read ID address mark */
        READ_SECT,                  /** Read Sector data */
        OPERATION_COMPLETED         /** Read data completed  */
    } m_state;

    mfm_codec m_codec;              /** MFM codec object */
    fdc_crc m_crcgen;               /** CRC generator object */

    size_t m_sampling_rate;         /** Track data sampling rate [Hz] (4MHz == 4e6) */
    size_t m_data_bit_rate;         /** FDC bit data rate [bit/sec] (2D/MFM == 500Kbit/sec == 5e3 */
public:
    /**
    * Structure for ID field data.
    */
    struct id_field {
        uint8_t     C;
        uint8_t     H;
        uint8_t     R;
        uint8_t     N;
        uint16_t    crc_val;
        bool        crc_sts;
        size_t      pos;
    };

    /** 
    * Structure for sector data.
    */
    struct sector_data {
        std::vector<uint8_t> data;
        bool        dam_type;      /** false:DAM, true : DDAM */
        bool        crc_sts;       /** true:error */
        bool        record_not_found;
        size_t      id_pos;
        size_t      data_pos;
    };

    fdc_bitstream();
    void set_fdc_params(size_t sampling_rate, size_t data_bit_rate);
    inline size_t get_track_length(void) { return m_codec.get_track_length(); }       /** unit = bit */
    void set_raw_track_data(bit_array track_data);
    void set_pos(size_t bit_pos);
    size_t get_pos(void);
    inline bool is_wraparound(void) { return m_codec.is_wraparound(); }
    inline void clear_wraparound(void) { m_codec.clear_wraparound(); }

    inline void enable_fluctuator(size_t numerator, size_t denominator) { m_codec.enable_fluctuator(numerator, denominator); } /** Enable FDC read operation fluctuatior */
    inline void disable_fluctuator(void) { m_codec.disable_fluctuator(); } /** Disable FDC read operation fluctuator */

    void write_data(uint8_t data, bool mode = false, bool write_gate = true);
    void fdc_bitstream::read_data(uint8_t& data, bool& missing_clock, bool ignore_missing_clock = true, bool ignore_sync_field = true);

    std::vector<uint8_t> read_track(void);
    void write_track(const std::vector<uint8_t>& track_buf);
    size_t read_id(std::vector<uint8_t>& id_field, bool& crc_error);
    std::vector<id_field> read_all_idam(void);
    size_t read_sector_body(size_t sect_length_code, std::vector<uint8_t>& sect_data, bool& crc_error, bool& dam_type);
    void write_sector_body(std::vector<uint8_t> write_data, bool dam_type, bool write_crc=true);

    sector_data read_sector(int trk, int sid, int sct);
    bool write_sector(int trk, int sid, int sct, bool dam_type, std::vector<uint8_t>& write_data);
};
