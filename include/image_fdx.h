#pragma once

#include <string.h>

#include "bit_array.h"
#include "image_base.h"

// FDX header
// IMPORTANT: Some of the data in this header has data type boundary misalignment.
//            You need to specify #pragma pack(1). Otherwise, the compiler will insert 
//            some padding between elements and that will mess up the offset calculation. 
#pragma pack(1)
typedef struct {
    uint8_t     signature[3];       // +0
    uint8_t     revision;           // +3
    uint8_t     disk_name[64-4];    // +4
    uint8_t     pad2[68-64];        // +64
    uint32_t    type;               // +68  0:2D, 1:2DD, 2:2HD, 9:RAW
    uint32_t    cylinders;          // +72
    uint32_t    heads;              // +76
    uint32_t    rate;               // +80  500 or 1000
    uint32_t    rpm;                // +84
    uint8_t     writeprotect;       // +88
    uint8_t     pad3[92-89];
    uint32_t    option;             // +92
    uint32_t    unused;             // +96
    uint32_t    tracksize;          // +100
    uint8_t     reserve[256-104];   // +104  (Total header size is 256bytes)
} fdx_header;


// Track data
typedef struct {
    uint32_t    cylinder;           // +0
    uint32_t    head;               // +4
    uint32_t    index;              // +8  (bit count)
    uint32_t    length;             // +12 (bit count)
} fdx_track_header;

#pragma pack()

// Track data * 84
//   ofst(byte) = track_table[track#].offset
//   size(byte) = track_table[track#].length_bit/8 + (track_table[track#].length%8)?1:0)

class disk_image_fdx : public disk_image {
private:
    bit_array simple_raw_to_mfm(bit_array &raw) const;
    bit_array simple_mfm_to_raw(bit_array &mfm) const;
public:
    disk_image_fdx(void) : disk_image() {};

    void read(const std::string file_name) override;
    void write(const std::string file_name) override;

    disk_image_fdx& operator=(disk_image &image)
    {
        m_base_prop = image.get_property();
        m_track_data = image.get_track_data_all();
        return *this;
    }

};
