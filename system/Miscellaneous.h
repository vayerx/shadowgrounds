#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

#include <string.h>

#ifndef __GNUC__
// non-gcc compilers don't like attribute
#  ifndef __attribute__
#    define __attribute__(x)
#  endif // __attribute__
#endif   // __GNUC__

void sysSetArgs(int argc, char *argv[]);

// something has gone horribly wrong
// show error msg and exit
void sysFatalError(std::string msg) __attribute__( (noreturn) );

// print stuff into a string
std::string strPrintf(const char *fmt, ...) __attribute__( ( format(printf, 1, 2) ) );

#if defined(_WIN64)

#  define FMT_SIZE    "%I64u"
#  define FMT_U64     "%I64u"
#  define FMT_X64     "%I64x"
#  define FMT_INTPTR  "%I64d"
#  define FMT_UINTPTR "%I64u"

#elif defined(__x86_64) || defined(__x86_64__)

#  define FMT_SIZE    "%lu"
#  define FMT_U64     "%lu"
#  define FMT_X64     "%lx"
#  define FMT_INTPTR  "%ld"
#  define FMT_UINTPTR "%lu"

#elif defined(_WIN32)

#  define FMT_SIZE    "%u"
#  define FMT_U64     "%I64u"
#  define FMT_X64     "%I64x"
#  define FMT_INTPTR  "%d"
#  define FMT_UINTPTR "%u"

#else  // assume i386

#  define FMT_SIZE    "%u"
#  define FMT_U64     "%llu"
#  define FMT_X64     "%llx"
#  define FMT_INTPTR  "%d"
#  define FMT_UINTPTR "%u"


#endif  // machine-specific printfs

#endif  // MISCELLANEOUS_H
