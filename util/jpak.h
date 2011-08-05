
#ifndef JPAK_H
#define JPAK_H

// A simple RLE packer
// Copyright (C) Jukka Kokkonen, 1999

// size is the size of the unpacked source buffer, destination buffer should be slightly overallocated to be safe
// (no predefined rule for the destination buffer overallocation, +16 bytes is nice ;)
// note, if using 16 bit packing, buffer sizes must be padded to 2 byte size (pad source with zeroes)
// returns 0 if packed data would have been larger than source, in which case you need to handle it differently
extern int jpak_pack(int size, unsigned const char *src, unsigned char *dest);

// size is the size of the packed source buffer, destination buffer should large enough to fit the unpacked data
// (size of unpacked data should written in the packfile or somewhere)
// maxDestSize gives the maximum allowed size for unpacked data or if 0, no limit (potential buffer overflow!)
extern int jpak_unpack(int size, unsigned const char *src, unsigned char *dest, int maxDestSize = 0);

// 8 or 16 bits supported
extern void jpak_set_bits(unsigned char bits);

extern void jpak_set_8bit_params(unsigned char mark1, unsigned char mark2, unsigned char mark3);
extern void jpak_set_16bit_params(unsigned short mark1, unsigned short mark2, unsigned short mark3);

#endif


