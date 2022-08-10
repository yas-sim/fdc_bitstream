# C++ FDC library to manipulate 2D/MFM bitstream image data

Work in progress.

### Supported features:
- Track read
- Track write
- ID read
- Sector read
- Sector write

### Description:
This program handles the raw FDD bitstream data before the C/D separation is applied.  
The FDC includes a data separator and simple PLL(DLL) emulation features thus, the FDC can reproduce some copy protection data which require sub-bitrate data pulse timing.  

### MFM image data format:
The default sampling rate for the MFM format is 4MHz. The data rate of an orginary 2D/MFM format data is 500KHz. This means, one bit cell will be recorded with eight bits of data in the MFM format.  

`|00001000|00000000|00100000|00010000|00000000| => 0x08,0x00,0x20,0x10,0x00 in MFM format`

```C++
// Header (ofst(byte) = 0)
typedef struct mfm_header_ {
    uint8_t     id_str[8];                  //  "MFM_IMG "
    uint64_t    track_table_offset;         // 
    uint64_t    number_of_tracks;           //
    uint64_t    spindle_time_ns;            //  Time for 1 rotation (ns unit) 
    uint64_t    data_bit_rate;              //  Data bit rate (bit/sec)    MFM=500Kbit/sec = 500,000
    uint64_t    sampling_rate;              //  Sampling rate of the bit stream data     4MHz = 4,000,000
} mfm_header;

// Track offset table (ofst(byte) = header.track_table_offset)
typedef struct track_table_ {
    uint64_t    offset;                     // Offset to the track data (unit=byte, from the top of the file == absolute offset)
    uint64_t    length_bit;                 // Track data length (uint=bits, not bytes)
} mfm_track_table;

// Track data * 84
//   ofst(byte) = track_table[track#].offset
//   size(byte) = track_table[track#].length_bit/8 + (track_table[track#].length%8)?1:0)
```
[test](docs/html/index.html)
