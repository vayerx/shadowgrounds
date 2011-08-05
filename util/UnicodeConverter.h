#ifndef UTIL_UNICODE_CONVERTER_H
#define UTIL_UNICODE_CONVERTER_H

#include <string>

namespace util {

void convertToWide(const std::string &source, std::wstring &destination);
void convertToWide(const char *source, std::wstring &destination);
wchar_t *convertToWide(const char *source);

void convertToMultiByte(const std::wstring &source, std::string &destination);
void convertToMultiByte(const wchar_t *source, std::string &destination);
char *convertToMultiByte(const wchar_t *source);

} // util

#endif
