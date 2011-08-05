// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "window.h"
#include "icommand.h"
#include "command_list.h"
#include "mouse.h"

#include <windows.h>
#include <map>
#include <cassert>
#include "resource/resource.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)
#endif

namespace frozenbyte {
namespace editor {
namespace {
	static const char *windowClassName = "editor window";
}

struct WindowData
{
	HWND windowHandle;
	CommandList commands;

	Mouse *mouse;
	bool windowActive;

	WindowData()
	{
		windowHandle = 0;
		windowActive = false;

		mouse = 0;
	}

	~WindowData()
	{
		deleteWindow();
	}

	void deleteWindow()
	{
		if(windowHandle)
		{
			DestroyWindow(windowHandle);
			UnregisterClass(windowClassName, GetModuleHandle(0));
			windowHandle = 0;
		}
	}

	static BOOL CALLBACK EnumChildProc(HWND windowHandle, LPARAM lparam)
	{
		SendMessage(windowHandle, WM_SIZE, 0, 0);
		return TRUE;
	}

	// Windows message proc
	static LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WindowData *data = reinterpret_cast<WindowData *> (GetWindowLong(windowHandle, GWL_USERDATA));

		// Windows want's us to quit
		if(message == WM_DESTROY)
		{
			PostQuitMessage(0);
		}
		else if(message == WM_NCCREATE)
		{
			// Get data pointer
			CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT *> (lParam);
			WindowData *data = reinterpret_cast<WindowData *> (createStruct->lpCreateParams);

			// Store our data pointer to window
			SetWindowLong(windowHandle, GWL_USERDATA, reinterpret_cast<long> (data));
		}
		else if(message == WM_SIZE)
		{
			// Resize childs if needed
			EnumChildWindows(windowHandle, EnumChildProc, 0);
		}

		if(message == WM_ACTIVATE)
		{
			if(LOWORD(wParam) != WA_INACTIVE)
				data->windowActive = true;
			else
				data->windowActive = false;

			// Minimized?
			if(HIWORD(wParam))
				data->windowActive = false;
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			data->commands.execute(command);
		}
		else if(message == WM_MOUSEWHEEL && data && data->mouse)
		{
			short value = HIWORD(wParam);
			data->mouse->setWheelDelta(value);
		}

		return DefWindowProc(windowHandle, message, wParam, lParam);
	}
};

Window::Window(const char *title, int iconId, bool maximize, bool disableSizing, int width, int height )
{
	boost::scoped_ptr<WindowData> tempData(new WindowData());

	// Window class
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = 0; //CS_OWNDC;
	windowClass.lpfnWndProc = &WindowData::WindowProc;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.hIcon = LoadIcon(GetModuleHandle(0), reinterpret_cast<const char *> (iconId));
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = 0; // CreateSolidBrush(0x383838); //COLOR_TEXT; 
	windowClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU); //0;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	// Default to fullscreen
	int windowSizeX = GetSystemMetrics(SM_CXSCREEN);
	int windowSizeY = GetSystemMetrics(SM_CYSCREEN);
	int windowStyle = WS_POPUP;

	windowSizeX /= 2;
	windowSizeY /= 2;
	windowStyle = WS_OVERLAPPEDWINDOW;

	if(disableSizing)
		windowStyle = WS_CAPTION | WS_SYSMENU |WS_MINIMIZEBOX;

	windowSizeX = width;
	windowSizeY = height;

	tempData->windowHandle = CreateWindowEx(0, windowClassName, title, windowStyle, 0, 0, windowSizeX, windowSizeY, 0, 0, GetModuleHandle(0), tempData.get());

	if(maximize)
		ShowWindow(tempData->windowHandle, SW_MAXIMIZE);
	else
		ShowWindow(tempData->windowHandle, SW_SHOW);
	SetFocus(tempData->windowHandle);

	data.swap(tempData);
}

Window::~Window()
{
}

void Window::setMouse(Mouse &mouse)
{
	data->mouse = &mouse;
}

CommandList &Window::getCommandList()
{
	return data->commands;
}

HWND Window::getWindowHandle() const
{
	return data->windowHandle;
}

bool Window::isActive() const
{
	return data->windowActive;
}

void Window::resize( int width, int height )
{
	MoveWindow( data->windowHandle, 0, 0, width, height, true );
}

} // end of namespace editor
} // end of namespace frozenbyte
