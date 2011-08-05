
// Copyright(C) Jukka Kokkonen, 2007

#ifndef RLE_PACKED_FILE_WRAPPER_H
#define RLE_PACKED_FILE_WRAPPER_H

// A silly read/write "chunk packed" file.
// (NOTE: therefore must read data in same sized chunks as it was written!)
// (NOTE: also, that reading/writing in really small chunks will just result in extremely large files!)

struct RLE_PACKED_FILE;

// return true if the file seems to be rle packed.
bool rle_packed_detect(const char *filename);

RLE_PACKED_FILE *rle_packed_fopen(const char *filename, const char *params);

size_t rle_packed_fread(void *buffer, size_t size, size_t count, RLE_PACKED_FILE *stream);

size_t rle_packed_fwrite(void *buffer, size_t size, size_t count, RLE_PACKED_FILE *stream);

long rle_packed_fsize(RLE_PACKED_FILE *stream);

bool rle_packed_was_error(RLE_PACKED_FILE *stream);

int rle_packed_fclose(RLE_PACKED_FILE *stream);

#endif


