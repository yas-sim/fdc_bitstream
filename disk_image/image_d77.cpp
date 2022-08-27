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
        track_data.set_array(m_base_prop.m_sampling_rate * (m_base_prop.m_spindle_time_ns / 1e9));
        codec.set_track_data(track_data);
        codec.set_pos(0);
        // preamble
        for(size_t i=0; i<80; i++) codec.mfm_write_byte(0x4e, false); // Gap4a
        for(size_t i=0; i<12; i++) codec.mfm_write_byte(0x00, false); // SYNC
        for(size_t i=0; i< 3; i++) codec.mfm_write_byte(0xf6, true);  // 0xc2
                                   codec.mfm_write_byte(0xfc, false); // IAM
        for(size_t i=0; i<50; i++) codec.mfm_write_byte(0x4e, false); // Gap1
        // sectors
        d77img::track_data image_track = input_image.m_disk_data[track_n]; 
        if (image_track.size() == 0) continue;
        for(size_t sect_n=0; sect_n<image_track.size(); sect_n++) {
            d77img::sector_data sect = image_track[sect_n];
            byte_array sect_body = sect.m_sector_data;
            for(size_t i=0; i<12; i++) codec.mfm_write_byte(0x00, false); // SYNC
            for(size_t i=0; i< 3; i++) codec.mfm_write_byte(0xf5, true);  // 0xa1
            crcgen.reset();
            codec.mfm_write_byte(0xfe, false); crcgen.data(0xfe); // IDAM
            codec.mfm_write_byte(sect.m_C, false); crcgen.data(sect.m_C);
            codec.mfm_write_byte(sect.m_H, false); crcgen.data(sect.m_H);
            codec.mfm_write_byte(sect.m_R, false); crcgen.data(sect.m_R);
            codec.mfm_write_byte(sect.m_N, false); crcgen.data(sect.m_N);
            uint16_t crcval;
            crcgen.data(0); crcgen.data(0);
            crcval = crcgen.get();
            switch(sect.m_status) {
            case 0xa0: // ID-CRC error
                codec.mfm_write_byte(~crcval >> 8, false); codec.mfm_write_byte(~crcval, false);   // CRC (intentional CRC error)
                break;
            default:
                codec.mfm_write_byte(crcval >> 8, false); codec.mfm_write_byte(crcval, false);   // CRC
                break;
            }
            if(sect.m_status != 0xf0) {  // no DAM
                for(size_t i=0; i<22; i++) codec.mfm_write_byte(0x4e, false); // Gap2
                for(size_t i=0; i<12; i++) codec.mfm_write_byte(0x00, false); // SYNC
                for(size_t i=0; i< 3; i++) codec.mfm_write_byte(0xf5, true);  // 0xa1
                crcgen.reset();
                switch(sect.m_dam_type) {
                case 0x10:
                    codec.mfm_write_byte(0xfb, false); crcgen.data(0xf8); // DDAM
                    break;
                default:
                    codec.mfm_write_byte(0xfb, false); crcgen.data(0xfb); // DAM
                    break;
                }
                for(size_t i=0; i<sect.m_sector_data_length; i++) {
                    codec.mfm_write_byte(sect_body[i], false);
                    crcgen.data(sect_body[i]);
                }
                crcgen.data(0); crcgen.data(0);
                crcval = crcgen.get();
                switch(sect.m_status) {
                case 0xb0: // DT-CRC error
                    codec.mfm_write_byte(~crcval >> 8, false); codec.mfm_write_byte(~crcval, false);   // CRC (intentional CRC error)
                    break;
                default:
                    codec.mfm_write_byte(crcval >> 8, false); codec.mfm_write_byte(crcval, false);   // CRC
                    break;
                }
                for(size_t i=0; i<54; i++) codec.mfm_write_byte(0x4e, false); //Gap3
            }
        }
        for(size_t i=0; i<152; i++) codec.mfm_write_byte(0x4e, false); //Gap4b

        track_data = codec.get_track_data();
        m_track_data[track_n] = track_data;
        m_base_prop.m_number_of_tracks = track_n+1;
    }
    m_track_data_is_set = true;
}

void disk_image_d77::write(const std::string file_name) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    fdc_bitstream fdc;
    d77img output_image;

    output_image.m_disk_name = "D77IMG";
    output_image.m_write_protect = 0;
    output_image.m_disk_type = 0;       // 2D
    output_image.m_disk_size = 0;

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
            sect_dt.m_dam_type = 0;
            sect_dt.m_density = 0;
            sect_dt.m_num_sectors = id_list.size();
            sect_dt.m_status = id_list[sect_n].crc_sts ? 0xb0 : 0x00;  // DT-CRC error
            size_t sect_pos = id_list[sect_n].pos;
            sect_pos = (sect_pos < sect_pos_ofst) ? 0 : sect_pos - sect_pos_ofst;
            fdc.set_pos(sect_pos);
            fdc_bitstream::sector_data read_sect = fdc.read_sector(sect_dt.m_C, sect_dt.m_R);
            sect_dt.m_sector_data = read_sect.data;
            sect_dt.m_sector_data_length = read_sect.data.size();
            d77_trk.push_back(sect_dt);
            if (sect_dt.m_status == 0x00) {
                sector_good++;
            } else {
                sector_bad++;
            }
        }
        if(m_verbose) {
            std::cout << std::setw(4) << std::dec << (sector_good+sector_bad) << "/" << std::setw(4) << sector_good << "  ";
            if(track_n % 5 == 4) {
                std::cout << std::endl;
            }
        }
        output_image.m_disk_data.push_back(d77_trk);
    }
    output_image.write(file_name);
    if(m_verbose) {
        std::cout << std::endl;
    }
    std::cout.flags(flags_saved);
}
