#ifndef UTIL_IS_INCLUDED
#define UTIL_IS_INCLUDED
/* { */

#include <string>
#include <set>

#include "disk_images.h"
#include "fdc_bitstream.h"

class CompareDiskOption
{
public:
	bool ignoreCRCErrorSector=false;
	unsigned int trackLimit=0x7fffffff;
	std::set <int> excludeTracks;
};

bool CompareDisk(const disk_image &diskA,const disk_image &diskB,std::string diskALabel,std::string diskBLabel,const CompareDiskOption &opt);


/* } */
#endif
