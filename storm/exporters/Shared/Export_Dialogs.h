// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_DIALOGS_H
#define INCLUDED_DIALOGS_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#pragma warning(disable: 4710) // function not inlined
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

namespace frozenbyte {
namespace exporter {

class Exporter;

// Creates dialogs and saves data as needed
void createExportDialog(Exporter *exporter, HINSTANCE moduleHandle);

} // end of namespace export
} // end of namespace frozenbyte

#endif
