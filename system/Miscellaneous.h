#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H


#include <string.h>

#include "igios.h"


void sysSetArgs(int argc, char *argv[]);

// something has gone horribly wrong
// show error msg and exit
void sysFatalError(std::string msg) __attribute__((noreturn));


// print stuff into a string
std::string strPrintf(const char *fmt, ...) __attribute__((format (printf, 1, 2) ));


#endif  // MISCELLANEOUS_H
