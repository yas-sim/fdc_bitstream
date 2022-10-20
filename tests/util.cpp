#include <algorithm>
#include <string>
#include <set>

#include "disk_images.h"
#include "fdc_bitstream.h"
#include "util.h"


bool CompareDisk(const disk_image &diskA,const disk_image &diskB,std::string diskALabel,std::string diskBLabel,const CompareDiskOption &opt)
{
	std::cout << "Compare " << diskALabel << " and " << diskBLabel << std::endl;

	auto numTracksA=std::min<unsigned int>(diskA.get_number_of_tracks(),opt.trackLimit);
	auto numTracksB=std::min<unsigned int>(diskB.get_number_of_tracks(),opt.trackLimit);

	if(numTracksA!=numTracksB)
	{
		std::cout << "Track count do not match." << std::endl;
		std::cout << "    " << numTracksA << std::endl;
		std::cout << "    " << numTracksB << std::endl;
		return false;
	}
	for(unsigned int trk=0; trk<numTracksA && trk<numTracksB; ++trk)
	{
		if(opt.excludeTracks.end()!=opt.excludeTracks.find(trk))
		{
			continue;
		}

		fdc_bitstream drive[2];

		auto propsA = diskA.get_property();
		auto propsB = diskB.get_property();

		drive[0].set_track_data(diskA.get_track_data(trk));
		drive[0].set_pos(0);
		drive[0].set_vfo_type(VFO_TYPE_DEFAULT);
		drive[0].set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);
		drive[0].set_fdc_params(propsA.m_sampling_rate, propsA.m_data_bit_rate);
		auto idDisk0=drive[0].read_all_idam();

		drive[1].set_track_data(diskB.get_track_data(trk));
		drive[1].set_pos(0);
		drive[1].set_vfo_type(VFO_TYPE_DEFAULT);
		drive[1].set_vfo_gain_val(VFO_GAIN_L_DEFAULT,VFO_GAIN_H_DEFAULT);
		drive[1].set_fdc_params(propsB.m_sampling_rate, propsB.m_data_bit_rate);
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

			int C=id.C;
			int H=id.H;
			int R=id.R;
			int N=id.N;

			if(true==sector0.record_not_found || true==sector1.record_not_found)
			{
				std::cout << "Record Not Found Error." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << C << "," << H << "," << R << "," << N << ")" << std::endl;
				return false;
			}

			if(sector0.crc_sts!=sector1.crc_sts)
			{
				std::cout << "CRC Error Status Do Not Match." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << C << "," << H << "," << R << "," << N << ")" << std::endl;
				return false;
			}

			if(sector0.data.size()!=sector1.data.size())
			{
				std::cout << "Sector sizes do not match." << std::endl;
				std::cout << "Sector (C,H,R,N)=(" << C << "," << H << "," << R << "," << N << ")" << std::endl;
				return false;
			}

			if(true==opt.ignoreCRCErrorSector && true==sector0.crc_sts)
			{
				continue;
			}

			for(int i=0; i<sector0.data.size(); ++i)
			{
				if(sector0.data[i]!=sector1.data[i])
				{
					std::cout << "Sector contents do not match." << std::endl;
					std::cout << "Sector (C,H,R,N)=(" << C << "," << H << "," << R << "," << N << ")" << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

