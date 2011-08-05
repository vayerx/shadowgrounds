// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_DIALOG_H
#define INCLUDED_EDITOR_DIALOG_H

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif
#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

class Window;
class CommandList;
class IDlgHandler;
class ICommand;
class Mouse;
struct DialogData;

class Dialog
{
	boost::scoped_ptr<DialogData> data;

public:
	enum ResizeType
	{
		ATTACH_NONE = 0,
		ATTACH_RIGHT = 1,
		ATTACH_BOTTOM = 2,
		ATTACH_ALL = 3
	};

	Dialog(int resourceId, const Window &parentWindow, ResizeType type = ATTACH_NONE);
	Dialog(int resourceId, HWND parentWindowHandle, ResizeType type = ATTACH_NONE);
	explicit Dialog(int resourceId); // modal dialog, explicit creation with show()!
	~Dialog();
	
	void setPosition(int xPosition, int yPosition);
	void setSize(int xSize, int ySize);
	void getSize(int &xSize, int &ySize) const;

	void show();
	void hide();
	void setMouse(Mouse &mouse);

	CommandList &getCommandList(int id = WM_COMMAND);
	HWND getWindowHandle() const;

	HWND getParentWindowHandle() const;
	HWND getItem(int id) const;

	void setCustomHandler(IDlgHandler* handler);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
