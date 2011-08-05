#include "precompiled.h"

#include "UnicodeConverter.h"
#include <cassert>
#include "../util/Debug_MemoryManager.h"

#ifdef WIN32
#include <windows.h>
#include <boost/cstdint.hpp>

using namespace boost;

namespace util {
namespace {

	uint32_t CODE_PAGE = CP_ACP;
	//uint32_t CODE_PAGE = CP_OEMCP;

	int getLength(const char *source, int bytes)
	{
		if(bytes == 0)
			return 0;

		return MultiByteToWideChar(CODE_PAGE | CP_UTF8, 0, source, bytes, 0, 0);
	}

	void convert(const char *source, int sourceBytes, wchar_t *destination, int destinationBytes)
	{
		if(sourceBytes > 0)
		MultiByteToWideChar(CODE_PAGE | CP_UTF8, 0, source, sourceBytes, destination, destinationBytes);
	}

	// --

	int getLength(const wchar_t *source, int bytes)
	{
		return WideCharToMultiByte(CODE_PAGE | CP_UTF8, 0, source, bytes, 0, 0, 0, 0);
	}

	void convert(const wchar_t *source, int sourceBytes, char *destination, int destinationBytes)
	{
		WideCharToMultiByte(CODE_PAGE | CP_UTF8, 0, source, sourceBytes, destination, destinationBytes, 0, 0);
	}

} //unnamed

void convertToWide(const std::string &source, std::wstring &destination)
{
	int length = getLength(&source[0], source.size());
	destination.resize(length);

	convert(&source[0], source.size(), &destination[0], length);
}

void convertToWide(const char *source, std::wstring &destination)
{
	assert(source);

	int length = getLength(source, strlen(source));
	destination.resize(length);

	convert(source, strlen(source), &destination[0], length);
}

wchar_t *convertToWide(const char *source)
{
	assert(source);

	int length = getLength(source, strlen(source) + 1);
	wchar_t *destination = new wchar_t[length];
	
	convert(source, strlen(source) + 1, destination, length);
	return destination;
}

// ---

void convertToMultiByte(const std::wstring &source, std::string &destination)
{
	int length = getLength(&source[0], source.size());
	destination.resize(length);

	convert(&source[0], source.size(), &destination[0], length);
}

void convertToMultiByte(const wchar_t *source, std::string &destination)
{
	assert(source);

	int length = getLength(source, wcslen(source));
	destination.resize(length);

	convert(source, wcslen(source), &destination[0], length);
}

char *convertToMultiByte(const wchar_t *source)
{
	assert(source);

	int length = getLength(source, wcslen(source) + 1);
	char *destination = new char[length];
	
	convert(source, wcslen(source) + 1, destination, length);
	return destination;
}

} // util

#else

#include <wchar.h>
#include <errno.h>
#include <boost/cstdint.hpp>

#include "igios.h"

namespace util {
namespace {
/*
	size_t convert(const char *source, int sourceBytes, wchar_t *destination)
	{
		mbstate_t state;
		memset(&state, '\0', sizeof (state));
		return mbsrtowcs (destination, source, sourceBytes, %state);
	}

	size_t convert(const wchar_t *source, int sourceBytes, char *destination)
	{
		mbstate_t state;
		memset(&state, '\0', sizeof (state));
		return wcsrtombs(destination, source, sourceBytes, &state);
	}
*/
} //unnamed

void convertToWide(const std::string &source, std::wstring &destination)
{
	igios_unimplemented();
	/*
	int length = getLength(&source[0], source.size());
	destination.resize(length);

	convert(&source[0], source.size(), &destination[0], length);
	*/
}

void convertToWide(const char *source, std::wstring &destination)
{
	assert(source);
	/*
	mbstate_t state;
	int length = 256;
	while (true) {
		memset(&state, 0, sizeof(mbstate_t));
		wchar_t *buf = new wchar_t[length];
		int actualSize = mbsrtowcs(buf, &source, length, &state);
		if (actualSize < length) { // success
			destination.resize(actualSize);
			destination.replace(0, actualSize, buf);
			delete[] buf;
			break;
		}
		delete[] buf; // insufficient space
		length *= 2;
	}
	*/

	unsigned int len = strlen(source);
	destination.resize(len);
	for (unsigned int i = 0; i < len; i++) {
		destination[i] = 0;
	}

	unsigned int srcPos = 0;
	unsigned int dstPos = 0;
	while (srcPos < len && dstPos < len) {
		unsigned char c = source[srcPos];
		srcPos++;

		// http://en.wikipedia.org/wiki/UTF-8
		if (c & 0x80) {
			if ((c & 0xF0) == 0xE0)
			{
				// 4 highest bits == 1110
				// means 3-byte encoding
				unsigned char c2 = source[srcPos];
				srcPos++;

				unsigned char c3 = source[srcPos];
				srcPos++;

				wchar_t wide = 0;
				// 0x0F = 4 lowest bits set
				wide |= (c & 0x0F) << 12;
				// 0x3f = 6 lowest bits set
				wide |= (c2 & 0x3f) << 6;
				wide |= c3 & 0x3f;
			} else {
				// assume 2-byte encoding
				wchar_t wide = 0;

				// 0x1f = 5 lowest bits set
				wide |= (c & 0x1f) << 6;

				unsigned char c2 = source[srcPos];
				srcPos++;

				// 0x3f = 6 lowest bits set
				wide |= c2 & 0x3f;

				destination[dstPos] = wide;
				dstPos++;
			}
		} else {
			destination[dstPos] = c;
			dstPos++;
		}
	}
}

wchar_t *convertToWide(const char *source)
{
	igios_unimplemented();
	/*
	assert(source);

	int length = getLength(source, strlen(source) + 1);
	wchar_t *destination = new wchar_t[length];
	
	convert(source, strlen(source) + 1, destination, length);
	return destination;
	*/

	return NULL;
}

// ---

void convertToMultiByte(const std::wstring &source, std::string &destination)
{
	unsigned int dstlen = source.length();

	// calculate destination size
	for (unsigned int i = 0; i < source.length(); i++)
	{
		if ((source[i] & 0xF800) != 0) {
			dstlen += 2;
		} else if (source[i] & 0x80) {
			// char that decodes to multibyte sequence
			// assume it takes 2 bytes
			dstlen++;
		}
	}

	destination.resize(dstlen);

	unsigned int dstPos = 0;
	for (unsigned int i = 0; i < source.length(); i++)
	{
		// 0xF8 = 5 highest bits set
		if ((source[i] & 0xF800) != 0) {
			// 3-byte encoding
			uint8_t c1 = 0xE0 | ((source[i] >> 12) & 0x0F);
			// 0x3f = 6 lowest bits set
			uint8_t c2 = 0x80 | ((source[i] >> 6)  & 0x3F);
			uint8_t c3 = 0x80 | (source[i]         & 0x3F);

			destination[dstPos] = c1;
			dstPos++;

			destination[dstPos] = c2;
			dstPos++;

			destination[dstPos] = c3;
			dstPos++;
		} else if (source[i] & 0x80) {
			uint8_t first = ((source[i] >> 6) & 0x1F) | 0xC0;
			uint8_t second = ((source[i] & 0x3f)) | 0x80;

			destination[dstPos] = first;
			dstPos++;

			destination[dstPos] = second;
			dstPos++;
		} else {
			destination[dstPos] = source[i] & 0x7F;
			dstPos++;
		}
	}
}

void convertToMultiByte(const wchar_t *source, std::string &destination)
{
	igios_unimplemented();
	/*
	assert(source);

	int length = getLength(source, wcslen(source));
	destination.resize(length);

	convert(source, wcslen(source), &destination[0], length);
	*/
}

char *convertToMultiByte(const wchar_t *source)
{
	igios_unimplemented();
	/*
	assert(source);

	int length = getLength(source, wcslen(source) + 1);
	char *destination = new char[length];
	
	convert(source, wcslen(source) + 1, destination, length);
	return destination;
	*/

	return NULL;
}

} // util
#endif
