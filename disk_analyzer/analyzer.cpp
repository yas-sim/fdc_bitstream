#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#endif

#include "fdc_bitstream.h"

#include "disk_images.h"

disk_image *disk_img = nullptr;
fdc_bitstream *fdc = nullptr;

double g_gain_l = 1.f, g_gain_h = 2.f;

void dump_buf(uint8_t* ptr, size_t size, bool line_feed = true) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    for (size_t i = 0; i < size; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[i]) << " ";
        if (i % 64 == 63) {
            std::cout << std::endl;
        }
    }
    if (line_feed) std::cout << std::endl;
    std::cout.flags(flags_saved);
}

void bit_dump(const uint64_t data, size_t bit_width, size_t spacing = 0, bool line_feed = true) {
    size_t count = 0;
    for (uint64_t bit_pos = 1 << (bit_width - 1); bit_pos != 0; bit_pos >>= 1) {
        std::cout << ((data & bit_pos) ? 1 : 0);
        count++;
        if (spacing > 0) {
            if (count % spacing == 0) {
                std::cout << " ";
            }
        }
    }
    if (line_feed == true) {
        std::cout << std::endl;
    }
}

void bit_dump(bit_array &data, size_t bit_width = 0, size_t spacing = 0, bool line_feed = true) {
    size_t count = 0;
    size_t length = (bit_width == 0) ? data.get_length() : bit_width;
    for (uint64_t i = 0; i < length; length++) {
        std::cout << (data.get(i) ? 1 : 0);
        count++;
        if (spacing > 0) {
            if (count % spacing == 0) {
                std::cout << " ";
            }
        }
    }
    if (line_feed == true) {
        std::cout << std::endl;
    }
}

void display_id_list(std::vector<fdc_bitstream::id_field> id_fields) {
    std::ios::fmtflags flags_saved = std::cout.flags();
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (int i = 0; i < id_fields.size(); i++) {
        std::cout << std::dec << std::setw(2) << std::setfill(' ') << i << " ";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].C) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].H) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].R) << " ";
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(id_fields[i].N) << " ";
        std::cout << std::setw(4) << std::setfill('0') << static_cast<int>(id_fields[i].crc_val) << " ";
        std::cout << (id_fields[i].crc_sts ? "ERR" : "OK ") << std::endl;
    }
    std::cout.flags(flags_saved);
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
        std::cout << "Unsupported file extension." << std::endl;
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
    dump_buf(read_data.data(), read_data.size());
}

void cmd_read_id(size_t track_n) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    std::vector<fdc_bitstream::id_field> read_data;
    track_stream = disk_img->get_track_data(track_n);
    fdc->set_track_data(track_stream);
    read_data = fdc->read_all_idam();
    display_id_list(read_data);
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
    bit_array track_stream;
    fdc_bitstream::sector_data read_data;
    track_stream = disk_img->get_track_data(cyl);
    fdc->set_track_data(track_stream);
    read_data = fdc->read_sector(cyl, hed, rcd);
    dump_buf(read_data.data.data(), read_data.data.size());
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

void cmd_visualize_vfo(size_t track_n, size_t vfo_sel=0) {
    if(is_image_ready()==false) {
        std::cout << "Disk image is not ready." << std::endl;
        return;
    }
    bit_array track_stream;
    track_stream = disk_img->get_track_data(track_n);
    vfo_base *vfo;
    switch(vfo_sel) {
    default:
    case 0:
        vfo = new vfo_simple();
        break;
    case 1:
        vfo = new vfo_fixed();
        break;
    case 2:
        vfo = new vfo_pid();
        break;
    case 3:
        vfo = new vfo_pid2();
        break;
    }

    disk_image_base_properties props = disk_img->get_property();
    vfo->set_params(props.m_sampling_rate, props.m_data_bit_rate);
    vfo->set_gain_val(g_gain_l, g_gain_h);
    vfo->set_gain_mode(vfo_base::gain_state::low);
    track_stream.set_stream_pos(0);

    double dist = 0.f;
    for(size_t i=0; i<5000; i++) {
        dist += static_cast<double>(track_stream.distance_to_next_bit1());
        //dist -= std::floor(dist / vfo->m_cell_size) * vfo->m_cell_size;
        while(dist > vfo->m_cell_size) {
            dist -= vfo->m_cell_size;
        }

        // visualize
        std::string line = std::string(80, ' ');
        for(size_t x = vfo->m_window_ofst; x<vfo->m_window_ofst+vfo->m_window_size; x++) {
            line[x] = '_';
        }
        line[vfo->m_cell_size+1] = '<';
        if(dist>=0.f) {
            line[dist] = 'P';
        } else {
            line[0] = '*';
        }
        std::cout << line << std::endl;

        // run VFO
        dist = vfo->calc(dist);
    }
    vfo->disp_vfo_status();

    delete vfo;
}

void cmd_help(void) {
    std::cout <<
    "*** Command list\n"
    "o  file_name    Open an image file.\n"
    "rt trk          Read track\n"
    "ri trk          Read all sector IDs\n"
    "rs trk sid sct  Read sector\n"
    "ef sus_radio    Enable fluctuator (VFO stops operation at rate of sus_ratio (0.0-1.0))\n"
    "ef              Disable fluctuator\n"
    "gain gl gh      Set VFO gain (low=gl, high=gh)\n"
    "vfo             Display current VFO parameters"
    "vv trk vfo_type VFO visualizer. Read 5,000 pulses from the top of a track using specified type of VFO.\n"
    "                VFO type = 0:vfo_fixed, 1:vfo_simple, 2:vfo_pid, 3:vfo_pid2\n"
    "q               Quit analyzer\n"
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

        if(args[0] == "o") {
            cmd_open_image(args[1]);
        } 
        else if(args[0] == "rt" && args.size()>=2) {
            cmd_read_track(std::stoi(args[1]));
        } 
        else if(args[0] == "ri") {
            cmd_read_id(std::stoi(args[1]) && args.size()>=2);
        } 
        else if(args[0] == "rs") {
            cmd_read_sector(std::stoi(args[1]), std::stoi(args[2]), std::stoi(args[3]) && args.size()>=4);
        }
        else if(args[0] == "gain") {
            cmd_set_gain(std::stod(args[1]), std::stod(args[2]) && args.size()>=3);
        }
        else if(args[0] == "ef") {
            cmd_enable_fluctuator(std::stod(args[1]) && args.size()>=2);            
        }
        else if(args[0] == "df") {
            cmd_disable_fluctuator();
        }
        else if(args[0] == "vfo") {
            cmd_disp_vfo_status();
        }
        else if(args[0] == "vv" && args.size()>=2) {
            if(args.size()==2) cmd_visualize_vfo(std::stoi(args[1]));
                               cmd_visualize_vfo(std::stoi(args[1]), std::stoi(args[2]));
        }
        else if(args[0] == "h") {
            cmd_help();
        }
        prev_cmd = cmd_line;
    } while (cmd_line != "q");

    if(fdc != nullptr) delete fdc;
    if(disk_img != nullptr) delete disk_img;
}
