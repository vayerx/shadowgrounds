// miscellaneous system-specific helper functions...

#include "precompiled.h"

#include <stdarg.h>

#include <boost/scoped_array.hpp>

#include "igios.h"


std::string strPrintf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);

#ifdef __GLIBC__
	// glibc has a sane functions for this
	char *buf = NULL;
	int printedChars = vasprintf(&buf, fmt, args);
	if (printedChars < 0)
	{
		// failed, usually because out of memory
		// is a fatal error
		// so fatal in fact that sysFatalError might also fail
		fprintf(stderr, "strPrintf failed\n");
		fprintf(stderr, "format string was :\"%s\"\n", fmt);
		exit(1);
	}

	std::string retString(buf);

	// must be free and NOT delete[]
	free(buf);

#else  // __GLIBC__
	// using some insane c lib, just use a large buffer and hope for the best
	// FIXME: put the buffer somewhere where we don't constantly reallocate it
	// NOT the stack
	// bad things might happen
	const unsigned int BUFFERSIZE = 4096;
	boost::scoped_array<char> buf(new char[BUFFERSIZE]);
	vsnprintf(buf.get(), BUFFERSIZE, fmt, args);

	std::string retString(buf.get());

#endif  // __GLIBC__

	va_end(args);

	return retString;
}

#if defined(_MSC_VER) && !defined(FINAL_RELEASE)

void igiosWarning(const char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	vfprintf (stderr, fmt, argptr);
	va_end(argptr);
}

#endif  // defined(_MSC_VER) && !defined(FINAL_RELEASE)

void igiosErrorMessage(const char *msg, ...) {
	va_list args;
	char buf[1024];

	va_start(args, msg);
	vsnprintf(buf, 1024, msg, args);
	va_end(args);
	igiosWarning("%s\n", buf);

#ifdef WIN32
	MessageBox(0, buf, "Error", MB_OK);
#endif

#ifdef __GLIBC__
	igios_backtrace();
#endif

}

#ifdef __GLIBC__

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <ucontext.h>
#include <unistd.h>

#define BACKTRACELEN 16

//! Dump stack trace to stderr
void igios_backtrace(void) {
	void *frames[BACKTRACELEN];

	int num = backtrace(frames, BACKTRACELEN);
	backtrace_symbols_fd(frames, num, STDERR_FILENO);
	igiosWarning("\n");
}

#if 0
void igios_sighandler(int sig, siginfo_t *info, void *secret) {
	void *frames[BACKTRACELEN];
	int num = 0;
	ucontext_t *uc = (ucontext_t *) secret;

	if (sig == SIGSEGV)
		printf("Got signal %d at %p from %p\n", sig, info->si_addr, (void *) uc->uc_mcontext.gregs[REG_EIP]);
	else
		printf("Got signal %d\n", sig);
	
	num = backtrace(frames, 16);

	frames[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

	backtrace_symbols_fd(frames, num, STDERR_FILENO);

	exit(0);
}

#endif
#endif


