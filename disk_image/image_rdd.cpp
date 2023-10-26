#include "image_rdd.h"
#include "byte_array.h"

#include "fdc_bitstream.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <array>

#define RESAMPLE_COUNT  12
#define REWIND_WIDTH  2048
// 25MHz sampling rate:  8 counts per pulse window, 16 pulse windows per byte, 8*16=128 counts per byte, 2048 counts makes 16 bytes.
// 50MHz sampling rate: 16 counts per pulse window, 16 pulse windows per byte, 8*16=128 counts per byte, 2048 counts makes  8 bytes.

#define MB8877_STATUS_CRC_ERROR        0x08
#define MB8877_STATUS_RECORD_NOT_FOUND 0x10
#define MB8877_STATUS_DELETED_DATA     0x20

const unsigned int READ_TRACK_EX_POS=2;
const unsigned int READ_TRACK_EX_ERR=3;


template <class T>
static inline T Rewind(T pos)
{
	return (REWIND_WIDTH<pos ? pos-REWIND_WIDTH : 0);
}

class Fluctuator_Guard
{
private:
	fdc_bitstream *fdcPtr;
public:
	Fluctuator_Guard(fdc_bitstream &fdc)
	{
		fdcPtr=&fdc;
	}
	~Fluctuator_Guard()
	{
		fdcPtr->disable_fluctuator();
	}
};

void disk_image_rdd::read(const std::string file_name) {
    std::cout << "Reading from RDD is not supported yet." << std::endl;
}

void disk_image_rdd::read(std::istream &ifp) {
    std::cout << "Reading from RDD is not supported yet." << std::endl;
}


void disk_image_rdd::write(const std::string file_name) const {
    std::ofstream ofp(file_name,std::ios::binary);
    write(ofp);
}

bool disk_image_rdd::write(std::ostream &ofp) const {
    const size_t sector_length_table[] = { 128, 256, 512, 1024 };
    std::ios::fmtflags flags_saved = std::cout.flags();
    fdc_bitstream fdc;


    static char padding[1024];
    for(auto &p : padding) {
        p=0;
    }


    char fileID[16]={
        'R','E','A','L','D','I','S','K','D','U','M','P',0,0,0,0
    };
    ofp.write(fileID,16);

    unsigned char beginDisk[16]={0,0,0,0,0,0,0,0};
    beginDisk[1]=0; // Version
    if(m_base_prop.m_data_bit_rate == 1e6) {   // 1Mbps == 2HD_MFM , 500Kbps == 2D/2DD_MFM
        beginDisk[2] = 0x20;        // 2HD
    } else if (m_base_prop.m_number_of_tracks > 84) {
        beginDisk[2] = 0x10;        // 2DD
    }
    else {
        beginDisk[2]=0; // Assume 2D.
    }
    beginDisk[3]=0; // Write Protected -> 1
    beginDisk[4]=0xFF; // Converted.  Not directly from real device.
    ofp.write((char *)beginDisk,16);

    char diskName[32];
    memset(diskName,0,32);
    ofp.write(diskName,32);


    size_t total_sector_good = 0;
    size_t total_sector_bad = 0;

    fdc.set_fdc_params(m_base_prop.m_sampling_rate,m_base_prop.m_data_bit_rate);

    for (size_t track_n = 0; track_n < m_base_prop.m_number_of_tracks; track_n++) {
        if(m_verbose) {
            std::cout << std::setw(4) << std::dec << track_n << ":";
        }

        unsigned char beginTrack[16]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        beginTrack[1]=track_n/2; // Cylinder
        beginTrack[2]=track_n%2; // Head
        ofp.write((char *)beginTrack,16);

        size_t sector_good=0,sector_bad=0;

        for(int retry=0; retry<2; ++retry) // Try MFM, and then FM
        {
            const bool MFM=(0==retry ? true : false);

            bit_array mfm_trk = m_track_data[track_n];
            fdc.set_track_data(mfm_trk);
            fdc.set_pos(0);
            fdc.set_vfo_type(m_vfo_type);
            fdc.set_vfo_gain_val(m_gain_l, m_gain_h);
            std::vector<fdc_bitstream::id_field> id_list = fdc.read_all_idam();
			std::vector <bool> unstable_pulses=MarkUnstablePulses(fdc,id_list);

			bool leafInTheForest=CheckLeafInTheForestSignature(track_n/2,track_n%2,id_list);

            if(0==retry && 0==id_list.size())
            {
                // Try again.  Unformat or can be FM format.
                continue;
            }

            for(auto id : id_list) {
                unsigned char idMark[16]={2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
                idMark[1]=id.C;
                idMark[2]=id.H;
                idMark[3]=id.R;
                idMark[4]=id.N;
                idMark[5]=(id.crc_val>>8);
                idMark[6]=(id.crc_val&0xFF);
                if(true==id.crc_sts) {
                    idMark[7]|=MB8877_STATUS_CRC_ERROR;
                }

                fdc.set_pos(Rewind(id.pos));
                auto read_sect = fdc.read_sector(id.C, id.R);

                if(true==read_sect.record_not_found) {
                    idMark[7]|=MB8877_STATUS_RECORD_NOT_FOUND;
                }

                ofp.write((char *)idMark,16);
            }

            for(auto id : id_list) {
                unsigned char sectorHeader[16]={3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
                sectorHeader[1]=id.C;
                sectorHeader[2]=id.H;
                sectorHeader[3]=id.R;
                sectorHeader[4]=id.N;

                fdc.set_pos(Rewind(id.pos));
                auto read_sect = fdc.read_sector(id.C, id.R);

				bool corocoroTypeA=IsFM7CorocoroTypeA(fdc,id.pos,id.C,id.H,id.R,read_sect.crc_sts,read_sect.data);
				bool corocoroTypeB=IsFM7CorocoroTypeB(fdc,id.pos,id.C,id.H,id.R,read_sect.crc_sts,read_sect.data);


                if(true!=MFM)
                {
                    sectorHeader[6]|=1;
                }
                if(true==corocoroTypeA || true==corocoroTypeB)
                {
                    sectorHeader[6]|=2;
                }
                if(true==leafInTheForest)
                {
                    sectorHeader[6]|=4;
                }


                if(read_sect.record_not_found == false) {
                    if(true==read_sect.dam_type) {
                        sectorHeader[5]|=MB8877_STATUS_DELETED_DATA;
                    }
                    if(true==read_sect.crc_sts)
                    {
                        sectorHeader[5]|=MB8877_STATUS_CRC_ERROR;
                    }
                }
                else
                {
                    sectorHeader[5]|=MB8877_STATUS_RECORD_NOT_FOUND;
                }

                auto start_pos=read_sect.data_pos;
                auto end_pos=read_sect.data_end_pos;;
                if (end_pos < start_pos) {
                    end_pos += mfm_trk.get_bit_length();       // wrap around correction
                }

                uint64_t elapsed=end_pos-start_pos;
                uint64_t microsec=elapsed*1000000LL/(uint64_t)m_base_prop.m_sampling_rate;
                sectorHeader[0x0B]= microsec     &0xFF;
                sectorHeader[0x0C]=(microsec>> 8)&0xFF;
                sectorHeader[0x0D]=(microsec>>16)&0xFF;

                if(m_verbose) {
                    std::cout << int(id.C) << " " << int(id.H) << " " << int(id.R) << " POS0:" << id.pos << " POS1:" << fdc.get_real_pos() << " " << microsec << "usec" << std::endl;
                }

                unsigned int len=(128<<(id.N&3));
                sectorHeader[0x0E]= len    &0xFF;
                sectorHeader[0x0F]=(len>>8)&0xFF;

                if(true!=corocoroTypeA && true!=corocoroTypeB)
                {
                    ofp.write((char *)sectorHeader,16);
                    while(read_sect.data.size()<len)
                    {
                        read_sect.data.push_back(0);
                    }
                    read_sect.data.resize(len);
                    ofp.write((char *)read_sect.data.data(),read_sect.data.size());
                }
				else
				{
					Fluctuator_Guard guard(fdc);

					fdc_bitstream::sector_data samples[RESAMPLE_COUNT];
					for(double fluctuation=0.8; 0.2<fluctuation; fluctuation-=0.05)
					{
						bool tryAgain=false;
						fdc.enable_fluctuator(fluctuation);
						for(auto &s : samples)
						{
							fdc.set_pos(Rewind(id.pos));
							s=fdc.read_sector(id.C,id.R);
						}
						for(auto &s : samples)
						{
							if((true==corocoroTypeA && true!=CheckCorocoroTypeASignature(s.crc_sts,s.data)) ||
							   (true==corocoroTypeB && true!=CheckCorocoroTypeBSignature(s.crc_sts,s.data)))
							{
								tryAgain=true;
								break;
							}
						}
						if(tryAgain!=true)
						{
							break;
						}
					}
					if(true==corocoroTypeA)
					{
						for(auto &s : samples)
						{
							if(1024==s.data.size())
							{
								s.data[0x120]=rand()&255;
								s.data[0x121]=rand()&255;
								s.data[0x122]=rand()&255;
								s.data[0x123]=rand()&255;
							}
						}
					}
					if(true==corocoroTypeB)
					{
						for(auto &s : samples)
						{
							if(128==s.data.size())
							{
								s.data[20]=rand()&255;
								s.data[21]=rand()&255;
								s.data[22]=rand()&255;
								s.data[23]=rand()&255;
								for(int i=24; i<44; ++i)
								{
									s.data[i]=0xF6;
								}
							}
						}
					}

					for(auto &s : samples)
					{
	                    ofp.write((char *)sectorHeader,16);
	                    while(s.data.size()<len)
	                    {
	                        s.data.push_back(0);
	                    }
	                    s.data.resize(len);
	                    ofp.write((char *)s.data.data(),s.data.size());
					}
				}

                if (!read_sect.record_not_found && !read_sect.crc_sts && !id.crc_sts) {  // no error (RNF or DT_CRC error or ID_CRC error)
                    sector_good++;
                    total_sector_good++;
                } else {
                    sector_bad++;
                    total_sector_bad++;
                }
            }


            // Read Track
            fdc.set_pos(0);
            auto track_read = fdc.read_track();
            unsigned char trackReadHeader[16]={4,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
            trackReadHeader[1]=track_n/2; // Cylinder
            trackReadHeader[2]=track_n%2; // Head
            // trackReadHeader[3] MB8877 status, but I'm vague about what it should be.
            trackReadHeader[0x0E]= track_read.size()    &0xFF;
            trackReadHeader[0x0F]=(track_read.size()>>8)&0xFF;

            while(0!=track_read.size()%16)
            {
                track_read.push_back(0);
            }
            ofp.write((char *)trackReadHeader,16);
            ofp.write((char *)track_read.data(),track_read.size());


            if(m_verbose) {
                std::cout << std::setw(4) << std::dec << (sector_good+sector_bad) << "/" << std::setw(4) << sector_bad << "  ";
                if(track_n % 5 == 4) {
                    std::cout << std::endl;
                }
            }

            break; // If it reaches here, don't continue.
        }

        char endTrack[16]={5,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
        ofp.write(endTrack,16);
    }
    char endFile[16]={6,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    ofp.write(endFile,16);

    if(m_verbose) {
        std::cout << std::endl;
        std::cout << "**TOTAL RESULT(GOOD/BAD):" << total_sector_good << " " << total_sector_bad << std::endl;
    }
    std::cout.flags(flags_saved);
}

bool disk_image_rdd::IsFM7CorocoroTypeA(fdc_bitstream &fdc,uint64_t pos,unsigned char C,unsigned char H,unsigned char R,bool crc_sts,const std::vector <uint8_t> &data) const
{
	if(true==CheckCorocoroTypeASignature(crc_sts,data))
	{
		// Signature found.  Now let's verify.

		std::vector <std::array <uint8_t,4> > samples;
		const int nRepeat=16;

		// Reduce fluctuator until first 256 bytes are all 0xE5
		for(double fluctuation=0.8; 0.2<fluctuation; fluctuation-=0.05)
		{
			Fluctuator_Guard guard(fdc);

			samples.clear();
			fdc.enable_fluctuator(fluctuation);
			for(int i=0; i<nRepeat; ++i)
			{
				fdc.set_pos(Rewind(pos));
				auto sect=fdc.read_sector(C,R);
				if(1024!=sect.data.size())
				{
					goto UPDATE_FLUCTUATION;
				}
				for(int j=0; j<256; ++j)
				{
					// Signature: First 256 bytes are 0xE5.
					if(0xE5!=sect.data[j])
					{
						samples.clear();
						goto UPDATE_FLUCTUATION;
					}
				}
				std::array <uint8_t,4> s;
				s[0]=sect.data[0x120];
				s[1]=sect.data[0x121];
				s[2]=sect.data[0x122];
				s[3]=sect.data[0x123];
				samples.push_back(s);
			}
			if(samples.size()==nRepeat)
			{
				break;
			}
		UPDATE_FLUCTUATION:
			;
		}

		if(nRepeat!=samples.size())
		{
			return false;
		}

		// 4 bytes from offset $120 need to be unstable.
		for(int i=1; i<samples.size(); ++i)
		{
			if(samples[0][0]!=samples[i][0] ||
			   samples[0][1]!=samples[i][1] ||
			   samples[0][2]!=samples[i][2] ||
			   samples[0][3]!=samples[i][3])
			{
				std::cout << "C:" << int(C) << " H:" << int(H) << " R:" << int(R) << " Detected Corocoro Protect Type A" << std::endl;
				return true;
			}
		}
	}
	return false;
}
bool disk_image_rdd::IsFM7CorocoroTypeB(fdc_bitstream &fdc,uint64_t pos,unsigned char C,unsigned char H,unsigned char R,bool crc_sts,const std::vector <uint8_t> &data) const
{
	if(true==CheckCorocoroTypeBSignature(crc_sts,data))
	{
		// Signature found.  Now let's verify by fluctuating the VFO.

		const int nRepeat=16;

		// Reduce fluctuator until first 20 bytes are all 0xF7
		for(double fluctuation=0.8; 0.2<fluctuation; fluctuation-=0.05)
		{
			Fluctuator_Guard guard(fdc);

			fdc.enable_fluctuator(fluctuation);
			for(int i=0; i<nRepeat; ++i)
			{
				fdc.set_pos(Rewind(pos));
				auto sect=fdc.read_sector(C,R);
				if(128!=sect.data.size())
				{
					goto UPDATE_FLUCTUATION;
				}

				for(int j=0; j<20; ++j)
				{
					// Signature: First 20 bytes are 0xF7.
					if(0xF7!=sect.data[j])
					{
						goto UPDATE_FLUCTUATION;
					}
				}

				// Ideally, for conclusive identification I want to see data[20] to data[23] changes value.
				// However, only four bytes following stable bytes often do not fluctuate enough.
				// Instead, see if data[25] to data[43] are all 0xF6.
				for(int j=25; j<43; ++j)
				{
					if(0xF6!=sect.data[j])
					{
						goto UPDATE_FLUCTUATION;
					}
				}

				std::cout << "C:" << int(C) << " H:" << int(H) << " R:" << int(R) << " Detected Corocoro Protect Type B" << std::endl;
				return true;
			}
		UPDATE_FLUCTUATION:
			;
		}
	}
	return false;
}
bool disk_image_rdd::CheckCorocoroTypeASignature(bool crc_sts,const std::vector <uint8_t> &data) const
{
	if(true==crc_sts && 1024==data.size())
	{
		for(int i=0; i<256; ++i)
		{
			// Signature: First 256 bytes are 0xE5.
			if(0xE5!=data[i])
			{
				return false;
			}
		}
		return true;
	}
	return false;
}
bool disk_image_rdd::CheckCorocoroTypeBSignature(bool crc_sts,const std::vector <uint8_t> &data) const
{
	if(true==crc_sts && 128==data.size())
	{
		for(int i=0; i<20; ++i)
		{
			// Signature: First 20 bytes are 0xF7.
			if(0xF7!=data[i])
			{
				return false;
			}
		}

		// 20 bytes from offset 24 need to be 0xF6, however, often byte[24] changes.  Also byte[43] often is 0xF7.
		// All 20 bytes may change to zero.
		// The supplied checker will read the sector multiple times and pass if it finds 19 consecutive 0xF6 from offset 24 just once.
		// So, first check 18 bytes from offset 25 are equal.
		for(int i=25; i<43; ++i)
		{
			if(data[i]!=data[25])
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool disk_image_rdd::CheckLeafInTheForestSignature(uint8_t C,uint8_t H,const std::vector <fdc_bitstream::id_field> &id_list) const
{
	// Only confirmed in Thexder and Fire Crystal as of 2023/10/24.
	// There is a track that has 63 sectors with all same C,H,R.
	// All but two sectors are dummy.

	if(60<id_list.size())
	{
		for(auto id : id_list)
		{
			if(id.C!=id_list[0].C ||
			   id.H!=id_list[0].H ||
			   id.R!=id_list[0].R)
			{
				return false;
			}
		}
		std::cout << "C:" << int(C) << " H:" << int(H) << " Detected Leaf-In-The-Forest Protect." << std::endl;
		return true;
	}
	return false;
}

std::vector <bool> disk_image_rdd::MarkUnstablePulses(fdc_bitstream &fdc,const std::vector<fdc_bitstream::id_field> &id_list) const
{
	auto track_data=fdc.get_track_data();
	std::vector <bool> unstable;
	std::vector <bool> confirmed;

	unstable.resize(track_data.get_bit_length());
	confirmed.resize(track_data.get_bit_length());

	auto Fill=[&](size_t start_pos,size_t end_pos,bool isUnstable)
	{
		if(start_pos<=end_pos)
		{
			for(size_t i=start_pos; i<end_pos; ++i)
			{
				if(true!=confirmed[i])
				{
					unstable[i]=isUnstable;
					confirmed[i]=true;
				}
			}
		}
		else // Wrapped around
		{
			for(size_t i=start_pos; i<unstable.size(); ++i)
			{
				if(true!=confirmed[i])
				{
					unstable[i]=isUnstable;
					confirmed[i]=true;
				}
			}
			for(size_t i=0; i<end_pos; ++i)
			{
				if(true!=confirmed[i])
				{
					unstable[i]=isUnstable;
					confirmed[i]=true;
				}
			}
		}
	};


	fdc.disable_fluctuator();

	// want to do for(auto &b : unstable)b=false;, but clang may whine.
	for(size_t i=0; i<unstable.size(); ++i)
	{
		unstable[i]=false;
		confirmed[i]=false;
	}

	// First read sectors, and all bits should be marked stable if there is no CRC error.
	fdc.set_pos(0);
	for(auto id : id_list)
	{
        fdc.set_pos(Rewind(id.pos));
        auto read_sect = fdc.read_sector(id.C, id.R);
        if(true==read_sect.crc_sts) // No CRC Error.  Unlikely to include an unstable bit.
        {
			Fill(read_sect.data_pos,read_sect.data_end_pos,false);
		}
	}

	// Then read track and mark unstable if the error total of the two consecutive byte exceeds 96000 (worse than Pink+Red), mark as unstable.
	auto read_track=fdc.read_track_ex();
	for(size_t i=0; i+1<read_track.size(); ++i)
	{
		auto errsum=read_track[i][READ_TRACK_EX_ERR]+read_track[i+1][READ_TRACK_EX_ERR];
		if(96000<=errsum)
		{
			auto start_pos=read_track[i][READ_TRACK_EX_POS];
			auto end_pos=(i+2<read_track.size() ? read_track[i+2][READ_TRACK_EX_POS] : read_track[i+1][READ_TRACK_EX_POS]);
			Fill(start_pos,end_pos,true);
		}
	}

	return unstable;
}
