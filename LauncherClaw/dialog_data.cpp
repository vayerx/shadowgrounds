#include "dialog_data.h"
#include "window.h"
#include "idlghandler.h"

#include <windows.h>
#include <iostream.h>
// #include <boost/lexical_cast.hpp>

namespace frozenbyte {
namespace launcher {

///////////////////////////////////////////////////////////////////////////////

class DialogDataImpl
{
public:
	HWND windowHandle;
	// std::map<int, CommandList> commands;

	IDlgHandler* handler;
	DialogData::ResizeType resizeType;

	bool modalDialog;
	int resourceId;

	int xPosition;
	int yPosition;
	

	DialogDataImpl() :
	  handler( NULL )
	{
	}

	~DialogDataImpl()
	{
	}

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		DialogDataImpl *data = reinterpret_cast<DialogDataImpl *> (GetWindowLong(windowHandle, GWL_USERDATA));
		
		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			data = reinterpret_cast<DialogDataImpl *> (lParam);
			data->windowHandle = windowHandle;

			// data->commands[message].execute(message);
		}

		if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY) 
			{
				EndDialog(windowHandle, 0);
				data->windowHandle = NULL;
			} else {
				// data->commands[message].execute(command);
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

			// data->commands[message].execute(id);
		} 
		else if(message == WM_NOTIFY)
		{
			// data->commands[message].execute(message);
			if(lParam)
			{
				NMHDR *ptr = (NMHDR *) lParam;
				// data->commands[message].execute(ptr->idFrom);
			}
		}


		if(data && data->handler && windowHandle)
		{
			data->handler->handleMessages(windowHandle, message, wParam, lParam);			
		}

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

		if((resizeType == DialogData::ATTACH_RIGHT) || (resizeType == DialogData::ATTACH_ALL))
			width = -(xPosition - parentSize.right);
		if((resizeType == DialogData::ATTACH_BOTTOM) ||(resizeType == DialogData::ATTACH_ALL))
			height = -(yPosition - parentSize.bottom);

		if((width != originalWidth) || (height != originalHeight))
			setSize(width, height);
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

///////////////////////////////////////////////////////////////////////////////

DialogData::DialogData(int resourceId, const Window &parentWindow, ResizeType type, IDlgHandler* handler )
{
	impl = new DialogDataImpl;
	impl->resizeType = type;
	impl->handler = handler;

	CreateDialogParam(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), parentWindow.getWindowHandle(), DialogDataImpl::DialogHandler, reinterpret_cast<LPARAM> (impl) );

}

//=============================================================================
/*
DialogData::DialogData(int resourceId, HWND parentWindowHandle, ResizeType type )
{
	impl = new DialogDataImpl;
	impl->resizeType = type;

	CreateDialogParam(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), parentWindowHandle, DialogDataImpl::DialogHandler, reinterpret_cast<LPARAM> (impl) );

}
*/
//=============================================================================

DialogData::DialogData( int resourceId )
{
	impl = new DialogDataImpl;
	
	impl->modalDialog = true;
	impl->resourceId = resourceId;

}

//=============================================================================

DialogData::~DialogData()
{
	delete impl;
}

///////////////////////////////////////////////////////////////////////////////

void DialogData::getSize(int &x, int &y) const
{
	RECT windowSize = { 0 };
	GetWindowRect(impl->windowHandle, &windowSize);

	x = windowSize.right - windowSize.left;
	y = windowSize.bottom - windowSize.top;
}

///////////////////////////////////////////////////////////////////////////////

void DialogData::updateSize()
{
	impl->updateSize();
}

//=============================================================================

void DialogData::setPosition( int x, int y )
{
	impl->setPosition( x, y );
}

//=============================================================================

void DialogData::setSize( int width, int height )
{
	impl->setSize( width, height );
}

//=============================================================================

void DialogData::show()
{
	impl->show();
}

//=============================================================================

void DialogData::hide()
{
	impl->hide();
}

///////////////////////////////////////////////////////////////////////////////

void DialogData::setDialogHandler( IDlgHandler* handler )
{
	impl->handler = handler;
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte