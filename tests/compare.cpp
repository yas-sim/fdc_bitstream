#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "util.h"



std::vector <std::string> Parse(std::string line)
{
	std::vector <std::string> words;

	int state=0;
	for(auto c : line)
	{
		if(0==state)
		{
			if(' '!=c && '\t'!=c && '\n'!=c)
			{
				state=1;
				words.push_back("");
				words.back().push_back(c);
			}
		}
		else // if(0!=state)
		{
			if(' '==c || '\t'==c || '\n'==c)
			{
				state=0;
			}
			else
			{
				words.back().push_back(c);
			}
		}
	}
	return words;
}

int main(int ac,char *av[])
{
	std::string fName[2];
	CompareDiskOption opt;

	if(3<=ac)
	{
		fName[0]=av[1];
		fName[1]=av[2];
	}
	else
	{
		std::cout << "Two file names must be given." << std::endl;
		return 1;
	}

	if(4<=ac)
	{
		std::vector <std::string> argv;
		std::ifstream ifp(av[3]);
		if(true==ifp.is_open())
		{
			while(true!=ifp.eof())
			{
				std::string str;
				std::getline(ifp,str);
				auto words=Parse(str);
				argv.insert(argv.end(),words.begin(),words.end());
			}
		}

		for(int i=0; i<argv.size(); ++i)
		{
			if("-ignore_crc"==argv[i])
			{
				opt.ignoreCRCErrorSector=true;
			}
			else if("-exclude_track"==argv[i] && 1+i<argv.size())
			{
				opt.excludeTracks.insert(atoi(argv[i+1].c_str()));
				++i;
			}
			else if("-track_limit"==argv[i] && i+1<argv.size())
			{
				opt.trackLimit=atoi(argv[i+1].c_str());
				++i;
			}
			else
			{
				std::cout << "Undefined option: [" << argv[i] << "]" << std::endl;
				return 1;
			}
		}
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
