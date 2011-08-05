#include "precompiled.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <GL/glew.h>

#include "igios.h"

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


void igios_setsighandler(void) {
	// disable this, it's more trouble than it's worth
	// just use core dumps
	return;
#if 0
#ifdef __GLIBC__
	struct sigaction sa;

	sa.sa_sigaction = igios_sighandler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
#endif
#endif

}

std::string igios_getFontDirectory() {
#ifdef WIN32
	TCHAR temp[MAX_PATH];
	GetWindowsDirectory(temp, MAX_PATH);
	std::string dir(temp);
	dir.append("\\fonts\\");
	return dir;
#else
	return "";
#endif
}

bool gl_initialized = false;

bool igiosCheckGlErrors(const char *file, int line) {
	if (gl_initialized) {
		int err = glGetError();
		if (err != GL_NO_ERROR) {
			igiosWarning("gl error at %s:%d: %x %s\n", file, line, err, gluErrorString(err));
			igios_backtrace();
			return true;
		}
	}

	return false;
}

#ifdef _MSC_VER

void igiosWarning(const char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	vfprintf (stderr, fmt, argptr);
	va_end(argptr);
}

#endif

#ifdef __GNUC__

extern "C" {
	void __cyg_profile_func_enter(void *func, void *caller) __attribute__((no_instrument_function));

	void __cyg_profile_func_enter(void *func, void *caller) {
		if (glErrors()) {
			igiosWarning("while entering function %p called at %p\n", func, caller);
			igios_backtrace();
		}
	}

	void __cyg_profile_func_exit(void *func, void *caller) __attribute__((no_instrument_function));

	void __cyg_profile_func_exit(void *func, void *caller) {
		if (glErrors()) {
			igiosWarning("while exiting function %p called at %p\n", func, caller);
			igios_backtrace();
		}
	}

}
#endif
