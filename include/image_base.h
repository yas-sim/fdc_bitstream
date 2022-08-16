#pragma once

#ifdef _WIN32
#ifdef DLL_BODY
#define DLL_EXPORT  __declspec(dllexport)
#else
#define DLL_EXPORT  __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif

#include <cstdint>
#include <cassert>

#include "fdc_defs.h"
#include "bit_array.h"

class DLL_EXPORT disk_image_exception {
private:
    int m_error_code;
    std::string m_message;
public:
    disk_image_exception(const int error_code, const std::string message) : m_error_code(error_code), m_message(message) {};
    int get_error_code(void) { return m_error_code; };
    std::string what(void) { return m_message; };
};

class DLL_EXPORT disk_image_base_properties {
public:
    disk_image_base_properties() : m_max_track_number(0), m_spindle_time_ns(0), m_sampling_rate(0), m_data_bit_rate(0) {};

    size_t      m_max_track_number;
    /** Spindle rotation time [ns] */
    size_t      m_spindle_time_ns;
    /** Disk image sampling frequency [Hz] */
    size_t      m_sampling_rate;
    /** FDC bit data rate [bit/sec] */
    size_t      m_data_bit_rate;
};

class DLL_EXPORT disk_image {
private:
protected:
    bool        m_track_data_is_set;            /** true=track data is set and ready */
    disk_image_base_properties m_base_prop;
    std::vector<bit_array>  m_track_data;

    /** align a number with specified boundary */
    inline size_t align(size_t pos, size_t grain_size = 0x400) { return ((pos / grain_size) + ((pos % grain_size) ? 1 : 0)) * grain_size; }
public:

    disk_image();

    void clear_track_data(void);                            // Clear track buffer
    void create_empty_track_data(size_t num_tracks);        // Create empty track buffers with length of 0.

    std::ifstream open_binary_file(const std::string file_name);
    std::ifstream open_text_file(const std::string file_name);
    virtual void read(const std::string file_name) = 0;
    virtual void write(const std::string file_name) = 0;

    bit_array get_track_data(const size_t track_number);
    void set_track_data(const size_t track_number, const bit_array track_data);

    size_t media_max_track_number(const media_type mtype);

    inline bool is_ready(void) { return m_track_data_is_set; };

    disk_image_base_properties get_property(void) { return m_base_prop; }
    void set_property(const disk_image_base_properties prop) { m_base_prop = prop; }
};
