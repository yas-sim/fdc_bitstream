/*
pauline2raw
Pauline .hxcstream samples to fdc_bitstream .RAW file converter.
By Soji Yamakawa (CaptainYS, http://www.ysflight.com)
*/

#ifndef PAULINE2RAW_IS_INCLUDED
#define PAULINE2RAW_IS_INCLUDED
/* { */


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
[Chunk]
Chunk Header
+0  DWORD    CHKH
+4  DWORD    Total size of packet + data + CRC
+8  DWORD    Packet Number (Chunk Number?)

[Packet]
+0  DWORD    Packet Type
         0:Metadata
         1:IO Packet Stream  I don't know what it is
         2:Data Stream
(If Packet Type==0)
+4  DWORD    N Data Size excluding Packet Type and this DWORD
+8  N BYTES  Data
             The official loader searches for "sample_rate_hz 25000000" or "sample_rate_hz 50000000" in the text
             to find the sampling rate.
(If Packet Type==1)
+4  DWORD    N Data Size excluding Packet Type and this DWORD
+8  DWORD    Packed size  (Packet Size+3)&~3 must match N-8
+12 DWORD    Unpacked size
+16 N BYTES  LZ4 compressed data
(If Packet Type==2)
+4  DWORD    N Data Size excluding packet Type and this DWORD
+8  DWORD    Packed size  (Packet Size+3)&~3 must match N-12
+12 DWORD    Unpacked size
+16 DWORD    Number of pulses (Apparently needs to be less than 500*1000*1000)
+20 N BYTES  LZ4 compressed data

Last 4 Bytes
DWORD    CRC Code



[Unpacked Pulse]
If (byteData&0x80)==0 then:      (0xxxxxxx)
    Take the byte data as pulse.
else if (byteData&0xC0)==0x80 then:  (10xxxxxx)
    Take ((this byte&0x3F)<<8)|(next byte) as pulse.
else if (byteData&0xE0)==0xC0 then:  (110xxxxx)
    Take ((this byte&0x1F)<<16)|(next byte<<8)|(2 bytes ahead) as pulse.
else if (byteData&0xF0)==0xE0 then:  (1110xxxx)
    Take ((this byte&0xF)<<24)|(next byte<<16)|(2 bytes ahead<<8)|(3 bytes ahead) as pulse.
Move pointer by the number of bytes used.



[IO]
1250000 samples for 800ms at 25MHz pulse-sampling rate.
If it is sampled at uniform interval, 1562500 samples per second.
25M/1562500=16

Or, 20000000 pulse ticks for 1250000 IO samples.
20000000/1250000=16

Probably, IO sample is taken every 16 ticks.

If pulse-sampling rate is 25MHz, IO sampling rate is 25MHz/16=1.5625MHz.

Which bit corresponds to which pin is unknown.  However, based on the observation, bit 0 must be the index-hole signal.

*/

class HxCStream
{
public:
	enum
	{
		TICKS_PER_IO_READ=16,
	};

	int numPulses=0;
	std::vector <unsigned int> pulse;
	std::vector <bool> indexHole;
	std::vector <uint16_t> io;
	size_t samplingRate=25000000;
	unsigned int timePerTrack=200;
	unsigned int millisecPerRotation=200;

	std::string dumpName,comment,comment2;

	/*!
	*/
	void Clear(void);

	/*!
	*/
	bool LoadTrack(std::ifstream &ifp);

private:
	bool DecodePulse(const std::vector <unsigned char> &encoded,unsigned int pulseCount);
	void MarkIndexHole(void);
	void AnalyzeMetaData(std::string meta);
	/*! Find a tag, and returns a pointer to the string after spaces after the tag.
	*/
	static size_t FindMetaDataTag(std::string meta,std::string tag);
	static std::string GetMetaDataParam(std::string meta,size_t paramStartIndex);
	static bool CheckID(const unsigned char bytes[]);
	static unsigned int ReadDWORD(const unsigned char bytes[]);
	static unsigned int ReadWORD(const unsigned char bytes[]);
	void CalculateMillisecPerRotation(void);

public:
	/*! Returns start- and end- pulse indexes for nth rotation.
	    Returns false if the rotation is not in the sample.
	*/
	bool GetIndexForNthRotation(size_t &iStart,size_t &iEnd,unsigned int nthRotation) const;

	class TrackFileName
	{
	public:
		unsigned int C,H;
		std::string fName;
	};
	/*! Makes a list of file name of the stream files.
	*/
	static std::vector <TrackFileName> MakeTrackFileNameList(std::string path);
};

////////////////////////////////////////////////////////////

class PaulineToRaw
{
public:
	const int TIME_READ_MACKEREL=10; // 10ms

	std::string paulineStreamPath;
	std::string outFileName="output.raw";
	bool allRevolutions=false;
	int nthRevolution=0;            // Will be ignored when allRevolutions is true.
	uint64_t resampleRate=8000000;  // 8MHz
	uint64_t dataBitRate=0;         // 0 for auto (based on rpm)  500000bps 2D/2DD   1000000 2HD

	/*! Set export parameters by command-line arguments.
	*/
	bool RecognizeCommandParameter(int ac,char *av[]);

	/*! Set above parameters and call this.
	*/
	bool ExportRaw(void);
private:
	HxCStream firstStream;
	double spinSpeed=0.2; // Tentatively assume 300rpm.
	bool ExportRaw(std::string fName,const std::vector <HxCStream::TrackFileName> &fileList,int nthRotation);

	void Help(void);
};

/* } */
#endif
