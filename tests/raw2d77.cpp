#include <string>

#include "disk_images.h"



// Just in case, if both RAW writer and D77 reader consistently broke.
unsigned char trueSectorData[]=
{
0,1,
0x1a,0x50,0x10,0xce,0x01,0x00,0x86,0xfd,0x1f,0x8b,0x8e,0x10,0x00,0xaf,0x8c,0x18,
0x30,0x8c,0x13,0xbd,0xfe,0x08,0x6c,0x8c,0x0f,0x6c,0x8c,0x0f,0xa6,0x8c,0x0c,0x81,
0x11,0x26,0xed,0x7e,0x10,0x00,0x0a,0x00,0x10,0x00,0x00,0x09,0x00,0x00,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

1,1,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

2,1,
0x00,0xff,0xff,0xff,0xff,0xc0,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,
0x0c,0x0d,0x0e,0x0f,0x10,0x11,0xc0,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0xc6,
0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0xc0,0xc6,0x26,0x27,0x28,0x29,0x2a,0x2b,
0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0xc0,0xc4,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

3,1,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

4,1,
0x0d,0x0a,0x31,0x30,0x20,0x43,0x4c,0x45,0x41,0x52,0x2c,0x26,0x48,0x36,0x46,0x46,
0x46,0x0d,0x0a,0x32,0x30,0x20,0x4c,0x4f,0x41,0x44,0x4d,0x22,0x44,0x4d,0x32,0x30,
0x31,0x39,0x4c,0x22,0x2c,0x2c,0x52,0x0d,0x0a,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

5,1,
0xe4,0x16,0x8f,0xe5,0x1e,0xa8,0xe6,0x1e,0xa8,0xf9,0x03,0xa4,0xe4,0x1c,0x67,0xe5,
0x24,0x67,0xe6,0x1d,0x3c,0xdc,0x41,0x00,0xdd,0x6e,0x00,0xde,0x49,0x01,0xef,0xf9,
0x02,0x00,0x08,0x0d,0x09,0x0d,0x0a,0x0d,0xf9,0x6d,0xfc,0xe4,0x1c,0x5e,0xe5,0x24,
0x6d,0xe6,0x1d,0x43,0xf8,0xb3,0xe4,0x1c,0x65,0xe5,0x24,0x68,0xe6,0x1d,0x3d,0xf7,
0xb4,0xe4,0x1c,0x60,0xe5,0x24,0x5b,0xe6,0x1d,0x2d,0x08,0x00,0x09,0x00,0x0a,0x00,
0xfd,0x63,0xe3,0xfd,0xac,0xe4,0x16,0xfe,0xe5,0x24,0xf1,0xe6,0x1e,0xfe,0xdc,0x3a,
0x00,0xdd,0x52,0x00,0xde,0x25,0x01,0xef,0xf7,0xaf,0x08,0x0d,0x09,0x0d,0x0a,0x0d,
0xf9,0x0d,0xe8,0x08,0x00,0x09,0x00,0x0a,0x00,0xfd,0x9f,0xe3,0xfd,0xad,0xe4,0x16,
0x99,0xe5,0x25,0x3c,0xe6,0x1e,0x99,0xdc,0x37,0x00,0xdd,0x57,0x00,0xde,0x05,0x01,
0xef,0xf8,0x88,0x08,0x0d,0x09,0x0d,0x0a,0x0d,0xf9,0x28,0xdf,0xe4,0x16,0x8c,0xe5,
0x25,0x43,0xe6,0x1e,0xa2,0xf8,0xbb,0xe4,0x16,0x96,0xe5,0x25,0x3d,0xe6,0x1e,0x9a,
0xf7,0xc4,0xe4,0x16,0x99,0xe5,0x25,0x3c,0xe6,0x1e,0x99,0x08,0x00,0x09,0x00,0x0a,
0x00,0xf9,0x04,0x10,0xe3,0xe4,0x16,0xa0,0xe5,0x25,0x37,0xe6,0x1e,0x93,0xfd,0xb1,
0xe4,0x16,0x99,0xe5,0x24,0xf1,0xe6,0x1e,0x99,0xdc,0x3a,0x00,0xde,0x25,0x01,0xef,
0xf8,0xf7,0x08,0x0d,0x09,0x0d,0x0a,0x0d,0xf9,0x0d,0xe1,0x08,0x00,0x09,0x00,0x0a,
0x00,0xfd,0x60,0xe3,0xfd,0x63,0xe4,0x16,0x99,0xe5,0x25,0x3c,0xe6,0x1e,0x99,0xdc,

6,1,
0xe6,0x26,0xaa,0xf9,0x07,0xb9,0xe4,0x1d,0x31,0xe5,0x26,0x93,0xe6,0x26,0x93,0xf7,
0xab,0xe4,0x1d,0x35,0xe5,0x26,0x87,0xe6,0x26,0x87,0xf9,0x06,0xdb,0xe4,0x1d,0x3e,
0xe5,0x26,0x87,0xe6,0x26,0x87,0xf7,0xab,0xe4,0x1c,0x67,0xe5,0x25,0x3c,0xe6,0x25,
0x3c,0xdc,0x57,0x00,0xdd,0x6e,0x00,0xde,0x05,0x01,0xef,0xf8,0x91,0x08,0x0d,0x09,
0x0d,0x0a,0x0d,0xf9,0x08,0x27,0x0a,0x00,0xf7,0x86,0xe0,0xf9,0x04,0x81,0x08,0x00,
0x09,0x00,0xf8,0x80,0xe1,0xe2,0xfd,0xd8,0xe4,0x1e,0x99,0xe5,0x25,0xe1,0xe6,0x25,
0xe1,0xdc,0x52,0x00,0xdd,0x61,0x00,0xde,0xae,0x00,0xef,0xf9,0x02,0x1d,0x08,0x0d,
0x09,0x0d,0x0a,0x0d,0xf9,0x1f,0x13,0x08,0x00,0x09,0x00,0xf9,0x06,0xaf,0xe4,0x1e,
0x8c,0xe5,0x25,0xe9,0xe6,0x25,0xe9,0xf8,0xb3,0xe1,0xe2,0xe4,0x1e,0x91,0xe5,0x25,
0xe6,0xe6,0x25,0xe6,0xf9,0x15,0xe3,0xe4,0x1e,0x9b,0xe5,0x25,0xdf,0xe6,0x25,0xdf,
0xf7,0xba,0xe4,0x1e,0xa5,0xe5,0x25,0xd8,0xe6,0x25,0xd8,0xf8,0xbb,0xe5,0x26,0x99,
0xed,0xe6,0x26,0x99,0xee,0xdc,0x49,0x00,0xdd,0x57,0x00,0xf8,0xe7,0xe4,0x1e,0x8a,
0x08,0x0d,0x09,0x0d,0xf9,0x0d,0x04,0xe4,0x1e,0x80,0xf8,0xbb,0xe4,0x1e,0x94,0xf7,
0xb3,0xe4,0x1e,0xa3,0x0a,0x00,0xf9,0x03,0x3a,0xe0,0xf9,0x09,0x86,0xe4,0x1e,0xb7,
0xf8,0xaf,0xe4,0x1e,0x99,0xf7,0xc9,0xe4,0x1e,0x91,0x08,0x00,0x09,0x00,0xf9,0x05,
0xc8,0xe4,0x1e,0x73,0xe5,0x26,0xa2,0xe6,0x26,0xa2,0xf7,0xbc,0xe4,0x1e,0x6c,0xe5,

20,1,
0xf9,0x02,0x08,0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,0x0d,0x01,0xf9,0x14,0x98,
0x08,0x00,0x09,0x00,0xf7,0xe6,0xe8,0x13,0xec,0xe9,0x26,0x99,0xdc,0xae,0x00,0xdd,
0x57,0x00,0xf7,0xdd,0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,0xf9,0x12,0xc0,0xe6,
0x1b,0xf2,0xf7,0xab,0xe6,0x1b,0xed,0xf8,0xbb,0xe6,0x1b,0xec,0x08,0x00,0x09,0x00,
0xf7,0xa2,0xe3,0xe4,0x13,0xec,0xe5,0x26,0x99,0xe6,0x1b,0xea,0xfd,0x98,0x48,0x02,
0x4c,0x07,0xe5,0x25,0xe1,0xe6,0x1d,0x3c,0xdc,0xc3,0x00,0xdd,0x61,0x00,0xde,0xdc,
0x00,0xef,0xf9,0x02,0x08,0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,0x0d,0x01,0xf9,
0x14,0x69,0x08,0x00,0x09,0x00,0xf7,0xea,0xe0,0xe1,0xfd,0x81,0x48,0x0a,0x4c,0x0f,
0xe4,0x13,0xec,0xec,0xe5,0x25,0x3c,0xed,0xdc,0xdc,0x00,0xdd,0x6e,0x00,0xf8,0x99,
0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,0xf9,0x13,0x2a,0xe6,0x1d,0x43,0xf7,0xb4,
0xe6,0x1d,0x3d,0x08,0x00,0x09,0x00,0xf9,0x02,0x0c,0xe0,0xe1,0xfd,0x81,0xe4,0x13,
0xec,0xec,0xe5,0x24,0xf1,0xed,0xdc,0xe9,0x00,0xdd,0x74,0x00,0xde,0xdc,0x00,0xf7,
0xac,0xe6,0x1d,0x3a,0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,0xf9,0x11,0xcc,0xe6,
0x1d,0x34,0xf7,0xad,0xe6,0x1d,0x3a,0xf8,0xbb,0xe6,0x1d,0x4b,0x08,0x00,0x09,0x00,
0xf7,0xef,0xe3,0xfd,0x78,0xe4,0x13,0xec,0xec,0xe5,0x24,0x67,0xed,0xdc,0x05,0x01,
0xdd,0x82,0x00,0xf7,0xb8,0xe6,0x1d,0x45,0x08,0x10,0x0d,0x01,0x09,0x10,0x0d,0x01,

40,1,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};



int main(int ac,char *av[])
{
	if(ac<4)
	{
		std::cout << "Test Raw to D77" << std::endl;
		std::cout << "Usage:" << std::endl;
		std::cout << "  testRaw2d77 input-file.raw correct.d77 working-directory" << std::endl;
	}

	std::string inputRAWFileName,correctD77FileName,workDir;
	inputRAWFileName=av[1];
	correctD77FileName=av[2];
	workDir=av[3];

	return 1;
}
