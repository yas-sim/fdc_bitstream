#include "image_rdd.h"
#include "byte_array.h"

#include "fdc_bitstream.h"

#include <fstream>
#include <iostream>
#include <iomanip>

#define MB8877_STATUS_CRC_ERROR        0x08
#define MB8877_STATUS_RECORD_NOT_FOUND 0x10
#define MB8877_STATUS_DELETED_DATA     0x20

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

                fdc.set_pos(id.pos);
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

                if(true!=MFM)
                {
                    sectorHeader[6]|=1;
                }
                // if(true==needResample)
                //{
                //    sectorHeader[6]|=2;
                //}
                // if(true==leafInTheForest)
                //{
                //    sectorHeader[6]|=4;
                //}


                fdc.set_pos(id.pos);
                auto read_sect = fdc.read_sector(id.C, id.R);

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

                int resampleCount=1;
                for(int i=0; i<resampleCount; ++i)
                {
                    if(0<i)
                    {
                        // Randomize identified unstable bytes.
                    }
                    ofp.write((char *)sectorHeader,16);
                    while(read_sect.data.size()<len)
                    {
                        read_sect.data.push_back(0);
                    }
                    read_sect.data.resize(len);

                    ofp.write((char *)read_sect.data.data(),read_sect.data.size());
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
