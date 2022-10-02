/*
pauline2raw
Pauline .hxcstream samples to fdc_bitstream .RAW file converter.
By Soji Yamakawa (CaptainYS, http://www.ysflight.com)
*/

#include <iostream>
#include "pauline2raw.h"

int main(int ac,char *av[])
{
	PaulineToRaw exporter;
	if(true==exporter.RecognizeCommandParameter(ac,av) &&
	   true==exporter.ExportRaw())
	{
		std::cout << "Completed." << std::endl;
		return 0;
	}
	std::cout << "There were errors." << std::endl;
	return 1;
}
