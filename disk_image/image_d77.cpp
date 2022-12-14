#include "image_d77.h"
#include "d77img.h"
#include "byte_array.h"

#include "fdc_bitstream.h"

#include <iostream>

void disk_image_d77::read(const std::string file_name) {
    m_track_data_is_set = false;
    d77img input_image;
    input_image.read(file_name);
    m_base_prop.m_number_of_tracks = 0;
    m_base_prop.m_spindle_time_ns = (60*1e9)/300;
    m_base_prop.m_sampling_rate = 4e6;
    switch(input_image.m_disk_type) {
    case 0x00: // 2D
    case 0x10: // 2DD
    case 0x30: // 1D
    case 0x40: // 1DD
    default:
        m_base_prop.m_data_bit_rate = 500e3;
        break;
    case 0x20: // 2HD
        m_base_prop.m_data_bit_rate = 1e6;
        break;
    }

    bit_array track_data;
    mfm_codec codec;
    fdc_crc crcgen;

    for(size_t track_n=0; track_n<input_image.m_disk_data.size(); track_n ++) {
        // Format a track by parsing D77 track data
        track_data.clear_array();
        //track_data.set_array(m_base_prop.m_sampling_rate * (m_base_prop.m_spindle_time_ns / 1e9));
        codec.set_track_data(track_data);
        codec.set_pos(0);
#if 0
        // Parameters for the standard IBM format
        size_t gap4a = 80;
        size_t gap1  = 50;
        size_t gap2  = 22;
        size_t gap3  = 54;
        size_t gap4b = 152;
        size_t sync  = 12;
        bool index_address_mark = true;
#elif 0
        // Parameters for the ECMA/ISO standard format
        size_t gap4a = 32;
        size_t gap1  = 0;
        size_t gap2  = 22;
        size_t gap3  = 54;
        size_t gap4b = 266;
        size_t sync  = 12;
        bool index_address_mark = false;
#else
        // Parameters for the shrank format (to support protect format)
        size_t gap4a = 32;
        size_t gap1  = 0;
        size_t gap2  = 22;
        size_t gap3  = 32;
        size_t gap4b = 32;
        size_t sync  = 12;
        bool index_address_mark = false;
#endif
        // preamble
        for(size_t i=0; i < gap4a; i++) codec.mfm_write_byte(0x4e, false, true, true); // Gap4a
        if(index_address_mark) {
            for(size_t i=0; i <  sync; i++) codec.mfm_write_byte(0x00, false, true, true); // SYNC
            for(size_t i=0; i <     3; i++) codec.mfm_write_byte(0xf6, true , true, true); // 0xc2
                                            codec.mfm_write_byte(0xfc, false, true, true); // IAM
            for(size_t i=0; i <  gap1; i++) codec.mfm_write_byte(0x4e, false, true, true); // Gap1
        }
        // sectors
        d77img::track_data image_track = input_image.m_disk_data[track_n]; 
        if (image_track.size() == 0) continue;
        for(size_t sect_n=0; sect_n<image_track.size(); sect_n++) {
            d77img::sector_data sect = image_track[sect_n];
            byte_array sect_body = sect.m_sector_data;
            for(size_t i=0; i < sync; i++) codec.mfm_write_byte(0x00, false, true, true); // SYNC
            for(size_t i=0; i <    3; i++) codec.mfm_write_byte(0xf5, true , true, true); // 0xa1
            crcgen.reset();
            codec.mfm_write_byte(0xfe    , false, true, true); crcgen.data(0xfe); // IDAM
            codec.mfm_write_byte(sect.m_C, false, true, true); crcgen.data(sect.m_C);
            codec.mfm_write_byte(sect.m_H, false, true, true); crcgen.data(sect.m_H);
            codec.mfm_write_byte(sect.m_R, false, true, true); crcgen.data(sect.m_R);
            codec.mfm_write_byte(sect.m_N, false, true, true); crcgen.data(sect.m_N);
            uint16_t crcval;
            crcgen.data(0); crcgen.data(0);
            crcval = crcgen.get();
            switch(sect.m_status) {
            case 0xa0: // ID-CRC error
                codec.mfm_write_byte(~crcval >> 8, false, true, true); codec.mfm_write_byte(~crcval, false, true, true);   // CRC (cause CRC error intentionally)
                break;
            default:
                codec.mfm_write_byte(crcval >> 8, false, true, true); codec.mfm_write_byte(crcval, false, true, true);   // CRC
                break;
            }
            if(sect.m_status != 0xf0) {  // no DAM
                for(size_t i=0; i < gap2; i++) codec.mfm_write_byte(0x4e, false, true, true); // Gap2
                for(size_t i=0; i < sync; i++) codec.mfm_write_byte(0x00, false, true, true); // SYNC
                for(size_t i=0; i <    3; i++) codec.mfm_write_byte(0xf5, true, true, true);  // 0xa1
                crcgen.reset();
                switch(sect.m_dam_type) {
                case 0x10:
                    codec.mfm_write_byte(0xf8, false, true, true); crcgen.data(0xf8); // DDAM
                    break;
                default:
                    codec.mfm_write_byte(0xfb, false, true, true); crcgen.data(0xfb); // DAM
                    break;
                }
                for(size_t i=0; i<sect.m_sector_data_length; i++) {
                    uint8_t data=(i < sect.m_sector_data.size() ? sect_body[i] : 0);
                    codec.mfm_write_byte(data, false, true, true);
                    crcgen.data(data);
                }
                crcgen.data(0); crcgen.data(0);
                crcval = crcgen.get();
                switch(sect.m_status) {
                case 0xb0: // DT-CRC error
                    codec.mfm_write_byte(~crcval >> 8, false, true, true); codec.mfm_write_byte(~crcval, false, true, true);   // CRC (intentional CRC error)
                    break;
                default:
                    codec.mfm_write_byte(crcval >> 8, false, true, true); codec.mfm_write_byte(crcval, false, true, true);   // CRC
                    break;
                }
                for(size_t i=0; i < gap3; i++) codec.mfm_write_byte(0x4e, false, true, true); //Gap3
            }
        }
        for(size_t i=0; i < gap4b; i++) codec.mfm_write_byte(0x4e, false, true, true); //Gap4b

        track_data = codec.get_track_data();
        track_data.set_stream_pos(0);
        m_track_data[track_n] = track_data;
        m_base_prop.m_number_of_tracks = track_n+1;
    }
    m_track_data_is_set = true;
}


void disk_image_d77::write(const std::string file_name) const {
    const size_t sector_length_table[] = { 128, 256, 512, 1024 };
    std::ios::fmtflags flags_saved = std::cout.flags();
    fdc_bitstream fdc;
    d77img output_image;

    output_image.m_disk_name = "D77IMG";
    output_image.m_write_protect = 0;
    output_image.m_disk_type = 0;       // 0x00=2D 0x10=2DD 0x20=2HD 0x30=1D 0x40=1DD
    output_image.m_disk_size = 0;

    if(m_base_prop.m_data_bit_rate == 1e6) {   // 1Mbps == 2HD_MFM , 500Kbps == 2D/2DD_MFM
        output_image.m_disk_type = 0x20;        // 2HD
    } else if (m_base_prop.m_number_of_tracks > 84) {
        output_image.m_disk_type = 0x10;        // 2DD
    }

    size_t total_sector_good = 0;
    size_t total_sector_bad = 0;

    fdc.set_fdc_params(m_base_prop.m_sampling_rate,m_base_prop.m_data_bit_rate);

    for (size_t track_n = 0; track_n < m_base_prop.m_number_of_tracks; track_n++) {
        if(m_verbose) {
            std::cout << std::setw(4) << std::dec << track_n << ":";
        }
        d77img::track_data d77_trk;
        bit_array mfm_trk = m_track_data[track_n];
        fdc.set_track_data(mfm_trk);
        fdc.set_pos(0);
        fdc.set_vfo_type(m_vfo_type);
        fdc.set_vfo_gain_val(m_gain_l, m_gain_h);
        std::vector<fdc_bitstream::id_field> id_list = fdc.read_all_idam();
        size_t sector_good = 0;
        size_t sector_bad = 0;
        size_t cell_size_ref = m_base_prop.m_sampling_rate / m_base_prop.m_data_bit_rate;
        size_t sect_pos_ofst = (cell_size_ref * 16) * 16;       // sector read start position needs to be ahead a bit from the sector ID start position.
        for (size_t sect_n = 0; sect_n < id_list.size(); sect_n++) {
            d77img::sector_data sect_dt;
            sect_dt.m_C = id_list[sect_n].C;
            sect_dt.m_H = id_list[sect_n].H;
            sect_dt.m_R = id_list[sect_n].R;
            sect_dt.m_N = id_list[sect_n].N;
            sect_dt.m_dam_type = 0;         // temporary
            sect_dt.m_density = 0;
            sect_dt.m_num_sectors = id_list.size();
            fdc_bitstream::sector_data read_sect;
            if(id_list[sect_n].crc_sts == false) {                                   // no CRC error in IDAM
                size_t sect_pos = id_list[sect_n].pos;
                sect_pos = (sect_pos < sect_pos_ofst) ? 0 : sect_pos - sect_pos_ofst;
                fdc.set_pos(sect_pos);
                read_sect = fdc.read_sector(sect_dt.m_C, sect_dt.m_R);

                if(read_sect.record_not_found == false) {
                    if(read_sect.dam_type == false) {                               // not Record_not_found (read succeeded)
                        sect_dt.m_dam_type = 0;     // DAM
                        sect_dt.m_status   = 0;
                    } else {
                        sect_dt.m_dam_type = 0x10;  // DDAM
                        sect_dt.m_status   = 0x10;
                    }
                    sect_dt.m_status   = (true==read_sect.crc_sts && 0==sect_dt.m_status) ? 0xb0 : 0x00;
#if 0
                    // Use actual sector body length
                    sect_dt.m_sector_data = read_sect.data;
                    sect_dt.m_sector_data_length = read_sect.data.size();
#else
                    // Use ID.N information for the sector length
                    sect_dt.m_sector_data_length = sector_length_table[id_list[sect_n].N & 0x03];
                    sect_dt.m_sector_data = read_sect.data;
                    if(sect_dt.m_sector_data.size() < sect_dt.m_sector_data_length) {
                        sect_dt.m_sector_data.resize(sect_dt.m_sector_data_length);
                    }
#endif
                } else {                                                            // Record_not_found
                    sect_dt.m_status = 0xf0;    // no DAM
                    sect_dt.m_dam_type = 0;
                    sect_dt.m_sector_data_length = 0;
                    sect_dt.m_sector_data = byte_array();
                    sect_dt.m_sector_data.clear();
                    read_sect.record_not_found = true;
                    read_sect.crc_sts = false;
                }
            } else {                                                                // ID CRC error
                sect_dt.m_status = 0xa0;                                            // 0xa0 == ID_CRC error (D88/D77)
                sect_dt.m_dam_type = 0x00;
                sect_dt.m_sector_data_length = 0;
                sect_dt.m_sector_data = byte_array();
                sect_dt.m_sector_data.clear();
                read_sect.record_not_found = false;
                read_sect.crc_sts = false;
            }
            d77_trk.push_back(sect_dt);
            if (!read_sect.record_not_found && !read_sect.crc_sts && !id_list[sect_n].crc_sts) {  // no error (RNF or DT_CRC error or ID_CRC error)
                sector_good++;
                total_sector_good++;
            } else {
                sector_bad++;
                total_sector_bad++;
            }
        }
        if(m_verbose) {
            std::cout << std::setw(4) << std::dec << (sector_good+sector_bad) << "/" << std::setw(4) << sector_bad << "  ";
            if(track_n % 5 == 4) {
                std::cout << std::endl;
            }
        }
        output_image.m_disk_data.push_back(d77_trk);
    }
    output_image.write(file_name);
    if(m_verbose) {
        std::cout << std::endl;
        std::cout << "**TOTAL RESULT(GOOD/BAD):" << total_sector_good << " " << total_sector_bad << std::endl;
    }
    std::cout.flags(flags_saved);
}
