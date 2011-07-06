#include "precompiled.h"

#include "UnicodeConverter.h"
#include <windows.h>
#include <cassert>

namespace frozenbyte {
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
} // frozenbyte
