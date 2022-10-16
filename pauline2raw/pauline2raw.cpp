/*
pauline2raw
Pauline .hxcstream samples to fdc_bitstream .RAW file converter.
By Soji Yamakawa (CaptainYS, http://www.ysflight.com)
*/
#include "lz4.h"
#include "pauline2raw.h"



void HxCStream::Clear(void)
{
	numPulses=0;
	pulse.clear();
	io.clear();
	indexHole.clear();
}

bool HxCStream::LoadTrack(std::ifstream &ifp)
{
	bool returnCode=true;
	const int chunkHeaderSize=12;

	Clear();

	while(true!=ifp.eof())
	{
		auto chunkPtr=ifp.tellg();

		unsigned char chunkHeader[chunkHeaderSize]={0,0,0,0};
		ifp.read((char *)chunkHeader,chunkHeaderSize);

		if(true==ifp.eof())
		{
			std::cout << "EOF" << std::endl;
			break;
		}

		if(true!=CheckID(chunkHeader))
		{
			break;
		}

		size_t packetSize=ReadDWORD(chunkHeader+4);
		size_t packetNum=ReadDWORD(chunkHeader+8);

		if(true!=quiet)
		{
			std::cout << "Chunk #" << packetNum << " at " << chunkPtr << " (" << packetSize << " bytes)" << std::endl;
		}

		std::vector <unsigned char> packetData;
		unsigned char crcBytes[4];

		const unsigned int dataSize=packetSize-chunkHeaderSize-4;

		packetData.resize(dataSize);
		ifp.read((char *)packetData.data(),dataSize);

		ifp.read((char *)crcBytes,4);

		// Nicer to check CRC.  But, I'm lazy.

		for(size_t packetTop=0; packetTop<packetData.size(); )
		{
			auto packetPtr=packetData.data()+packetTop;
			auto packetType=ReadDWORD(packetPtr);
			size_t packetDataSize=ReadDWORD(packetPtr+4);
			switch(packetType)
			{
			case 0:
				if(true!=quiet)
				{
					std::cout << "[Metadata] (" << packetDataSize << "+8 bytes)" << std::endl;
				}
				{
					std::string meta;
					for(size_t i=0; i<packetDataSize; ++i)
					{
						meta.push_back(packetPtr[8+i]);
					}
					AnalyzeMetaData(meta);
				}
				packetTop+=packetDataSize+8;
				break;
			case 1:
				if(true!=quiet)
				{
					std::cout << "[IO] (" << packetDataSize << "+8 bytes)" << std::endl;
				}
				{
					auto packedSize=ReadDWORD(packetPtr+8);
					auto unpackedSize=ReadDWORD(packetPtr+12);
					if(((packedSize+3)&~3)+8!=packetDataSize)
					{
						std::cout << "Packet Data Size and Packed Size are inconsistent." << " " << packedSize << " " << packetDataSize << std::endl;
						returnCode=false;
					}

					std::vector <unsigned char> extBuf;
					extBuf.resize(unpackedSize);
					LZ4_decompress_safe((const char *)packetPtr+16,(char *)extBuf.data(),packedSize,unpackedSize);

					for(size_t i=0; i+1<extBuf.size(); i+=2)
					{
						io.push_back(ReadWORD(extBuf.data()+i));
					}
				}
				packetTop+=packetDataSize+8;
				break;
			case 2:
				if(true!=quiet)
				{
					std::cout << "[Pulse] (" << packetDataSize << "+8 bytes)" << std::endl;
				}
				{
					auto packedSize=ReadDWORD(packetPtr+8);
					auto unpackedSize=ReadDWORD(packetPtr+12);
					auto pulseCount=ReadDWORD(packetPtr+16);
					if(((packedSize+3)&~3)+12!=packetDataSize)
					{
						std::cout << "Packet Data Size and Packed Size are inconsistent." << " " << packedSize << " " << packetDataSize << std::endl;
						returnCode=false;
					}

					std::vector <unsigned char> extBuf;
					extBuf.resize(unpackedSize);
					LZ4_decompress_safe((const char *)packetPtr+20,(char *)extBuf.data(),packedSize,unpackedSize);

					DecodePulse(extBuf,pulseCount);

					numPulses+=pulseCount;
				}
				packetTop+=packetDataSize+8;
				break;
			case 0xFFFFFFFF:
				if(true!=quiet)
				{
					std::cout << "[Padding]" << std::endl;
				}
				packetTop=packetData.size();
				break;
			default:
				std::cout << "[Unknown Packet " << packetType << "]" << std::endl;
				packetTop=packetData.size();
				break;
			}
		}
	}

	MarkIndexHole();
	CalculateMillisecPerRotation();

	if(true!=quiet)
	{
		std::cout << "Time per Track " << timePerTrack << "ms" << std::endl;
		std::cout << millisecPerRotation << "ms per rotation." << std::endl;
	}

	if(numPulses!=pulse.size())
	{
		std::cout << "Total pulse count and read pulse count do not match. " << numPulses << " " << pulse.size() << std::endl;;
	}

	size_t totalTime=0;
	for(auto p : pulse)
	{
		totalTime+=p;
	}

	size_t millisec=totalTime*1000/samplingRate;
	if(true!=quiet)
	{
		std::cout << "Total Pulse: " << totalTime << std::endl;
		std::cout << "Total Time:  " << millisec << "ms" << std::endl;
		std::cout << "Total IO:    " << io.size() << std::endl;
		std::cout << "Pulse Count: " << numPulses << std::endl;
	}

	bool prev=false;
	for(size_t i=0; i<indexHole.size(); ++i)
	{
		if(prev!=indexHole[i])
		{
			if(true!=quiet)
			{
				std::cout << "Index " << (indexHole[i] ? "ON " : "OFF") << " at " << i << std::endl;
			}
		}
		prev=indexHole[i];
	}

	pulse.push_back(0); // HxC Floppy Disk Emulator Software source adds one zero at the end.  Probably the terminator.
	return returnCode;
}

bool HxCStream::DecodePulse(const std::vector <unsigned char> &encoded,unsigned int pulseCount)
{
	size_t pulseRead=0;
	size_t i=0;
	while(i<encoded.size() && pulseRead<pulseCount)
	{
		unsigned int data=0;
		if(0==(encoded[i]&0x80))
		{
			data=encoded[i];
			++i;
		}
		else if(i+1<encoded.size() && 0x80==(encoded[i]&0xC0))
		{
			data=(encoded[i]&0x3F);
			data<<=8;
			data|=encoded[i+1];
			i+=2;
		}
		else if(i+2<encoded.size() && 0xC0==(encoded[i]&0xE0))
		{
			data=(encoded[i]&0x1F);
			data<<=8;
			data|=encoded[i+1];
			data<<=8;
			data|=encoded[i+2];
			i+=3;
		}
		else if(i+3<encoded.size() && 0xE0==(encoded[i]&0xF0))
		{
			data=(encoded[i]&0x0F);
			data<<=8;
			data|=encoded[i+1];
			data<<=8;
			data|=encoded[i+2];
			data<<=8;
			data|=encoded[i+3];
			i+=4;
		}
		else
		{
			std::cout << "Encoding Error." << std::endl;
			return false;
		}
		pulse.push_back(data);
		++pulseRead;
	}

	if(i!=encoded.size())
	{
		std::cout << "Warning: Not all bytes have been used." << std::endl;
	}
	else if(pulseRead!=pulseCount)
	{
		std::cout << "Warning: Stopped short of expected pulse count." << std::endl;
	}

	return true;
}

void HxCStream::MarkIndexHole(void)
{
	indexHole.resize(pulse.size());
	for(auto &b : indexHole)
	{
		b=false;
	}

	size_t sum=0;
	for(size_t i=0; i<pulse.size(); ++i)
	{
		auto ioIndex=sum/TICKS_PER_IO_READ;
		if(ioIndex<io.size() && 0!=(io[ioIndex]&1))
		{
			indexHole[i]=true;
		}
		sum+=pulse[i];
	}
}

void HxCStream::AnalyzeMetaData(std::string meta)
{
	auto pos=FindMetaDataTag(meta,"sample_rate_hz");
	if(std::string::npos!=pos)
	{
		samplingRate=atol(meta.c_str()+pos);
		std::cout << "Sampling Rate " << samplingRate << "Hz" << std::endl;
	}

	pos=FindMetaDataTag(meta,"dump_name");
	if(std::string::npos!=pos)
	{
		dumpName=GetMetaDataParam(meta,pos);
	}

	pos=FindMetaDataTag(meta,"dump_comment");
	if(std::string::npos!=pos)
	{
		comment==GetMetaDataParam(meta,pos);
	}

	pos=FindMetaDataTag(meta,"dump_comment2");
	if(std::string::npos!=pos)
	{
		comment2==GetMetaDataParam(meta,pos);
	}

	pos=FindMetaDataTag(meta,"time_per_track");
	if(std::string::npos!=pos)
	{
		timePerTrack=atol(meta.c_str()+pos);
	}
}

/* static */ size_t HxCStream::FindMetaDataTag(std::string meta,std::string tag)
{
	auto found=meta.find(tag);
	if(found!=std::string::npos)
	{
		for(size_t i=found+tag.size(); i<meta.size(); ++i)
		{
			if(' '!=meta[i])
			{
				return i;
			}
		}
	}
	return std::string::npos;
}

/* static */ std::string HxCStream::GetMetaDataParam(std::string meta,size_t paramStartIndex)
{
	std::string str;
	if(meta[paramStartIndex]=='\"')
	{
		for(auto i=1+paramStartIndex; i<meta.size() && '\"'!=meta[i]; ++i)
		{
			str.push_back(meta[i]);
		}
	}
	else
	{
		for(auto i=paramStartIndex; i<meta.size() && 0x0A!=meta[i] && 0x0D!=meta[i]; ++i)
		{
			str.push_back(meta[i]);
		}
	}
	return str;
}

bool HxCStream::CheckID(const unsigned char bytes[])
{
	return bytes[0]=='C' && bytes[1]=='H' && bytes[2]=='K' && bytes[3]=='H';
}

unsigned int HxCStream::ReadDWORD(const unsigned char bytes[])
{
	unsigned int dat=0;
	dat|=bytes[3];
	dat<<=8;
	dat|=bytes[2];
	dat<<=8;
	dat|=bytes[1];
	dat<<=8;
	dat|=bytes[0];
	return dat;
}

unsigned int HxCStream::ReadWORD(const unsigned char bytes[])
{
	unsigned int dat=bytes[1];
	dat<<=8;
	dat|=bytes[0];
	return dat;
}

void HxCStream::CalculateMillisecPerRotation(void)
{
	int state=0;

	size_t trackStart=0;
	for(size_t i=0; i<io.size(); ++i)
	{
		bool indexBit=io[i]&1;
		if(0==state && true==indexBit)
		{
			trackStart=i;
			state=1;  // Track starts.
		}
		else if(1==state && true!=indexBit)
		{
			state=2;  // Passed index hole.
		}
		else if(2==state && true==indexBit)
		{
			state=3;  // End of one rotation.

			size_t trackLength=i-trackStart;
			trackLength*=TICKS_PER_IO_READ;

			trackLength*=1000;
			trackLength/=samplingRate;

			millisecPerRotation=trackLength;
			break;
		}
	}
}

bool HxCStream::GetIndexForNthRotation(size_t &iStart,size_t &iEnd,unsigned int nthRotation) const
{
	int state=0;
	iStart=1; // The first pulse is garbage.  Throw it away.
	for(int i=1; i<pulse.size(); ++i)
	{
		if(0==state && true==indexHole[i])
		{
			iStart=i;
			state=1;
		}
		else if(1==state && true!=indexHole[i])
		{
			state=2;
		}
		else if(2==state && true==indexHole[i])
		{
			iEnd=i;
			state=0;
			if(0==nthRotation)
			{
				return true;
			}
			--nthRotation;
		}
	}
	if(0==nthRotation)
	{
		iEnd=pulse.size()-1;
		return true;
	}
	return false;
}

size_t HxCStream::MakeOverlap(size_t iStart,size_t iEnd,float overlap) const
{
	float len=(float)(iEnd-iStart);
	len*=overlap;
	return std::min(iStart+(size_t)len,pulse.size()-1);
}

size_t HxCStream::ExactCut(size_t iStart,size_t iEnd,int searchRange,size_t searchWindowSize) const
{
	if(iEnd<searchRange || pulse.size()<=iEnd+searchRange)
	{
		return iEnd;
	}

	size_t iEndBest=iEnd;
	size_t maxError=0x7fffffff;
	for(size_t i=iEnd-searchRange; i<=iEnd+searchRange; ++i)
	{
		size_t e=0;
		for(int j=0; j<searchWindowSize; ++j)
		{
			int64_t p0=pulse[iStart+j];
			int64_t p1=pulse[i+j];
			e+=(p1-p0)*(p1-p0);
		}
		if(e<maxError)
		{
			iEndBest=i;
			maxError=e;
		}
		// std::cout << "[" << i << "]" << e << std::endl;
	}

	if(iStart+1<iEndBest)
	{
		--iEndBest; // iEnd is inclusive.
	}

	return iEndBest;
}

std::vector <HxCStream::TrackFileName> HxCStream::MakeTrackFileNameList(std::string path)
{
	std::vector <TrackFileName> files;
	{
		std::ifstream ifp(path,std::ios::binary);
		if(true==ifp.is_open())
		{
			// path is a file name, not a directory name.
			while(0<path.size() && path.back()!='/' && path.back()!='\\' && path.back()!=':')
			{
				path.pop_back();
			}
			ifp.close();
		}
	}

	if(0<path.size() && path.back()!='/' && path.back()!='\\' && path.back()!=':')
	{
		path.push_back('/');
	}

	for(unsigned int track=0; track<200; ++track)
	{
		auto C=track/2;
		auto H=track%2;

		std::string ful=path;
		ful+="track";
		ful.push_back('0'+C/10);
		ful.push_back('0'+C%10);
		ful.push_back('.');
		ful.push_back('0'+H);
		ful+=".hxcstream";

		std::ifstream ifp(ful,std::ios::binary);
		if(ifp.is_open())
		{
			TrackFileName tfn;
			tfn.C=C;
			tfn.H=H;
			tfn.fName=ful;
			files.push_back(tfn);
		}
	}
	return files;
}

////////////////////////////////////////////////////////////

bool PaulineToRaw::RecognizeCommandParameter(int ac,char *av[])
{
	bool streamPathSet=false;
	for(int i=1; i<ac; ++i)
	{
		std::string arg=av[i];
		for(auto &c : arg)
		{
			if('a'<=c && c<='z')
			{
				c=c+'A'-'a';
			}
		}
		if("-H"==arg || "-HELP"==arg || "-?"==arg)
		{
			Help();
			return false;
		}
		else if(("-O"==arg || "-OUT"==arg) && i+1<ac)
		{
			outFileName=av[i+1];
			++i;
		}
		else if("-REV"==arg && i+1<ac)
		{
			nthRevolution=atoi(av[i+1]);
			++i;
		}
		else if("-ALLREV"==arg || "-ALLREVS"==arg)
		{
			allRevolutions=true;
		}
		else if("-2HD"==arg)
		{
			dataBitRate=1000000;
		}
		else if("-2D"==arg || "-2DD"==arg)
		{
			dataBitRate=500000;
		}
		else if("-RESAMPLE"==arg && i+1<ac)
		{
			resampleRate=atoi(av[i+1]);
			++i;
		}
		else if("-OVERLAP"==arg && i+1<ac)
		{
			overlap=atof(av[i+1]);
			exactCut=false;
			++i;
		}
		else if(true!=streamPathSet)
		{
			paulineStreamPath=av[i];
			streamPathSet=true;
		}
		else
		{
			std::cout << "Unrecognized parameter: " << av[i] << std::endl;
			goto HELP;
		}
	}

	if(true!=streamPathSet)
	{
		std::cout << "Stream path/file is not set." << std::endl;
		goto HELP;
	}

	return true;

HELP:
	std::cout << "Type:" << std::endl;
	std::cout << "  pauline2raw -h" << std::endl;
	std::cout << "for help." << std::endl;
	return false;
}

bool PaulineToRaw::ExportRaw(void)
{
	auto fileList=HxCStream::MakeTrackFileNameList(paulineStreamPath);
	if(fileList.empty())
	{
		std::cout << "Pauline stream does not exist." << std::endl;
		return false;
	}

	std::ifstream ifp(fileList[0].fName,std::ios::binary);
	if(true!=firstStream.LoadTrack(ifp))
	{
		std::cout << "Cannot read the first track." << std::endl;
		return false;
	}
	ifp.close();

	spinSpeed=(double)firstStream.millisecPerRotation/1000.0;
	if(0==dataBitRate)
	{
		if(160<=firstStream.millisecPerRotation && firstStream.millisecPerRotation<=170) // 166.7
		{
			// 360rpm.  Must be 2HD.
			dataBitRate=1000000;
		}
		else
		{
			// Assume 300rpm.  Must be 2D/2DD.
			dataBitRate=500000;
		}
	}

	if(true==allRevolutions)
	{
		int NRev=(firstStream.timePerTrack+TIME_READ_MACKEREL)/firstStream.millisecPerRotation;
		for(int i=0; i<NRev; ++i)
		{
			auto outFileName=this->outFileName;
			std::string ext;
			for(int j=outFileName.size()-1; 0<=j; --j)
			{
				if('.'==outFileName[j])
				{
					ext=outFileName.substr(j+1);
					outFileName.resize(j+1);
					break;
				}
			}

			outFileName.push_back('0'+i/10);
			outFileName.push_back('0'+i%10);
			outFileName.push_back('.');
			outFileName+=ext;
			if(true!=ExportRaw(outFileName,fileList,i))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return ExportRaw(outFileName,fileList,nthRevolution);
	}
}

bool PaulineToRaw::ExportRaw(std::string fName,const std::vector <HxCStream::TrackFileName> &fileList,int nthRotation)
{
	unsigned int minTrack=0xffffffff,maxTrack=0;
	for(auto tfn : fileList)
	{
		auto trk=tfn.C*2+tfn.H;
		minTrack=std::min(minTrack,trk);
		maxTrack=std::max(maxTrack,trk);
	}
	if(minTrack>maxTrack)
	{
		return false;
	}

	std::ofstream ofp(fName);
	if(true!=ofp.is_open())
	{
		return false;
	}

	ofp << "**TRACK_RANGE " << minTrack << " " << maxTrack << std::endl;
	ofp << "**SAMPLING_RATE " << resampleRate << std::endl;
	ofp << "**BIT_RATE " << dataBitRate << std::endl;
	ofp << "**SPIN_SPD " << spinSpeed << std::endl;

	{
		int ovlp=(int)(100.0f*(overlap-1.0f));
		if(0!=ovlp)
		{
			ofp << "**OVERLAP " << ovlp << std::endl;
		}
	}

	ofp << "**START" << std::endl;
	for(auto tfn : fileList)
	{
		std::ifstream ifp(tfn.fName,std::ios::binary);
		if(true!=ifp.is_open())
		{
			std::cout << "Could not open a stream file." << std::endl;
			return false;
		}

		HxCStream hxc;
		if(true==hxc.LoadTrack(ifp))
		{
			size_t iStart,iEnd;
			if(true!=hxc.GetIndexForNthRotation(iStart,iEnd,nthRotation))
			{
				std::cout << "The rotation is not sampled." << std::endl;
				return false;
			}

			if(true==exactCut)
			{
				int searchBytes=4;
				size_t searchWindowSize=256; // Ad-hoc
				iEnd=hxc.ExactCut(iStart,iEnd,16*searchBytes,searchWindowSize); // 16 pulses for one byte.
			}

			iEnd=hxc.MakeOverlap(iStart,iEnd,overlap);

			ofp << "**TRACK_READ " << tfn.C << " " << tfn.H << std::endl;
			unsigned int lineCount=0;

		    const uint8_t encode_base = ' ';
		    const uint8_t max_length = 'z' - encode_base;
		    const uint8_t extend_char = '{';

			uint64_t leftOverTime=0;

			for(auto i=iStart; i<=iEnd; ++i)
			{
				if(0==lineCount)
				{
					ofp << "~";
				}
				uint64_t pulseLen=hxc.pulse[i];
				pulseLen*=resampleRate;

				pulseLen+=leftOverTime;
				leftOverTime=pulseLen%hxc.samplingRate;

				pulseLen/=hxc.samplingRate;

				while(max_length<pulseLen)
				{
					ofp << extend_char;
					pulseLen-=max_length;
					++lineCount;
				}
				ofp << (char)(encode_base+pulseLen);
				++lineCount;
				if(80<=lineCount || i==iEnd)
				{
					ofp << std::endl;
					lineCount=0;
				}
			}
			ofp << "**TRACK_END" << std::endl;
		}
		else
		{
			std::cout << "Something went wrong." << std::endl;
			return false;
		}
	}

	ofp << "**COMPLETED" << std::endl;

	return true;
}

void PaulineToRaw::Help(void)
{
	std::cout << "Usage:" << std::endl;
	std::cout << "    pauline2raw stream-path <options>" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h" << std::endl;
	std::cout << "    Show help." << std::endl;
	std::cout << "  -o output-filename.raw" << std::endl;
	std::cout << "  -out output-filename.raw" << std::endl;
	std::cout << "    Specify output file name.  Default is output.raw" << std::endl;
	std::cout << "  -rev number" << std::endl;
	std::cout << "    If multiple-revolution sample, specify which revolution to convert." << std::endl;
	std::cout << "  -allrev" << std::endl;
	std::cout << "    If multiple-revolution sample, output all revolutions in separate .raw files." << std::endl;
	std::cout << "  -2hd" << std::endl;
	std::cout << "    Set data bit rate to 1000000 (1Mbps)." << std::endl;
	std::cout << "    Default is automatic from rotation speed info in the hxcstream." << std::endl;
	std::cout << "  -2d" << std::endl;
	std::cout << "  -2dd" << std::endl;
	std::cout << "    Set data bit rate to 500000 (500Kbps)." << std::endl;
	std::cout << "    Default is automatic from rotation speed info in the hxcstream." << std::endl;
	std::cout << "  -resample sampling_rate_in_hz" << std::endl;
	std::cout << "    Set re-sampling rate.  Default is 8000000 (8MHz)" << std::endl;
	std::cout << "  -overlap overlap_ratio" << std::endl;
	std::cout << "    Set overlap ratio.  In multiple-revolution sample, setting greater than 1.0 overlap" << std::endl;
	std::cout << "    will take samples beyond the index hole to capture cross-index sectors." << std::endl;
	std::cout << "    Overlap ratio 1.0 means exactly one revolution." << std::endl;
	std::cout << "    Overlap ratio 1.1 means take 10% more than one revolution." << std::endl;
	std::cout << "    This option also disables exact-cut feature." << std::endl;
	std::cout << "    -overlap 1.0 for disabling exact-cut and cut out one revolution." << std::endl;
}
