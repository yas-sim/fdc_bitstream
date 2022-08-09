#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "image_base.h"

class disk_image_raw : public disk_image {
private:
public:
    disk_image_raw() : disk_image() {};

    void read(std::string raw_file_name);
};
