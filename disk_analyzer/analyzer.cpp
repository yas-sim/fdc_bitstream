#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>


#ifdef _WIN32
#include <direct.h>
#include <stdio.h>
#include <conio.h>
#endif


#include "fdc_bitstream.h"

#include "disk_images.h"
#include "fdc_vfo_def.h"
#include "fdc_misc.h"

disk_image *disk_img = nullptr;
fdc_bitstream *fdc = nullptr;

double g_gain_l = VFO_GAIN_L_DEFAULT;
double g_gain_h = VFO_GAIN_H_DEFAULT;
size_t g_vfo_type = VFO_TYPE_DEFAULT;

size_t g_sampling_rate = 0;
size_t g_data_bit_rate = 0;
size_t g_spindle_time_ns = 0;
size_t g_number_of_tracks = 0;

void disp_status(void) {
    std::cout << "Gain L:" << g_gain_l << ", Gain H:" << g_gain_h << std::endl;
}

bool is_image_ready(void) {
    if(disk_img==nullptr) return false;
    return true;
}

std::string get_file_base(std::string file_name) {
    int period_pos = file_name.find_last_of(".");
    const std::string extension = file_name.substr(0, period_pos);
    return extension;
}

std::string get_file_extension(std::string file_name) {
    int period_pos = file_name.find_last_of(".");
    const std::string extension = file_name.substr(period_pos + 1, file_name.length());
    return extension;
}

bool check_extension(std::string extension) {
    std::vector<std::string> allowed = { "hfe", "mfm", "raw", "d77" };
    bool res = false;
    for(auto it = allowed.begin(); it != allowed.end(); ++it) {
        if(*it == extension) res = true;
    }
    return res;
}

disk_image* create_object_by_ext(std::string ext) {
    disk_image *obj;
    if(ext == "hfe") obj = new disk_image_hfe();
    if(ext == "raw") obj = new disk_image_raw();
    if(ext == "mfm") obj = new disk_image_mfm();
    if(ext == "d77") obj = new disk_image_d77();
    return obj;
}

// -------------------------------------------------------------------------

void cmd_open_image(std::string file_name) {
    std::ifstream dmy(file_name, std::ios::in);
    if(!dmy.is_open()) {
        std::cout << "File not found." << std::endl;
        dmy.close();
        return;
    }
    std::string ext = get_file_extension(file_name);
    if(check_extension(ext) == false) {
        fdc_misc::color(2);
        std::cout << "Unsupported file extension." << std::endl;
        fdc_misc::color(7);
        return;
    }
    disk_img = create_object_by_ext(ext);
    disk_img->read(file_name);

    disk_image_base_properties props = disk_img->get_property();
    std::cout << "Track num     : " << props.m_number_of_tracks << std::endl;
    std::cout << "Spindle speed : " << props.m_spindle_time_ns / 1e6 << " [ms/rotation]" << std::endl;
    std::cout << "Sampling rate : " << props.m_sampling_rate / 1e6 << " [Msamples/sec]" << std::endl;
    std::cout << "Data bit rate : " << props.m_data_bit_rate / 1e3 << " [Kbit/sec]" << std::endl;

    fdc->set_fdc_params(props.m_sampling_rate, props.m_data_bit_rate);
    fdc->disp_vfo_status();

    g_sampling_rate = props.m_sampling_rate;
    g_data_bit_rate = props.m_data_bit_rate;
    g_spindle_time_ns = props.m_spindle_time_ns;
    g_number_of_tracks = props.m_number_of_tracks;
}

void cmd_write_image(std::string file_name) {
    std::string ext = get_file_extension(file_name);
    if (ext != "mfm" && ext != "d77") {
        fdc_misc::color(2);
        std::cout << "Unsupported file extension." << std::endl;
        fdc_misc::color(7);
        return;        
    }
    disk_image_base_properties props;
    props.m_sampling_rate = g_sampling_rate;
    props.m_data_bit_rate = g_data_bit_rate;
    props.m_spindle_time_ns = g_spindle_time_ns;
    props.m_number_of_tracks = g_number_of_tracks;
    disk_image *out_img = create_object_by_ext(ext);
    out_img->set_property(props);
    out_img->set_track_data_all(disk_img->get_track_data_all());
    out_img->write(file_name);
    delete out_img;
}

void cmd_read_track(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    std::vector<std::vector<size_t>> read_data;
    track_stream = disk_img->get_track_data(track_n);
    fdc->set_track_data(track_stream);
    read_data = fdc->read_track_ex();
    constexpr size_t cols = 32;
    constexpr size_t rows = 16;
    //fdc_misc::dump_buf(read_data.first.data(), read_data.first.size(), true, cols, rows, true, read_data.second.data());
    fdc_misc::dump_buf2(read_data);
}

void cmd_read_id(size_t track_n, size_t track_end_n = -1) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    if (track_end_n == -1) track_end_n = track_n;
    for(size_t trk_n=track_n; trk_n <= track_end_n; trk_n++) {
        std::cout << "Read ID (" << trk_n << ")" << std::endl;
        bit_array track_stream;
        std::vector<fdc_bitstream::id_field> read_data;
        track_stream = disk_img->get_track_data(trk_n);
        fdc->set_track_data(track_stream);
        read_data = fdc->read_all_idam();
        fdc_misc::display_id_list(read_data, true);
    }
}

void trim_track(bit_array &in_array, bit_array &out_array, size_t start_byte, size_t end_byte) {
    constexpr size_t err = 99999999;
    size_t start_pos=err, end_pos=err, prev_pos;
    size_t read_count = 0;
    fdc->set_track_data(in_array);
    fdc->set_pos(0);
    uint8_t dt;
    bool mc;
    std::cout << start_byte << " " << end_byte << std::endl;
    do {
        prev_pos = fdc->get_pos();
        fdc->read_data(dt, mc, false, false);
        if(read_count == start_byte) start_pos = prev_pos;
        if(read_count == end_byte)   end_pos   = fdc->get_pos();
        read_count++;
    } while(!fdc->is_wraparound());
    if(start_pos == err || end_pos == err) {
        std::cout << "Could not find start or end position." << std::endl;
        out_array = in_array;
        return;
    }
    out_array.clear_array();
    in_array.set_stream_pos(start_pos);
    for(size_t pos = start_pos; pos<end_pos; pos++) {
        out_array.write_stream(in_array.read_stream(), true);
    }
}

void cmd_trim_track(size_t track_n, size_t start_byte, size_t end_byte) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    size_t track_n_s, track_n_e;
    if (track_n == 65535) { // target = all track
        track_n_s = 0;
        track_n_e = disk_img->get_number_of_tracks()-1;
    } else {
        track_n_s = track_n_e = track_n;
    }

    for(size_t trk_n = track_n_s; trk_n <= track_n_e; trk_n++) {
        bit_array track_stream = disk_img->get_track_data(trk_n);
        bit_array track_trimmed;
        track_trimmed.clear_array();
        trim_track(track_stream, track_trimmed, start_byte, end_byte);

        disk_img->set_track_data(trk_n, track_trimmed);
        std::cout << "Track " << trk_n << " : Trimmed down track data from " << std::dec << track_stream.get_length() << "b to " << track_trimmed.get_length() << "b." << std::endl;
    }
}

void cmd_validate_track(size_t track_n, size_t track_end_n = -1) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    if (track_end_n == -1) track_end_n = track_n;
    for(size_t trk_n=track_n; trk_n <= track_end_n; trk_n++) {
        std::cout << std::endl << "Track " << trk_n << std::endl;
        bit_array track_stream;
        std::vector<fdc_bitstream::id_field> id_data;
        track_stream = disk_img->get_track_data(trk_n);
        fdc->set_track_data(track_stream);
        id_data = fdc->read_all_idam();
        size_t cell_size_ref = g_sampling_rate / g_data_bit_rate;
        size_t sect_pos_ofst = (cell_size_ref * 16) * 16;
        std::vector<fdc_bitstream::sector_data> sect_data;
        for(auto it = id_data.begin(); it != id_data.end(); ++it) {
            fdc_bitstream::id_field id = *it;
            size_t sect_pos = id.pos;
            sect_pos = (sect_pos < sect_pos_ofst) ? 0 : sect_pos - sect_pos_ofst;
            fdc->set_pos(sect_pos);
            sect_data.push_back(fdc->read_sector(id.C, id.R));
        }
        //           "  1: 00 00 01 01 fa0c ID-CRC_OK   256 DAM  DT-CRC OK  RNF_OK  IDAM_POS=    5208 DAM_POS=   10923"
        std::cout << "  #: CC HH RR NN --- ID CRC ---  SIZE" << std::endl;
        for(size_t i = 0; i < id_data.size(); i++) {
            std::cout << std::setw(3) << std::setfill(' ') << i+1 << ": ";
            fdc_misc::display_id(id_data[i], true);
            fdc_misc::display_sector_data(sect_data[i], true);
            std::cout << std::endl;
        }        
    }
}

void cmd_set_gain(double gain_l, double gain_h) {
    fdc->set_vfo_gain_val(gain_l, gain_h);
    g_gain_l = gain_l;      // for VFO visualizer
    g_gain_h = gain_h;
    std::cout << "gain=(L:" << gain_l << ", H:" << gain_h << ")" << std::endl;
    fdc->disp_vfo_status();
}

void cmd_read_sector(size_t cyl, size_t rcd, bool pulse_vis) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    fdc_bitstream::sector_data read_data;
    track_stream = disk_img->get_track_data(cyl);
    fdc->set_track_data(track_stream);
    if(rcd >= 1000) {   // read by sector index #
        std::vector<fdc_bitstream::id_field> id_list = fdc->read_all_idam();   // get the list of all IDs
        size_t sct_idx = rcd - 1000;
        if (sct_idx >= id_list.size()) {
            std::cout << "Index exceeded (the number of sectors)-1 in the track. (" << sct_idx << ">" << id_list.size()-1 << ")." << std::endl;
            return;
        }
        fdc_bitstream::id_field sct_id = id_list[sct_idx];
        std::cout << "Sector read by index : (" << std::dec << sct_idx + 1 << ") " << std::hex 
                << std::setfill('0')
                << std::setw(2) << static_cast<int>(sct_id.C) << " " << std::setw(2) << static_cast<int>(sct_id.H) << " "
                << std::setw(2) << static_cast<int>(sct_id.R) << " " << std::setw(2) << static_cast<int>(sct_id.N) << std::dec << std::endl;
        size_t cell_size = g_sampling_rate / g_data_bit_rate;
        size_t rewind = (cell_size * 16 /*1byte*/) * 16;
        size_t pos = sct_id.pos;
        pos = (pos >= rewind) ? pos-rewind : 0;
        cyl = sct_id.C;
        rcd = sct_id.R;
        fdc->set_pos(pos); // rewind the start position a bit.
    } else {
        std::cout << "Sector read : " << cyl << ", " << rcd << std::endl;
        cyl /= 2;
    }
    read_data = fdc->read_sector(cyl, rcd);
	if(true!=pulse_vis)
	{
        // hex dump
	    fdc_misc::dump_buf(read_data.data.data(), read_data.data.size(), true, 16, 16, true);
	}
	else
	{
        // bit dump
		for(size_t i=0; i<read_data.data.size() && i<read_data.pos.size(); ++i)
		{
            if(i % 8==0) { // display standard bit cell guide (ruler)
                size_t standard_bit_cell = g_sampling_rate / g_data_bit_rate;
                std::cout << std::string(19, ' ');
                for(size_t ii=0; ii<8; ii++) {
                    std::cout << "|" << std::string(standard_bit_cell-1, 'c');
                    std::cout << "|" << std::string(standard_bit_cell-1, 'D');
                }
                std::cout << std::endl;
            }
			size_t pulse_end=(i+1<read_data.pos.size() ? read_data.pos[i+1] : read_data.data_end_pos);
			std::cout << "+" << std::hex << std::setw(2) << i << ":";
			std::cout << std::hex << std::setw(2) << std::setfill('0') << int(read_data.data[i]);
			std::cout << std::dec << std::setw(10) << std::setfill(' ') << read_data.pos[i];
			std::cout << "   ";
			for(int j=read_data.pos[i]; j<pulse_end; ++j)
			{
				std::cout << (int(track_stream.get(j)) ? "#":".");
			}
			std::cout << std::endl;
		}
	}
    size_t end_pos = fdc->get_pos();
    if (end_pos < read_data.id_pos) {
        end_pos += track_stream.get_length();       // wrap around correction
    }
    double read_time = (static_cast<double>(end_pos - read_data.id_pos)*1000.f) / g_sampling_rate;
    std::cout << "CRC DAM  RNF ----ID_POS ---DAM_POS ---END_POS ---TIME(ms) SIZE" << std::endl;
    std::cout << (read_data.crc_sts ? "ERR " : "OK  ") << (read_data.dam_type ? "DDAM " : "DAM  ") << (read_data.record_not_found ? "ERR " : "OK  ");
    std::cout << std::setfill(' ');
    std::cout << std::setw(10) << read_data.id_pos << " " << std::setw(10) << read_data.data_pos << " " << 
        std::setw(10) << end_pos << " " << std::setw(10) << std::setprecision(10) << read_time << " " << read_data.data.size() << std::endl;
}


void cmd_enable_fluctuator(double vfo_suspension_rate) {
    fdc->enable_fluctuator(vfo_suspension_rate);
    std::cout << "VFO stops operation at rate of " << vfo_suspension_rate << "." << std::endl;
}

void cmd_disable_fluctuator(void) {
    fdc->disable_fluctuator();
}

void cmd_disp_vfo_status(void) {
    fdc->disp_vfo_status();
}

void cmd_visualize_vfo(size_t track_n, size_t vfo_sel=99) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    track_stream = disk_img->get_track_data(track_n);
    vfo_base *vfo;
    if(vfo_sel == 99) {
        vfo_sel = g_vfo_type;
    }
    switch(vfo_sel) {
    default:
    case VFO_TYPE_SIMPLE:       vfo = new vfo_simple();         break;
    case VFO_TYPE_FIXED:        vfo = new vfo_fixed();          break;
    case VFO_TYPE_PID:          vfo = new vfo_pid();            break;
    case VFO_TYPE_PID2:         vfo = new vfo_pid2();           break;
    case VFO_TYPE_SIMPLE2:      vfo = new vfo_simple2();        break;
    case VFO_TYPE_PID3:         vfo = new vfo_pid3();           break;
    case VFO_TYPE_EXPERIMANTAL: vfo = new vfo_experimental();   break;
    }

    disk_image_base_properties props = disk_img->get_property();
    vfo->set_params(props.m_sampling_rate, props.m_data_bit_rate);
    vfo->set_gain_val(g_gain_l, g_gain_h);
    vfo->set_gain_mode(vfo_base::gain_state::low);
    track_stream.set_stream_pos(0);

    double scale = (150 * 0.5f) / (vfo->m_cell_size_ref);

    double dist = -1.f;
    size_t irregular_pulse_count = 0;
    size_t count = 0;
    bool irregular = false;

#ifdef _WIN32
    while(count<500000 && !track_stream.is_wraparound()) {
#else
    while(count<5000 && !track_stream.is_wraparound()) {
#endif
        irregular = false;

        std::string line = std::string(150, ' ');
        size_t win_st = (vfo->m_window_ofst) * scale;
        size_t win_en = (vfo->m_window_ofst+vfo->m_window_size) * scale;
        for(size_t x = win_st; x < win_en; x++) line[x] = '-';
        line[vfo->m_cell_size * scale] = '<';

        do {
            if (dist < vfo->m_cell_size) {
                // visualize
                if(dist<0.f)                          line[0]                      = '*';
                else if(dist * scale >= line.size())  line[vfo->m_cell_size*scale] = '*';
                else                                  line[dist * scale]           = 'P';
                if(dist < vfo->m_window_ofst || dist > vfo->m_window_ofst + vfo->m_window_size) {
                    irregular = true;
                    irregular_pulse_count++;
                }
                // run VFO
                dist = vfo->calc(dist);
                dist += static_cast<double>(track_stream.distance_to_next_pulse());
            }
        } while (dist < vfo->m_cell_size);
        dist -= vfo->m_cell_size;

        std::cout << std::setw(6) << count << " " << std::setw(6) << std::setprecision(4) << std::setfill(' ') << vfo->m_cell_size << " >" << line;
        if(irregular) {
            fdc_misc::color(6);
            std::cout << "\a*** IRREGULAR PULSE DETECTED ***";
            irregular_pulse_count++;
            fdc_misc::color(7);
        }
        std::cout << std::endl;
        count++;
#if _WIN32
        if(kbhit()) {
            getch();
            break;
        }
#endif
    }
    vfo->disp_vfo_status();
    fdc_misc::color(4);
    std::cout << irregular_pulse_count << " irregular pulse(s) detected." << std::endl;
    fdc_misc::color(7);

    delete vfo;
}


void cmd_select_vfo(size_t vfo_type) {
    if (vfo_type > VFO_TYPE_MAX && vfo_type != VFO_TYPE_EXPERIMANTAL) {
        std::cout << "wrong VFO type. (" VFO_TYPE_DESC_STR ")" << std::endl;
        return;
    }
    g_vfo_type = vfo_type;
    fdc->set_vfo_type(vfo_type);
}

void cmd_reset_vfo(void) {
    std::cout << "Reset VFO" << std::endl;
    fdc->soft_reset_vfo();
}

void cmd_histogram(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    track_stream = disk_img->get_track_data(track_n);
    track_stream.set_stream_pos(0);
    track_stream.clear_wraparound_flag();

    // count pulse distance frequency
    size_t max_val = 0;
    std::vector<size_t> dist_array = fdc_misc::get_frequent_distribution(track_stream);

    // display histogram
    std::cout << "#clocks  #pulses" << std::endl;
    fdc_misc::display_histogram(dist_array, true);

    // find distribution peaks
    std::vector<size_t> peaks = fdc_misc::find_peaks(dist_array);
    std::cout << std::endl;
    std::cout << "Peaks:" << std::endl;
    for(size_t i=0; i<3; i++) {
        std::cout << i+1 << " : " << peaks[i] << " [CLKs]" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Estimated bit cell width : " << peaks[0] / 2 << " [CLKs] (CLK=" << g_sampling_rate/1e6 << " MHz)"  << std::endl;
    std::cout << "Data bit rate : " << (g_sampling_rate / (peaks[0] / 2.f)) / 1000.f << " [Kbits/sec]" << std::endl;
}

void cmd_normalize_track(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }

    size_t track_n_s, track_n_e;
    if (track_n == 65535) { // target = all track
        track_n_s = 0;
        track_n_e = disk_img->get_number_of_tracks()-1;
    } else {
        track_n_s = track_n_e = track_n;
    }

    bit_array track_stream;
    bit_array normalized_track;
    disk_image_base_properties props = disk_img->get_property();

    double cell_size_ref = static_cast<double>(props.m_sampling_rate) / static_cast<double>(props.m_data_bit_rate);
    double cell_size = cell_size_ref;
    double new_cell_size;
    std::cout << "Standard cell size : " << cell_size_ref << std::endl;

    constexpr int window_size = 7;
    for(size_t trk_n = track_n_s; trk_n <= track_n_e; trk_n++) {
        track_stream = disk_img->get_track_data(trk_n);
        track_stream.set_stream_pos(0);
        normalized_track.clear_array();
        size_t current_pos = 0;
        std::cout << trk_n << ", ";
        std::vector<double> dist_array;
        while(!track_stream.is_wraparound()) {
            dist_array.push_back(static_cast<double>(track_stream.distance_to_next_pulse()));
        }
        // calculate moving average of cell_size (window size = window_size)
        for(int i = 0; i < dist_array.size(); i++) {
            double mov_avg = 0.f;
            for(int j = -window_size/2; j <= window_size/2; j++) {
                double tmp_cell_size, tmp_dist;
                if(i+j < 0) {
                    tmp_dist = dist_array[-i-j];                             // == [-(i+j)]; // mirroring on lower boundary
                } else if(i+j >= dist_array.size()) {
                    tmp_dist = dist_array[dist_array.size()*2 - i - j - 1];  // == [dist_array.size()-((i+j)-dist_array.size()+1)] // mirroring on upper boudary
                } else {
                    tmp_dist = dist_array[i+j];
                }
                tmp_cell_size = round(tmp_dist/cell_size);       // quantize (by current cell size)
                if(tmp_cell_size>=2 && tmp_cell_size <=4) {
                    mov_avg += tmp_dist / tmp_cell_size;         // add quantized cell size if it's normal size
                } else {
                    mov_avg += cell_size;                        // ignore outlier data (use current cell size instead)
                }
            }
            mov_avg /= static_cast<double>(window_size);         // moving average of cell_size

            double dist = dist_array[i];
            size_t quantized = round(dist/mov_avg);              // quantize

            //if(quantized==1) quantized = 2;
            //if(quantized>=5) quantized = 4; 
            current_pos += quantized * cell_size_ref;
            normalized_track.set(current_pos, 1);

            cell_size = mov_avg;
        }
        disk_img->set_track_data(trk_n, normalized_track);
    }
    std::cout << std::endl;
}

void cmd_help(void) {
    std::cout <<
    "*** Command list\n"
    "o  file_name      Open an image file. (.raw, .mfm, .hfe, .d77)\n"
    "w  file_name      Write an image file. (mfm, d77)\n"
    "scr file_name     Run a script file.\n"
    "rt trk            Read track\n"
    "vt trk [trk_e]    Validate track(s). Performs read ID and read sector for a track.\n"
    "                  If you specify 'trk_e', the command will perform track validation\n"
    "                  from 'trk' to 'trk_e'.\n"
    "tt trk [s_byte] [e_byte]  Trim track. Cut out the track data starting from 's_byte' to 'e_byte'. \n"
    "                  The 's_byte' and 'e_byte' can be specified in byte position in the track dump.\n"
    "                  If '*' is specified as 'trk', all tracks will be trimmed.\n"
    "nt trk            Normalize track. Normalize the pulse-to-pulse distance.\n"
    "                  If '*' is specified as 'trk', all tracks will be processed.\n"
    "ri trk [trk_e]    Read all sector IDs. Perform ID read from 'trk' to 'trk_e' if you specify trk_e.\n"
    "                  Otherwise, an ID read operation will be performed for a track.\n"
    "rs trk sct        Read sector\n"
    "rsp trk sct       Read sector (Visualize pulses for each byte)\n"
    "ef sus_ratio      Enable fluctuator (VFO stops operation at rate of sus_ratio (0.0-1.0))\n"
    "ef                Disable fluctuator\n"
    "gain gl gh        Set VFO gain (low=gl, high=gh)\n"
    "vfo               Display current VFO parameters\n"
    "vv trk [vfo_type] VFO visualizer. Read 5,000 pulses from the top of a track using specified type of VFO.\n"
    "                  Current VFO setting will be used if 'vfo_type' is omitted.\n"
    "sv vfo_type       Select VFO type.\n"
    "rv                (soft) reset VFO\n"
    "histogram trk     Display histogram of data pulse distance in a track.\n"
    "q                 Quit analyzer\n"
    "\n"
    "Note1: The number starting with '$' will be handled as hexadecimal value (e.g. $f7)\n"
    "Note2: VFO type = 0:vfo_fixed, 1:vfo_simple, 2:vfo_pid, 3:vfo_pid2, 4:vfo_simple2(default) 9=experimental\n"
    << std::endl;
}

void cmd_vfo_pid_tune(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    track_stream = disk_img->get_track_data(track_n);
    vfo_pid3 vfo;
    disk_image_base_properties props = disk_img->get_property();
    vfo.set_params(props.m_sampling_rate, props.m_data_bit_rate);
    vfo.set_gain_val(g_gain_l, g_gain_h);
    vfo.set_gain_mode(vfo_base::gain_state::low);
    //std::vector<double> param_table { 1.f/1.f, 1.f/2.f, 1.f/4.f, 1.f/8.f, 1.f/16.f, 1.f/32.f, 1.f/64.f, 1.f/128.f, 1.f/256.f, 1.f/512.f };
    std::vector<double> param_table0 { 1/512.f, 1/256.f, 1/128.f, 1/64.f, 1/32.f, 1/16.f, 1/8.f, 1/4.f };
    std::vector<double> param_table1 { 1/64.f, 1/32.f, 1/16.f, 1/10.f, 1/8.f, 1/6.f, 1/4.f, 1/3.f, 1/2.f, 1/1.5f };
    std::vector<double> param_table2 { 1/128.f, 1/96.f, 1/80.f, 1/64.f, 1/48.f, 1/32.f };
    std::ofstream log("log.txt", std::ios::out);
    std::vector<std::vector<double>> results;
    std::cout << "working";
    for(auto i0=param_table0.begin(); i0!=param_table0.end(); i0++) {
        for(auto i1=param_table0.begin(); i1!=param_table0.end(); i1++) {
            for(auto i2=param_table0.begin(); i2!=param_table0.end(); i2++) {
                track_stream.set_stream_pos(0);
                vfo.reset();
                vfo.m_phase_err_PC = *i0;
                vfo.m_phase_err_IC = *i1;
                vfo.m_phase_err_DC = *i2;
                double pulse_pos = 0.f;
                size_t err = 0, good=0;
                //vfo->disp_vfo_status();
                while(!track_stream.is_wraparound()) {
                    do {
                        if(pulse_pos < vfo.m_cell_size) {
                            if(pulse_pos >= vfo.m_window_ofst && pulse_pos < vfo.m_window_ofst + vfo.m_window_size) {
                                // Good pulse
                                good++;
                            } else {
                                // Irregular pulse
                                err++;
                            }
                            vfo.calc(pulse_pos);
                            double dist = track_stream.distance_to_next_pulse();
                            pulse_pos += dist;
                        }
                    } while (pulse_pos < vfo.m_cell_size);
                    if(pulse_pos >= vfo.m_cell_size) pulse_pos -= vfo.m_cell_size;
                }
                //std::cout << good << "," << err << ",";
                //std::cout << vfo.m_phase_err_PC << "," << vfo.m_phase_err_IC << "," << vfo.m_phase_err_DC << "," << std::endl;
                std::cout << ".";
                log << good << "," << err << ",";
                log << vfo.m_phase_err_PC << "," << vfo.m_phase_err_IC << "," << vfo.m_phase_err_DC << "," << std::endl;
                results.push_back(std::vector<double>{static_cast<double>(good), static_cast<double>(err), vfo.m_phase_err_PC, vfo.m_phase_err_DC, vfo.m_phase_err_IC});
            }
        }
    }
    std::cout << std::endl;
    std::sort(results.begin(), results.end(), [](auto left, auto right) { return left[1]<right[1]; });
    for(auto it = results.begin(); it != results.begin()+5; it++) {
        std::cout << (*it)[0] << " / " << (*it)[1] << " : ";
        std::cout << 1/(*it)[2] << " " << 1/(*it)[3] << " " << 1/ (*it)[4] << std::endl;
    }
}


// -------------------------------------------------------------------------

int main(int argc, char* argv[]) {
#ifdef _WIN32
    char tmp[256];
    _getcwd(tmp, 256);
    std::cout << "Current working directory : " << tmp << std::endl;
#endif
    cmd_help();
    fdc = new fdc_bitstream();
    fdc->disp_vfo_status();

    size_t cmd_count = 1;
    std::ifstream ifs;
    if (argc>1) {
        ifs.open(argv[1]);
    }

    std::string cmd_line;
    std::string prev_cmd;
    do {
        std::cout << "CMD(" << cmd_count << ") > ";
        if(ifs.is_open()) {
            std::getline(ifs, cmd_line);
            if(ifs.eof()) ifs.close();
            if(cmd_line.size()==0) { std::cout << std::endl; continue; }  // Ignore blank line
            if(cmd_line[0]=='#') { std::cout << std::endl; continue; }    // Ignore comment line
            if(cmd_line[0]==';') { std::cout << std::endl; continue; }    // Ignore comment line
            if(cmd_line[0]=='!') { 
                std::cout << std::endl << cmd_line << std::endl;   // Disp message
                continue; 
            }
            std::cout << cmd_line << std::endl;
        } else {
            std::getline(std::cin, cmd_line);
        }
        if(cmd_line.size() == 0) {
            cmd_line = prev_cmd;
            cmd_disp_vfo_status();
        }

        cmd_count++;
        std::vector<std::string> args;
        std::stringstream ss{ cmd_line };
        std::string tmp_item;
        while (std::getline(ss, tmp_item, ' '))
        {
            args.push_back(tmp_item);
        }

        if(args[0] == "o" && args.size()>=2) {
            cmd_open_image(args[1]);
        }
        else if(args[0] == "w" && args.size()>=2) {
            cmd_write_image(args[1]);
        }
        else if(args[0] == "scr" && args.size()>=2) {
            ifs.open(args[1]);
            if (!ifs.is_open()) { 
                std::cout << "File not found" << std::endl;
                ifs.close();
            }
        }
        else if(args[0] == "rt" && args.size()>=2) {
            cmd_read_track(fdc_misc::str2val(args[1]));
        } 
        else if(args[0] == "ri") {
            if(args.size()==2) cmd_read_id(fdc_misc::str2val(args[1]), -1);
            if(args.size()==3) cmd_read_id(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]));
        } 
        else if (args[0] == "vt"){
            if(args.size()==2) cmd_validate_track(fdc_misc::str2val(args[1]), -1);
            if(args.size()==3) cmd_validate_track(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]));
        }
        else if(args[0] == "rs" && args.size()>=3) {
            if(args[2][0] == '#') {
                args[2].erase(args[2].begin());  // remove #
                size_t sct_num = fdc_misc::str2val(args[2]) + 1000-1;
                cmd_read_sector(fdc_misc::str2val(args[1]), sct_num, false);
            } else {
                cmd_read_sector(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]), false);
            }
        }
        else if(args[0] == "rsp" && args.size()>=3) {
            if(args[2][0] == '#') {
                args[2].erase(args[2].begin());  // remove #
                size_t sct_num = fdc_misc::str2val(args[2]) + 1000-1;
                cmd_read_sector(fdc_misc::str2val(args[1]), sct_num, true);
            } else {
                cmd_read_sector(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]), true);
            }
        }
        else if(args[0] == "gain" && args.size()>=3) {
            cmd_set_gain(std::stod(args[1]), std::stod(args[2]));
        }
        else if(args[0] == "ef" && args.size()>=2) {
            cmd_enable_fluctuator(std::stod(args[1]));            
        }
        else if(args[0] == "df") {
            cmd_disable_fluctuator();
        }
        else if(args[0] == "sv" && args.size()>=2) {
            cmd_select_vfo(std::stoi(args[1]));
        }
        else if(args[0] == "vfo") {
            cmd_disp_vfo_status();
        }
        else if(args[0] == "vv" && args.size()>=2) {
            if(args.size()==2) cmd_visualize_vfo(std::stoi(args[1]));
            else               cmd_visualize_vfo(std::stoi(args[1]), std::stoi(args[2]));
        }
        else if(args[0] == "rv") {
            cmd_reset_vfo();
        }
        else if(args[0] == "histogram" && args.size()>=2) {
            cmd_histogram(fdc_misc::str2val(args[1]));            
        }
        else if(args[0] == "tt" && args.size()>=4) {
            if(args[1] == "*") {
                cmd_trim_track(65535, fdc_misc::str2val(args[2]), fdc_misc::str2val(args[3]));  // trim all tracks
            } else {
                cmd_trim_track(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]), fdc_misc::str2val(args[3]));
            }
        }
        else if(args[0] == "nt" && args.size()>=2) {
            if(args[1] == "*") {
                cmd_normalize_track(65535);  // trim all tracks
            } else {
                cmd_normalize_track(fdc_misc::str2val(args[1]));
            }
        }
        else if(args[0] == "vfo_tune" && args.size()>=2) {
            cmd_vfo_pid_tune(std::stoi(args[1]));
        }
        else if(args[0] == "h" || args[0]=="help" || args[0]=="?") {
            cmd_help();
        }
        prev_cmd = cmd_line;
    } while (cmd_line != "q");

    if(fdc != nullptr) delete fdc;
    if(disk_img != nullptr) delete disk_img;
}
