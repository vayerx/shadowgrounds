#include "precompiled.h"

#include "UnicodeConverter.h"
#include <cassert>

#ifdef WIN32
#include <windows.h>
namespace util {
namespace {

	DWORD CODE_PAGE = CP_ACP;
	//DWORD CODE_PAGE = CP_OEMCP;

	int getLength(const char *source, int bytes)
	{
		return MultiByteToWideChar(CODE_PAGE | CP_UTF8, 0, source, bytes, 0, 0);
	}

	void convert(const char *source, int sourceBytes, wchar_t *destination, int destinationBytes)
	{
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
#include "igios.h"
#include <errno.h>

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

	// HACK, assumes wchar_t is at least 16bits wide
	typedef struct {
		union {
			unsigned char hi;
			unsigned char lo;
		};
		wchar_t wc;
	} conv;
	unsigned int len = strlen(source);
	destination.resize(len);
	for (unsigned int i = 0; i < len; ++i) {
		unsigned char c = source[i];
		// http://en.wikipedia.org/wiki/UTF-8
		if (c & 0x80) { // extended ascii?
			conv dst;
			dst.wc = 0; // stop gcc from whining
			dst.hi = ((c >> 6) & 0x1f) | 0x60; // higher unicode char 110xxxxx, upper two bits to xxxxx
			dst.lo = (c & 0x3f) | 0x80; // lower unicode 10xxxxxx, lower six bits to xxxxxx
			destination[i] = dst.wc;
		} else destination[i] = c;
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
	igios_unimplemented();
	/*
	int length = getLength(&source[0], source.size());
	destination.resize(length);

	convert(&source[0], source.size(), &destination[0], length);
	*/
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
