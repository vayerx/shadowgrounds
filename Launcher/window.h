#ifndef INC_WINDOW_H
#define INC_WINDOW_H

#include <string>
#include <windows.h>

namespace frozenbyte {
namespace launcher {

class WindowImpl;

class Window {
public:
	Window( const std::string& title, int iconId, bool maximize, bool disableSizing, int width = 320, int height = 200 );
	~Window();

	HWND getWindowHandle() const;
	void setSize(int xs, int ys);

private:
	WindowImpl* impl;
};

} // end of namespace launcher
} // end of namespace frozenbyte

#endif