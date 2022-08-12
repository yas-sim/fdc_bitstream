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

#include "fdc_defs.h"
#include "bit_array.h"

class DLL_EXPORT disk_image_exception {
private:
    int m_error_code;
    std::string m_message;
public:
    disk_image_exception(int error_code, std::string message) : m_error_code(error_code), m_message(message) {};
    int get_error_code(void) { return m_error_code; };
    std::string what(void) { return m_message; };
};

class DLL_EXPORT disk_image {
private:
protected:
    bool        m_track_data_is_set;            /** true=track data is set and ready */
    size_t      m_max_track_number;
    uint64_t    m_sampling_frequency;           /** Disk image sampling frequency [Hz] */
    uint64_t    m_fdc_data_rate;                /** FDC bit data rate [bit/sec] */

    std::vector<bit_array>  m_track_data;

public:

    disk_image();
    void clear_track_data(void);                            // Clear track buffer
    void create_empty_track_data(size_t num_tracks);        // Create empty track buffers with length of 0.

    std::ifstream open_binary_file(std::string file_name);
    virtual void read(std::string file_name) = 0;

    virtual bit_array get_track_data(size_t track_number);

    size_t media_max_track_number(media_type mtype);

    inline bool is_ready(void) { return m_track_data_is_set; }
};
