#pragma once

#include "dll_export.h"

#include <cassert>

#ifdef _WIN32
#ifdef DLL_BODY
#define DLL_EXPORT  __declspec(dllexport)
#else
#define DLL_EXPORT  __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif

#include <string>
#include <sstream>
#include <vector>

#include "image_base.h"

class DLL_EXPORT disk_image_raw : public disk_image {
private:
public:
    disk_image_raw() : disk_image() {};

    void read(const std::string raw_file_name) override;
    void write(const std::string raw_file_name) override { assert(false); }

    disk_image_raw& operator=(disk_image &image)
    {
        m_base_prop = image.get_property();
        m_track_data = image.get_track_data_all();
        return *this;
    }

};
