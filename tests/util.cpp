#include <string>

#include "disk_images.h"
#include "fdc_bitstream.h"
#include "util.h"


bool CompareDisk(const disk_image &diskA,const disk_image &diskB,std::string diskALabel,std::string diskBLabel,bool ignoreCRCErrorSector)
{
	std::cout << "Compare " << diskALabel << " and " << diskBLabel << std::endl;

	if(diskA.get_number_of_tracks()!=diskB.get_number_of_tracks())
	{
		std::cout << "Track count do not match." << std::endl;
		std::cout << "    " << diskA.get_number_of_tracks() << std::endl;
		std::cout << "    " << diskB.get_number_of_tracks() << std::endl;
		return false;
	}
	for(unsigned int trk=0; trk<diskA.get_number_of_tracks(); ++trk)
	{
		fdc_bitstream drive[2];

		drive[0].set_track_data(diskA.get_track_data(trk));
		drive[0].set_pos(0);
		drive[0].set_vfo_type(VFO_TYPE_DEFAULT);
		drive[0].set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);
		auto idDisk0=drive[0].read_all_idam();

		drive[1].set_track_data(diskB.get_track_data(trk));
		drive[1].set_pos(0);
		drive[1].set_vfo_type(VFO_TYPE_DEFAULT);
		drive[1].set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);
		auto idDisk1=drive[0].read_all_idam();

		if(idDisk0.size()!=idDisk1.size())
		{
			std::cout << "Sector count in track " << trk << " do not match." << std::endl;
			std::cout << "    " << idDisk0.size() << std::endl;
			std::cout << "    " << idDisk1.size() << std::endl;
			return false;
		}

		for(auto id0 : idDisk0)
		{
			bool foundMatch=false;
			for(auto id1 : idDisk1)
			{
				if(id0.C==id1.C &&
				   id0.H==id1.H &&
				   id0.R==id1.R &&
				   id0.N==id1.N)
				{
					foundMatch=true;
					break;
				}
			}
			if(true!=foundMatch)
			{
				std::cout << "Sector (C,H,R,N)=(" << id0.C << "," << id0.H << "," << id0.R << "," << id0.N << ")" << std::endl;
				std::cout << "in " << diskALabel << " was not found in " << diskBLabel << std::endl;
				return false;
			}
		}

		for(auto id : idDisk0)
		{
			auto sector0=drive[0].read_sector(id.C,id.R);
			auto sector1=drive[1].read_sector(id.C,id.R);

			if(true==sector0.record_not_found || true==sector1.record_not_found)
			{
				std::cout << "Record Not Found Error." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << id.C << "," << id.H << "," << id.R << "," << id.N << ")" << std::endl;
				return false;
			}

			if(sector0.crc_sts!=sector1.crc_sts)
			{
				std::cout << "CRC Error Status Do Not Match." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << id.C << "," << id.H << "," << id.R << "," << id.N << ")" << std::endl;
				return false;
			}

			if(sector0.data.size()!=sector1.data.size())
			{
				std::cout << "Sector sizes do not match." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << id.C << "," << id.H << "," << id.R << "," << id.N << ")" << std::endl;
				return false;
			}

			if(true==ignoreCRCErrorSector && true==sector0.crc_sts)
			{
				continue;
			}

			for(int i=0; i<sector0.data.size(); ++i)
			{
				if(sector0.data[i]!=sector1.data[i])
				{
					std::cout << "Sector contents do not match." << std::endl;
					std::cout << "Sector (C,H,R,N)=(" << id.C << "," << id.H << "," << id.R << "," << id.N << ")" << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

