#ifndef UTIL_IS_INCLUDED
#define UTIL_IS_INCLUDED
/* { */

#include <string>

#include "disk_images.h"
#include "fdc_bitstream.h"

bool CompareDisk(const disk_image &diskA,const disk_image &diskB,std::string diskALabel,std::string diskBLabel,bool ignoreCRCErrorSector);


/* } */
#endif
