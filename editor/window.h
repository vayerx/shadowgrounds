// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_WINDOW_H
#define INCLUDED_EDITOR_WINDOW_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

namespace frozenbyte {
namespace editor {

class Mouse;
class ICommand;
class CommandList;
struct WindowData;

class Window
{
	boost::scoped_ptr<WindowData> data;

public:
	explicit Window(const char *title, int iconId, bool maximize, bool disableSizing, int width = 1024, int height = 782 );
	~Window();

	void setMouse(Mouse &mouse);
	void resize( int width, int height ); 

	CommandList &getCommandList();
	HWND getWindowHandle() const;

	bool isActive() const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
