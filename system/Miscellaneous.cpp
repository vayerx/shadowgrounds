// miscellaneous system-specific helper functions...

#include "precompiled.h"

#include <stdarg.h>
#include <stdlib.h>

#include <boost/scoped_array.hpp>

#ifndef _WIN32
#  include <SDL.h>
#endif

#include <GL/glew.h>
#include "igios.h"

// FIXME: move this thing somewhere else
// FIXME: need to switch out from fullscreen mode and release input if grabbed

#ifdef _WIN32
#  include <windows.h>

void sysFatalError(std::string msg)
{
    MessageBox(0, msg.c_str(), "Fatal error", MB_OK | MB_ICONERROR);
    exit(1);
}

#else  // _WIN32
#  include <vector>
#  ifndef __APPLE__
#    include <gtk/gtk.h>
#  endif

// assume linux
// FIXME: not right, we have consoles etc too

static std::vector<char *> storedArgv;

void sysSetArgs(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        storedArgv.push_back(argv[i]);
    }
}

void sysFatalError(std::string msg)
{
    fprintf( stderr, "Fatal error: %s\n", msg.c_str() );

    // FIXME: which version should we use?

    // let go of fullscreen
    // otherwise game will just seem to hang
    SDL_Quit();

    int tempArgc = storedArgv.size();
    // gtk_init wants to change this, need to make copy
    boost::scoped_array<char *> tempArgv(new char *[tempArgc]);
    for (int i = 0; i < tempArgc; i++) {
        tempArgv[i] = storedArgv[i];
    }
    char **tempArgv_ = tempArgv.get();

#  ifndef __APPLE__
    gtk_init(&tempArgc, &tempArgv_);
    GtkWidget *dialog = gtk_message_dialog_new( NULL,
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                "%s", msg.c_str() );
    gtk_window_set_title(GTK_WINDOW(dialog), "Fatal error");
    gtk_dialog_run( GTK_DIALOG(dialog) );
    gtk_widget_destroy(dialog);
#  endif

    abort();
}

#endif  // _WIN32

#if defined(_MSC_VER) && !defined(FINAL_RELEASE)

void igiosWarning(const char *fmt, ...)
{
    va_list argptr;

    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
}

#endif  // defined(_MSC_VER) && !defined(FINAL_RELEASE)

#ifdef __GLIBC__

#  ifndef __USE_GNU
#    define __USE_GNU
#  endif

#  include <execinfo.h>
#  include <ucontext.h>
#  include <unistd.h>

#  define BACKTRACELEN 32

//! Dump stack trace to stderr
void igios_backtrace(void) {
#  ifndef FINAL_RELEASE_BUILD

    void *frames[BACKTRACELEN];

    int num = backtrace(frames, BACKTRACELEN);
    backtrace_symbols_fd(frames, num, STDERR_FILENO);
    igiosWarning("\n");

#  endif // FINAL_RELEASE_BUILD

}

#  if 0
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

#  endif // 0
#endif   // __GLIBC__

std::string strPrintf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

#ifdef __GLIBC__
    // glibc has a sane functions for this
    char *buf = NULL;
    int printedChars = vasprintf(&buf, fmt, args);
    if (printedChars < 0) {
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

    std::string retString( buf.get() );

#endif  // __GLIBC__

    va_end(args);

    return retString;
}

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
            igiosWarning( "gl error at %s:%d: %x %s\n", file, line, err, gluErrorString(err) );
            igios_backtrace();
            return true;
        }
    }

    return false;
}
