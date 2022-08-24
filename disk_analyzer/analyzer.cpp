#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#endif

#include "fdc_bitstream.h"

#include "disk_images.h"
#include "fdc_vfo_def.h"
#include "fdc_misc.h"

disk_image *disk_img = nullptr;
fdc_bitstream *fdc = nullptr;

double g_gain_l = 1.f, g_gain_h = 2.f;
size_t g_vfo_type = VFO_TYPE_DEFAULT;

size_t g_sampling_rate = 0;
size_t g_data_bit_rate = 0;
size_t g_spindle_time_ns = 0;
size_t g_max_track_number = 0;

/*
#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
#define COMMON_LVB_LEADING_BYTE    0x0100 // Leading Byte of DBCS
#define COMMON_LVB_TRAILING_BYTE   0x0200 // Trailing Byte of DBCS
#define COMMON_LVB_GRID_HORIZONTAL 0x0400 // DBCS: Grid attribute: top horizontal.
#define COMMON_LVB_GRID_LVERTICAL  0x0800 // DBCS: Grid attribute: left vertical.
#define COMMON_LVB_GRID_RVERTICAL  0x1000 // DBCS: Grid attribute: right vertical.
#define COMMON_LVB_REVERSE_VIDEO   0x4000 // DBCS: Reverse fore/back ground attribute.
#define COMMON_LVB_UNDERSCORE      0x8000 // DBCS: Underscore.*/

#ifdef _WIN32
#include <windows.h>
void color(size_t col) {
    if(col>7) col=7;
    size_t flags = FOREGROUND_INTENSITY;
    if (col & 0b0001) flags |= FOREGROUND_INTENSITY | FOREGROUND_BLUE;
    if (col & 0b0010) flags |= FOREGROUND_INTENSITY | FOREGROUND_RED;
    if (col & 0b0100) flags |= FOREGROUND_INTENSITY | FOREGROUND_GREEN;
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console_handle, flags);
}
#else
void color(size_t col) {
    const size_t col_tbl[8] { 30, 34, 31, 35, 32, 36, 33, 37 };
    if(col>7) col=7;
    std::stringstream ss;
    ss << "\e[" << std::dec << col_tbl[col] << "m";
    std::cout << ss.str();
}
#endif

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
    std::string ext = get_file_extension(file_name);
    if(check_extension(ext) == false) {
        color(2);
        std::cout << "Unsupported file extension." << std::endl;
        color(7);
        return;
    }
    disk_img = create_object_by_ext(ext);
    disk_img->read(file_name);

    disk_image_base_properties props = disk_img->get_property();
    std::cout << "Max Track num : " << props.m_max_track_number << std::endl;
    std::cout << "Spindle speed : " << props.m_spindle_time_ns / 1e6 << " [ms/rotation]" << std::endl;
    std::cout << "Sampling rate : " << props.m_sampling_rate / 1e6 << " [Msamples/sec]" << std::endl;
    std::cout << "Data bit rate : " << props.m_data_bit_rate / 1e3 << " [Kbit/sec]" << std::endl;

    fdc->set_fdc_params(props.m_sampling_rate, props.m_data_bit_rate);
    fdc->disp_vfo_status();

    g_sampling_rate = props.m_sampling_rate;
    g_data_bit_rate = props.m_sampling_rate;
    g_spindle_time_ns = props.m_spindle_time_ns;
    g_max_track_number = props.m_max_track_number;
}

void cmd_read_track(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    std::vector<uint8_t> read_data;
    track_stream = disk_img->get_track_data(track_n);
    fdc->set_track_data(track_stream);
    read_data = fdc->read_track();
    fdc_misc::dump_buf(read_data.data(), read_data.size());
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
        fdc_misc::display_id_list(read_data);
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
        std::vector<fdc_bitstream::sector_data> sect_data;
        for(auto it = id_data.begin(); it != id_data.end(); ++it) {
            fdc_bitstream::id_field id = *it;
            sect_data.push_back(fdc->read_sector(id.C, id.H, id.R));
        }
        //           "  1: 00 00 01 01 fa0c ID-CRC_OK   256 DAM  DT-CRC OK  RNF_OK  IDAM_POS=    5208 DAM_POS=   10923"
        std::cout << "  #: CC HH RR NN --- ID CRC ---  SIZE" << std::endl;
        for(size_t i = 0; i < id_data.size(); i++) {
            std::cout << std::setw(3) << std::setfill(' ') << i+1 << ": ";
            fdc_misc::display_id(id_data[i]);
            fdc_misc::display_sector_data(sect_data[i]);
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

void cmd_read_sector(size_t cyl, size_t hed, size_t rcd) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    std::cout << "Sector read : " << cyl << ", " << hed << ", " << rcd << std::endl;
    bit_array track_stream;
    fdc_bitstream::sector_data read_data;
    track_stream = disk_img->get_track_data(cyl * 2 + hed);
    fdc->set_track_data(track_stream);
    read_data = fdc->read_sector(cyl, hed, rcd);
    fdc_misc::dump_buf(read_data.data.data(), read_data.data.size());
    std::cout << "CRC DAM  RNF --ID_POS-- --DAM_POS- SIZE" << std::endl;
    std::cout << (read_data.crc_sts ? "ERR " : "OK  ") << (read_data.dam_type ? "DDAM " : "DAM  ") << (read_data.record_not_found ? "ERR " : "OK  ");
    std::cout << std::setw(10) << read_data.id_pos << " " << std::setw(10) << read_data.data_pos << " " << read_data.data.size() << std::endl;
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
    case VFO_TYPE_SIMPLE:       vfo = new vfo_simple();       break;
    case VFO_TYPE_FIXED:        vfo = new vfo_fixed();        break;
    case VFO_TYPE_PID:          vfo = new vfo_pid();          break;
    case VFO_TYPE_PID2:         vfo = new vfo_pid2();         break;
    case VFO_TYPE_EXPERIMANTAL: vfo = new vfo_experimental(); break;
    }

    disk_image_base_properties props = disk_img->get_property();
    vfo->set_params(props.m_sampling_rate, props.m_data_bit_rate);
    vfo->set_gain_val(g_gain_l, g_gain_h);
    vfo->set_gain_mode(vfo_base::gain_state::low);
    track_stream.set_stream_pos(0);

    double scale = 50.f / (vfo->m_cell_size_ref);

    double dist = 0.f;
    size_t irregular_pulse_count = 0;
    for(size_t i=0; i<5000; i++) {
        dist += static_cast<double>(track_stream.distance_to_next_pulse());
        dist -= std::floor(dist / vfo->m_cell_size) * vfo->m_cell_size;
        //while(dist > vfo->m_cell_size) {
        //    dist -= vfo->m_cell_size;
        //}

        // visualize
        std::string line = std::string(50 * 1.3, ' ');
        size_t win_st = (vfo->m_window_ofst) * scale;
        size_t win_en = (vfo->m_window_ofst+vfo->m_window_size) * scale;
        for(size_t x = win_st; x < win_en; x++) {
            line[x] = '-';
        }
        line[vfo->m_cell_size * scale] = '<';
        if(dist>=0.f) {
            line[dist * scale] = 'P';
        } else {
            line[0] = '*';
        }
        std::cout << std::setw(6) << i << " >" << line;
        if(dist < vfo->m_window_ofst || dist > vfo->m_window_ofst + vfo->m_window_size) {
            color(6);
            std::cout << "*** IRREGULAR PULSE DETECTED ***";
            irregular_pulse_count++;
            color(7);
        }
        std::cout << std::endl;

        // run VFO
        dist = vfo->calc(dist);
    }
    vfo->disp_vfo_status();
    color(4);
    std::cout << irregular_pulse_count << " irregular pulse(s) detected." << std::endl;
    color(7);

    delete vfo;
}


void cmd_select_vfo(size_t vfo_type) {
    if (vfo_type>3 && vfo_type !=9) {
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
    fdc_misc::display_histogram(dist_array);

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

void cmd_help(void) {
    std::cout <<
    "*** Command list\n"
    "o  file_name      Open an image file. (.raw, .mfm, .hfe, .d77)\n"
    "rt trk            Read track\n"
    "vt trk [trk_e]    Validate track(s). Performs read ID and read sector for a track.\n"
    "                  If you specify 'trk_e', the command will perform track validation\n"
    "                  from 'trk' to 'trk_e'.\n"
    "ri trk [trk_e]    Read all sector IDs. Perform ID read from 'trk' to 'trk_e' if you specify trk_e.\n"
    "                  Otherwise, an ID read operation will be performed for a track.\n"
    "rs trk sid sct    Read sector\n"
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
    "Note2: VFO type = 0:vfo_fixed, 1:vfo_simple, 2:vfo_pid, 3:vfo_pid2, 9=experimental\n"
    << std::endl;
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
    std::istream &in_stream = ifs.is_open()? ifs : std::cin;

    std::string cmd_line;
    std::string prev_cmd;
    do {
        std::cout << "CMD(" << cmd_count << ") > ";
        std::getline(in_stream, cmd_line);
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
        else if(args[0] == "rs" && args.size()>=4) {
            cmd_read_sector(fdc_misc::str2val(args[1]), fdc_misc::str2val(args[2]), fdc_misc::str2val(args[3]));
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
        else if(args[0] == "h") {
            cmd_help();
        }
        prev_cmd = cmd_line;
    } while (cmd_line != "q");

    if(fdc != nullptr) delete fdc;
    if(disk_img != nullptr) delete disk_img;
}
