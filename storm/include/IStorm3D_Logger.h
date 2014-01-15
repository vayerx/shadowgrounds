// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_LOGGER_H
#define INCLUDED_ISTORM3D_LOGGER_H

#pragma once

class IStorm3D_Logger {
public:
    virtual ~IStorm3D_Logger() { }
    virtual void debug(const char *msg) = 0;
    virtual void info(const char *msg) = 0;
    virtual void warning(const char *msg) = 0;
    virtual void error(const char *msg) = 0;
    // these all work just like printf
    // format argument index is 2 due to implicit this
    virtual void debug2(const char *fmt, ...) __attribute__ ((format (printf, 2, 3))) = 0;
    virtual void info2(const char *fmt, ...) __attribute__ ((format (printf, 2, 3))) = 0;
    virtual void warning2(const char *fmt, ...) __attribute__ ((format (printf, 2, 3))) = 0;
    virtual void error2(const char *fmt, ...) __attribute__ ((format (printf, 2, 3))) = 0;
};

#endif
