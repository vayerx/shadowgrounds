#ifndef INC_IDLGHANDLER_H
#define INC_IDLGHANDLER_H

#include <windows.h>

namespace frozenbyte {
namespace launcher {

class IDlgHandler 
{
public:
	virtual BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)=0;
	virtual ~IDlgHandler() {}
};

} // end of namespace launcher
} // end of namespace frozenbyte

#endif