#include "precompiled.h"

#include <stdio.h>
#include "crc32.h"

unsigned int crc32_tab[256];

void crc32_gentab()
{
	static bool generated = false;
	if (generated) return;

	unsigned int crc, poly;
	poly = 0xEDB88320;
	for (int i = 0; i < 256; ++i) {
		crc = i;
		for (int j = 8; j > 0; j--) {
			if (crc & 1) crc = (crc >> 1) ^ poly;
			else crc >>= 1;
		}
		crc32_tab[i] = crc;
	}
	generated = true;
}

unsigned int crc32_file(const char *filename)
{
	crc32_gentab();

	FILE *file;
	if ((file = fopen(filename, "rb")) != 0) {
		unsigned char buffer[1024];
		unsigned int read = 0;
		unsigned int crc = 0xFFFFFFFF;
		while ((read = fread(buffer, 1, 1024, file)) != 0) {
			for (unsigned int i = 0; i < read; ++i) {
				crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32_tab[(crc ^ buffer[i]) & 0xFF];
			}
		}
		fclose(file);
		return crc ^ 0xFFFFFFFF;
	}
	return 0;
}

#ifdef STANDALONE

#include <stdlib.h>

void usage(char *bin)
{
	printf("usage: %s files\n", bin);
	printf("files: file to calculate crc from\n");
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc < 2) usage(argv[0]);
	for (unsigned int i = 1; i < argc; ++i) {
		printf("calculating crc for %s ...", argv[i]);
		fflush(stdout);
		unsigned int crc = crc32_file(argv[i]);
		printf(" done, crc = 0x%08X\n", crc);
	}
}

#endif
