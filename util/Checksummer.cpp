
#include "precompiled.h"

#include "Checksummer.h"
#include "../filesystem/input_stream_wrapper.h"

#include <assert.h>
#include <stdio.h>

using namespace frozenbyte;

namespace util
{

	unsigned int Checksummer::countChecksumForFile(const char *filename)
	{
		unsigned int chksum = 0;
		int filesize = 0;
		bool success = countChecksumForFileImpl(&chksum, &filesize, filename);
		if (success)
		{
			return chksum;
		} else {
			assert(0);
			return 0;
		}
	}

	bool Checksummer::doesChecksumAndSizeMatchFile(unsigned int checksum, int filesize, const char *filename)
	{
		unsigned int chksum = 0;
		int size = 0;
		bool success = countChecksumForFileImpl(&chksum, &size, filename);
		if (success)
		{
			if (chksum == checksum && filesize == size)
				return true;
			else
				return false;
		} else {
			return false;
		}
	}


	bool Checksummer::countChecksumForFileImpl(unsigned int *checksum, int *filesize, const char *filename)
	{
		assert(filename != NULL);

		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		if (f == NULL)
		{
			return false;
		}

		//fseek(f, 0, SEEK_END);
		//int size = ftell(f);
		//fseek(f, 0, SEEK_SET);
		int size = filesystem::fb_fsize(f);

		char *buf = new char[size];

		bool success = true;
		int got = filesystem::fb_fread(buf, size, 1, f);
		if (got != 1)
		{
			success = false;
		} else {
			success = true;
			*filesize = size;
			unsigned int hashCode = 1327341033;
			int hashmult = 0;
			for (int i = 0; i < size; i++)
			{
				if ((i % 73) == 0)
					hashCode += (buf[i] << hashmult);
				else
					hashCode ^= (buf[i] << hashmult);
				hashmult+=4;
				if (hashmult > 23) hashmult -= 23;
			}
			*checksum = hashCode;
		}
		
		delete [] buf;

		filesystem::fb_fclose(f);

		return success;
	}	

}


