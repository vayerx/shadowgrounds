// miscellaneous system-specific helper functions...

#include "precompiled.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>

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


#include <execinfo.h>
#include <cxxabi.h>
#include <ucontext.h>
#include <unistd.h>

#define BACKTRACELEN 32

//! Dump stack trace to stderr
void igios_backtrace() {
   igios_backtrace(stderr);
}

void igios_backtrace(FILE* out) {
#if !FINAL_RELEASE_BUILD

    unsigned int max_frames = 63;

    //https://idlebox.net/2008/0901-stacktrace-demangled/
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
        fprintf(out, "  <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset
            && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name,
                                            funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                fprintf(out, "  %s : %s+%s\n",
                        symbollist[i], funcname, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                fprintf(out, "  %s : %s()+%s\n",
                        symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            fprintf(out, "  %s\n", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
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
