#ifndef INCLUDED_EDITOR_IDLG_HANDLER_H
#define INCLUDED_EDITOR_IDLG_HANDLER_H

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

namespace frozenbyte {
namespace editor {

class IDlgHandler 
{
public:
	virtual BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)=0;
	virtual ~IDlgHandler() {}
};

}
}

#endif
