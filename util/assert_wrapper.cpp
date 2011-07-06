
#include "precompiled.h"

#undef NDEBUG

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "assert.h"
#include "../system/Logger.h"
#include "../editor/string_conversions.h"

#include <string>
#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

namespace frozenbyte {
#ifdef WIN32
namespace {

	void removeMessages()
	{
		MSG msg = { 0 };
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_PAINT)
				return;
		}
	}

} // unnamed
#endif

void assertImp(const char *predicateString, const char *file, int line)
{
	string error = predicateString;
	error += " (";
	error += file;
	error += ", ";
	error += editor::convertToString(line);
	error += ")";

	LOG_ERROR(error.c_str());
#ifdef WIN32
	removeMessages();
#endif

	// can't use the actual assert clause here, because it
	// has been lost in the function call (a macro assert would be
	// the only possible solution, but that would cause problems
	// with the #undef NDEBUG in the header...)
#ifdef FB_TESTBUILD
	assert(!"testbuild fb_assert encountered.");
#endif
}

} // frozenbyte
