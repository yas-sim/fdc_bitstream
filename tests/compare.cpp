#include <stdlib.h>
#include <iostream>
#include <string>
#include <memory>
#include "util.h"


int main(int ac,char *av[])
{
	int fNameCount=0;
	std::string fName[2];
	CompareDiskOption opt;

	for(int i=1; i<ac; ++i)
	{
		std::string avi=av[i];
		if("-ignore_crc"==avi)
		{
			opt.ignoreCRCErrorSector=true;
		}
		else if("-exclude_track"==avi && 1+i<ac)
		{
			opt.excludeTracks.insert(atoi(av[i+1]));
			++i;
		}
		else if("-track_limit"==avi && i+1<ac)
		{
			opt.trackLimit=atoi(av[i+1]);
			++i;
		}
		else if(fNameCount<2)
		{
			fName[fNameCount++]=av[i];
		}
		else
		{
			std::cout << "Undefined option: [" << av[i] << "]" << std::endl;
			return 1;
		}
	}

	if(2!=fNameCount)
	{
		std::cout << "Two file names must be given." << std::endl;
		return 1;
	}

	std::unique_ptr <disk_image> img[2]={nullptr,nullptr};

	for(int i=0; i<2; ++i)
	{
		size_t lastDot=0;
		for(size_t j=0; j<fName[i].size(); ++j)
		{
			if(fName[i][j]=='.')
			{
				lastDot=j;
			}
		}
		std::string ext=fName[i].substr(lastDot);
		for(auto &c : ext)
		{
			if('a'<=c && c<='z')
			{
				c=c+'A'-'a';
			}
		}
		if(".RAW"==ext)
		{
			img[i]=std::unique_ptr<disk_image>(new disk_image_raw);
		}
		if(".MFM"==ext)
		{
			img[i]=std::unique_ptr<disk_image>(new disk_image_mfm);
		}
		if(".FDX"==ext)
		{
			img[i]=std::unique_ptr<disk_image>(new disk_image_fdx);
		}
		if(".D77"==ext)
		{
			img[i]=std::unique_ptr<disk_image>(new disk_image_d77);
		}
		if(".HFE"==ext)
		{
			img[i]=std::unique_ptr<disk_image>(new disk_image_hfe);
		}

		if(nullptr==img[i])
		{
			std::cout << "Unsupported file type." << std::endl;
			return 1;
		}

		img[i]->read(fName[i]);
		if(true!=img[i]->is_ready())
		{
			std::cout << "Failed to read image." << std::endl;
			return 1;
		}
	}

	std::cout << "Images Loaded." << std::endl;
	if(true!=CompareDisk(*img[0],*img[1],fName[0],fName[1],opt))
	{
		std::cout << "Comparison Failed." << std::endl;
		return 1;
	}

	std::cout << "Pass!" << std::endl;

	return 0;
}
