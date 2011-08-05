// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "dialog.h"
#include "window.h"
#include "icommand.h"
#include "command_list.h"
#include "idlg_handler.h"
#include "mouse.h"

#include <boost/lexical_cast.hpp>
#include <windows.h>
#include <map>
#include <cassert>

#include "string_conversions.h"

namespace frozenbyte {
namespace editor {

BOOL CALLBACK getChildRectProc(HWND hwndChild, LPARAM lParam) 
{
	RECT &childRect = *(RECT *)lParam;

	RECT rect;
	GetWindowRect(hwndChild, &rect);
	if(rect.bottom > childRect.bottom) childRect.bottom = rect.bottom;
	if(rect.top < childRect.top) childRect.top = rect.top;
	return TRUE;
}

struct DialogData
{
	HWND windowHandle;
	std::map<int, CommandList> commands;

	IDlgHandler* handler;
	Dialog::ResizeType resizeType;

	bool modalDialog;
	int resourceId;

	int xPosition;
	int yPosition;

	Mouse *mouse;

	DialogData()
	{
		handler = 0;
		windowHandle = 0;
		resizeType = Dialog::ATTACH_NONE;

		modalDialog = false;
		resourceId = 0;

		xPosition = 0;
		yPosition = 0;

		mouse = 0;
	}

	~DialogData()
	{
		if(windowHandle)
		{
			EndDialog(windowHandle, 0);
			windowHandle = 0;
		}
	}

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		DialogData *data = reinterpret_cast<DialogData *> (GetWindowLong(windowHandle, GWL_USERDATA));
		
		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			data = reinterpret_cast<DialogData *> (lParam);
			data->windowHandle = windowHandle;

			data->commands[message].execute(message);
		}

		if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY) 
			{
				EndDialog(windowHandle, 0);
				data->windowHandle = NULL;
			} else {
				data->commands[message].execute(command);
			}

		}
		else if(message == WM_SIZE)
		{
			data->updateSize();
		}
		else if(message == WM_HSCROLL) 
		{
			HWND hwndScrollBar = (HWND) lParam;
			int id = GetDlgCtrlID(hwndScrollBar);

			data->commands[message].execute(id);
		}
		else if(message == WM_VSCROLL)
		{
			HWND hwndScrollBar = (HWND) lParam;
			int id = GetDlgCtrlID(hwndScrollBar);

			SCROLLINFO si;
			si.cbSize = sizeof (si);
			si.fMask  = SIF_ALL;
			GetScrollInfo(windowHandle, SB_VERT, &si);
			int pos = si.nPos;
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				pos = si.nMin;
				break;
			case SB_BOTTOM:
				pos = si.nMax;
				break;
			case SB_LINEUP:
				pos -= 1;
				break;
			case SB_LINEDOWN:
				pos += 1;
				break;
			case SB_PAGEUP:
				pos -= si.nPage;
				break;
			case SB_PAGEDOWN:
				pos += si.nPage;
				break;
			case SB_THUMBPOSITION:
				pos = HIWORD(wParam);
				break;
			default:
				break;
			}

			SetScrollPos(windowHandle, SB_VERT, pos, TRUE);
			// SetScrollPos does the clamping
			pos = GetScrollPos(windowHandle, SB_VERT);
			int current = si.nPos;
			if(pos != current)
			{
				int delta = pos - current;
				RECT rect;
				GetClientRect(windowHandle, &rect);
				ScrollWindow(windowHandle, 0, -delta, NULL, NULL);
				InvalidateRect(windowHandle, NULL, TRUE);
			}
		}
		else if(message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN)
		{
			SetFocus(windowHandle);
		}
		else if(message == WM_NOTIFY)
		{
			//data->commands[message].execute(message);
			if(lParam)
			{
				NMHDR *ptr = (NMHDR *) lParam;
				data->commands[message].execute(ptr->idFrom);
			}
		}

		if(data && data->mouse)
		{
			if(message == WM_LBUTTONDOWN)
				data->mouse->setLeftButtonDown();
			else if(message == WM_LBUTTONUP)
				data->mouse->setLeftButtonUp();
			else if(message == WM_RBUTTONDOWN)
				data->mouse->setRightButtonDown();
			else if(message == WM_RBUTTONUP)
				data->mouse->setRightButtonUp();
		}
/*
		if(data && data->handler && windowHandle)
		{
			data->handler->handleMessages(windowHandle, message, wParam, lParam);			
		}
*/
		return 0;
	}

	void updateSize()
	{
		if(IsIconic(GetParent(windowHandle)))
			return;

		RECT parentSize =  { 0 };
		GetClientRect(GetParent(windowHandle), &parentSize);

		RECT windowSize = { 0 };
		GetWindowRect(windowHandle, &windowSize);
	
		int originalWidth = windowSize.right - windowSize.left;
		int originalHeight = windowSize.bottom - windowSize.top;

		int width = originalWidth;
		int height = originalHeight;

		if((resizeType == Dialog::ATTACH_RIGHT) || (resizeType == Dialog::ATTACH_ALL))
			width = -(xPosition - parentSize.right);
		if((resizeType == Dialog::ATTACH_BOTTOM) ||(resizeType == Dialog::ATTACH_ALL))
			height = -(yPosition - parentSize.bottom);

		if((width != originalWidth) || (height != originalHeight))
			setSize(width, height);

		// scroll back up
		ScrollWindow(windowHandle, 0, GetScrollPos(windowHandle, SB_VERT), NULL, NULL);
		SetScrollPos(windowHandle, SB_VERT, 0, FALSE);

		// calculate new scrolling range
		RECT childRect;
		childRect.bottom = -INT_MAX;
		childRect.top = INT_MAX;
		EnumChildWindows(windowHandle, getChildRectProc, (LPARAM) &childRect); 
		if(childRect.bottom > childRect.top)
		{
			RECT windowRect;
			GetWindowRect(windowHandle, &windowRect);

			RECT parentRect;
			GetWindowRect(GetParent(windowHandle), &parentRect);

			int visible_bottom = windowRect.bottom;
			if(parentRect.bottom < visible_bottom)
				visible_bottom = parentRect.bottom;

			int range = childRect.bottom - visible_bottom;
			if(range < 0) range = 0;
			else range += 32; // need to scroll a little further for some unknown reason
			SetScrollRange(windowHandle, SB_VERT, 0, range, FALSE);
		}
		else
		{
			SetScrollRange(windowHandle, SB_VERT, 0, 0, FALSE);
		}

		SCROLLINFO si;
		si.cbSize = sizeof (si);
		si.fMask  = SIF_ALL;
		GetScrollInfo(windowHandle, SB_VERT, &si);
		si.nPage = 16;
		SetScrollInfo(windowHandle, SB_VERT, &si, TRUE);
	}

	void setPosition(int xPosition_, int yPosition_)
	{
		xPosition = xPosition_;
		yPosition = yPosition_;

		RECT windowSize =  { 0 };
		GetWindowRect(windowHandle, &windowSize);

		MoveWindow(windowHandle, xPosition, yPosition, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, TRUE);
		updateSize();
	}

	void setSize(int xSize, int ySize)
	{
		MoveWindow(windowHandle, xPosition, yPosition, xSize, ySize, TRUE);
		updateSize();
	}

	void show()
	{
		ShowWindow(windowHandle, SW_SHOW);
	}

	void hide()
	{
		ShowWindow(windowHandle, SW_HIDE);
	}
};

Dialog::Dialog(int resourceId, const Window &parentWindow, ResizeType type)
{
	boost::scoped_ptr<DialogData> tempData(new DialogData());
	tempData->resizeType = type;
	
	CreateDialogParam(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), parentWindow.getWindowHandle(), DialogData::DialogHandler, reinterpret_cast<LPARAM> (tempData.get()) );
	data.swap(tempData);
}

Dialog::Dialog(int resourceId, HWND parentWindowHandle, ResizeType type) 
{
	boost::scoped_ptr<DialogData> tempData(new DialogData());
	tempData->resizeType = type;
	
	CreateDialogParam(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), parentWindowHandle, DialogData::DialogHandler, reinterpret_cast<LPARAM> (tempData.get()) );
	data.swap(tempData);
}

Dialog::Dialog(int resourceId)
{
	boost::scoped_ptr<DialogData> tempData(new DialogData());
	tempData->modalDialog = true;
	tempData->resourceId = resourceId;
	
	data.swap(tempData);
}

Dialog::~Dialog()
{
}

void Dialog::setPosition(int xPosition, int yPosition)
{
	data->setPosition(xPosition, yPosition);
}

void Dialog::setSize(int xSize, int ySize)
{
	data->setSize(xSize, ySize);
}

void Dialog::getSize(int &xSize, int &ySize) const
{
	RECT parentSize =  { 0 };
	GetClientRect(GetParent(data->windowHandle), &parentSize);

	RECT windowSize = { 0 };
	GetWindowRect(data->windowHandle, &windowSize);

	xSize = windowSize.right - windowSize.left;
	ySize = windowSize.bottom - windowSize.top;
}

void Dialog::show()
{
	if(data->modalDialog)
		DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(data->resourceId), 0, DialogData::DialogHandler, reinterpret_cast<LPARAM> (data.get()) );

	data->show();
}

void Dialog::hide()
{
	data->hide();

	if(data->windowHandle && data->modalDialog)
		EndDialog(data->windowHandle, 0);
}

void Dialog::setMouse(Mouse &mouse)
{
	data->mouse = &mouse;
}

CommandList &Dialog::getCommandList(int id)
{
	return data->commands[id];
}
HWND Dialog::getWindowHandle() const
{
	return data->windowHandle;
}

HWND Dialog::getParentWindowHandle() const
{
	return GetParent(data->windowHandle);
}

HWND Dialog::getItem(int id) const 
{
	return GetDlgItem(data->windowHandle, id);
}

void Dialog::setCustomHandler(IDlgHandler* handler) 
{
	data->handler = handler;
}

} // end of namespace editor
} // end of namespace frozenbyte
