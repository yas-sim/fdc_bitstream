#pragma once

#include <string.h>

#include "bit_array.h"
#include "image_base.h"
#include "fdc_vfo_def.h"

class disk_image_d77 : public disk_image {
private:
    size_t m_vfo_type;
public:
    disk_image_d77(void) : disk_image(), m_vfo_type(VFO_TYPE_DEFAULT) {};

    void read(const std::string file_name) override;
    void write(const std::string file_name) override;
    void set_vfo_type(const size_t vfo_type);
};
