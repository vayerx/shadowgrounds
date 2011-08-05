
#include "precompiled.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rle_packed_file_wrapper.h"
#include "../util/jpak.h"

#define USE_FB_INPUT_STREAM_WRAPPER

#ifdef USE_FB_INPUT_STREAM_WRAPPER
#include "input_stream_wrapper.h"
using namespace frozenbyte::filesystem;
#endif


static const int RLE_PACKED_CHUNK_MAGIC = *((int *)"RLE1");


struct RLE_PACKED_FILE
{
#ifdef USE_FB_INPUT_STREAM_WRAPPER
	FB_FILE *inputFile;
#else
	FILE *inputFile;
#endif
	FILE *outputFile;
	bool errorFlag;
	bool inputIsRLE;


	RLE_PACKED_FILE()
		: inputFile(NULL),
		outputFile(NULL),
		errorFlag(false),
		inputIsRLE(false)
	{
	}

	~RLE_PACKED_FILE()
	{
		assert(inputFile == NULL);
		assert(outputFile == NULL);
	}
};


bool rle_packed_detect(const char *filename)
{
	bool wasRLE = false;

#ifdef USE_FB_INPUT_STREAM_WRAPPER
	FB_FILE *inputFile = fb_fopen(filename, "rb");
#else
	FILE *inputFile = fopen(filename, "rb");
#endif
	if (inputFile != NULL)
	{
		int chunkId = 0;
#ifdef USE_FB_INPUT_STREAM_WRAPPER
		int got = fb_fread(&chunkId, sizeof(int), 1, inputFile);
#else
		int got = fread(&chunkId, sizeof(int), 1, inputFile);
#endif
		if (got == 1
			&& chunkId == RLE_PACKED_CHUNK_MAGIC)
		{
			wasRLE = true;
		}

#ifdef USE_FB_INPUT_STREAM_WRAPPER
		fb_fclose(inputFile);
#else
		fclose(inputFile);
#endif
	}
	return wasRLE;
}


RLE_PACKED_FILE *rle_packed_fopen(const char *filename, const char *params)
{
	assert(params != NULL);
	if (params == NULL)
		return NULL;

	if (strcmp(params, "rb") == 0)
	{
		bool rle = false;
		if (rle_packed_detect(filename))
			rle = true;

#ifdef USE_FB_INPUT_STREAM_WRAPPER
		FB_FILE *inputFile = fb_fopen(filename, params);
#else
		FILE *inputFile = fopen(filename, params);
#endif
		if (inputFile != NULL)
		{
			RLE_PACKED_FILE *f = new RLE_PACKED_FILE();
			f->inputFile = inputFile;
			f->inputIsRLE = rle;
			return f;
		} else {
			return NULL;
		}
	}
	else if (strcmp(params, "wb") == 0)
	{
		FILE *outputFile = fopen(filename, params);
		if (outputFile != NULL)
		{
			RLE_PACKED_FILE *f = new RLE_PACKED_FILE();
			f->outputFile = outputFile;
			return f;
		} else {
			return NULL;
		}
	}

	assert(!"rle_packed_fopen - unsupported open params.");
	return NULL;
}


size_t rle_packed_fread(void *buffer, size_t size, size_t count, RLE_PACKED_FILE *stream)
{
	assert(stream != NULL);
	assert(stream->inputFile != NULL);

	if (!stream->inputIsRLE)
	{
#ifdef USE_FB_INPUT_STREAM_WRAPPER
		size_t ret = fb_fread(buffer, size, count, stream->inputFile);
#else
		size_t ret = fread(buffer, size, count, stream->inputFile);
#endif
		if (ret <= 0)
			stream->errorFlag = true;
		return ret;
	}

	int tmpgot = 0;
	int chunkId = 0;
	char isPacked = 0;
	size_t packedSize = 0;
	size_t unpackedSize = 0;

#ifdef USE_FB_INPUT_STREAM_WRAPPER
	tmpgot = fb_fread(&chunkId, sizeof(int), 1, stream->inputFile);
#else
	tmpgot = fread(&chunkId, sizeof(int), 1, stream->inputFile);
#endif
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fread - no more data available (expected chunk id, but got nothing).");
		stream->errorFlag = true;
		return 0;
	}
	if (chunkId != RLE_PACKED_CHUNK_MAGIC)
	{
		assert(!"rle_packed_fread - invalid data (expected chunk id, but got something else).");
		stream->errorFlag = true;
		return 0;
	}
#ifdef USE_FB_INPUT_STREAM_WRAPPER
	tmpgot = fb_fread(&isPacked, sizeof(char), 1, stream->inputFile);
#else
	tmpgot = fread(&isPacked, sizeof(char), 1, stream->inputFile);
#endif
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fread - truncated chunk header (expected packed flag). ");
		stream->errorFlag = true;
		return 0;
	}
	if (isPacked != 0 && isPacked != 1)
	{
		assert(!"rle_packed_fread - invalid data (expected packed flag, but got something else). ");
		stream->errorFlag = true;
		return 0;
	}
#ifdef USE_FB_INPUT_STREAM_WRAPPER
	tmpgot = fb_fread(&packedSize, sizeof(int), 1, stream->inputFile);
#else
	tmpgot = fread(&packedSize, sizeof(int), 1, stream->inputFile);
#endif
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fread - truncated chunk header (expected packed size). ");
		stream->errorFlag = true;
		return 0;
	}
	if (packedSize <= 0)
	{
		assert(!"rle_packed_fread - invalid data (packed size zero or negative). ");
		stream->errorFlag = true;
		return 0;
	}
	if (packedSize > count * size)
	{
		assert(!"rle_packed_fread - invalid data (size of packed chunk is larger than requested data).");
		stream->errorFlag = true;
		return 0;
	}
#ifdef USE_FB_INPUT_STREAM_WRAPPER
	tmpgot = fb_fread(&unpackedSize, sizeof(int), 1, stream->inputFile);
#else
	tmpgot = fread(&unpackedSize, sizeof(int), 1, stream->inputFile);
#endif
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fread - truncated chunk header (expected unpacked size). ");
		stream->errorFlag = true;
		return 0;
	}
	if (unpackedSize <= 0)
	{
		assert(!"rle_packed_fread - invalid data (unpacked size zero or negative). ");
		stream->errorFlag = true;
		return 0;
	}
	if (unpackedSize != count * size)
	{
		assert(!"rle_packed_fread - invalid data (size of unpacked chunk does not equal requested size).");
		stream->errorFlag = true;
		return 0;
	}
	if (!isPacked && packedSize != unpackedSize)
	{
		assert(!"rle_packed_fread - invalid data (supposed to be unpacked chunk, but packed size and unpacked size mismatch).");
		stream->errorFlag = true;
		return 0;
	}

	unsigned char *packedbuf = new unsigned char[packedSize + 16];

#ifdef USE_FB_INPUT_STREAM_WRAPPER
	int got = fb_fread(packedbuf, packedSize, 1, stream->inputFile);
#else
	int got = fread(packedbuf, packedSize, 1, stream->inputFile);
#endif
	if (got == 1)
	{
		if (isPacked)
		{
			jpak_set_bits(16);
			jpak_set_16bit_params(0xA0EA, 0xA0EB, 0xA0EC);
			size_t resultSize = jpak_unpack(packedSize, packedbuf, (unsigned char *)buffer, unpackedSize);
			if(resultSize != unpackedSize)
			{
				assert(!"rle_packed_fread - invalid data (chunk header unpacked size did not match actual resulting unpacked size).");
				stream->errorFlag = true;
				delete [] packedbuf;
				return 0;
			}
		} else {
			memcpy(buffer, packedbuf, packedSize);
		}
		delete [] packedbuf;
		return count;
	} else {
		delete [] packedbuf;
		return 0;
	}

}


size_t rle_packed_fwrite(void *buffer, size_t size, size_t count, RLE_PACKED_FILE *stream)
{
	assert(stream != NULL);
	assert(stream->outputFile != NULL);
	assert(size > 0 && count > 0);

	int unpackedSize = (int)size * (int)count;
	assert(unpackedSize > 0);

	unsigned char *packedbuf = new unsigned char[unpackedSize + 16];

	jpak_set_bits(16);
	jpak_set_16bit_params(0xA0EA, 0xA0EB, 0xA0EC);
	int packedSize = jpak_pack(unpackedSize, (unsigned char *)buffer, packedbuf);

	char isPacked = 1;
	if (packedSize == 0)
	{
		isPacked = 0;
		packedSize = unpackedSize;
		memcpy(packedbuf, buffer, packedSize);
	}

	int tmpgot = 0;
	
	tmpgot = fwrite(&RLE_PACKED_CHUNK_MAGIC, sizeof(int), 1, stream->outputFile);
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fwrite - write failed (chunk id).");
		stream->errorFlag = true;
		delete [] packedbuf;
		return 0;
	}

	tmpgot = fwrite(&isPacked, sizeof(char), 1, stream->outputFile);
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fwrite - write failed (chunk packed flag).");
		stream->errorFlag = true;
		delete [] packedbuf;
		return 0;
	}

	tmpgot = fwrite(&packedSize, sizeof(int), 1, stream->outputFile);
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fwrite - write failed (chunk packed size).");
		stream->errorFlag = true;
		delete [] packedbuf;
		return 0;
	}

	tmpgot = fwrite(&unpackedSize, sizeof(int), 1, stream->outputFile);
	if (tmpgot != 1)
	{
		assert(!"rle_packed_fwrite - write failed (chunk unpacked size).");
		stream->errorFlag = true;
		delete [] packedbuf;
		return 0;
	}

	int got = fwrite(packedbuf, packedSize, 1, stream->outputFile);
	if (got != 1)
	{
		assert(!"rle_packed_fwrite - write failed (packed data).");
		stream->errorFlag = true;
		delete [] packedbuf;
		return 0;
	}

	delete [] packedbuf;

	return count;
}


long rle_packed_fsize(RLE_PACKED_FILE *stream)
{
	assert(stream != NULL);
	assert(stream->inputFile != NULL && stream->outputFile == NULL);

	if (stream->inputFile != NULL)
	{
#ifdef USE_FB_INPUT_STREAM_WRAPPER
		return fb_fsize(stream->inputFile);
#else
		fpos_t pos;
		if (fgetpos(stream->inputFile, &pos) != 0)
		{
			assert(!"fgetpos failed.");
			stream->errorFlag = true;
			return -1;
		}

		fseek(stream->inputFile, 0, SEEK_END);
		long tmp = ftell(stream->inputFile);

		if(fsetpos(stream->inputFile, &pos) != 0)
		{
			assert(!"fsetpos failed.");
			stream->errorFlag = true;
			return -1;
		}

		return tmp;
#endif
	}	else {
		return 0;
	}
}


int rle_packed_fclose(RLE_PACKED_FILE *stream)
{
	assert(stream != NULL);
	assert(stream->inputFile != NULL || stream->outputFile != NULL);

	int result = 0;
	if (stream->inputFile != NULL)
	{
#ifdef USE_FB_INPUT_STREAM_WRAPPER
		result = fb_fclose(stream->inputFile);
#else
		result = fclose(stream->inputFile);
#endif
		stream->inputFile = NULL;
	}
	if (stream->outputFile != NULL)
	{
		fclose(stream->outputFile);
		stream->outputFile = NULL;
	}
	return result;
}


bool rle_packed_was_error(RLE_PACKED_FILE *stream)
{
	assert(stream != NULL);
	return stream->errorFlag;
}
