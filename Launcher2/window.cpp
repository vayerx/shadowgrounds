#include "window.h"
#include <windows.h>
#include "resource.h"

namespace frozenbyte {
namespace launcher {

namespace {
	static const char *windowClassName = "editor window";
}

///////////////////////////////////////////////////////////////////////////////

class WindowImpl
{
public:
	WindowImpl( const std::string& title, int iconId, bool maximize, bool disableSizing, int width, int height )
	{

		// Window class
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof( WNDCLASSEX );
		windowClass.style = 0; //CS_OWNDC;
		windowClass.lpfnWndProc = &WindowImpl::WindowProc;
		windowClass.hInstance = GetModuleHandle( 0 );
		windowClass.hIcon = LoadIcon( GetModuleHandle( 0 ), reinterpret_cast<const char *> ( iconId ) );
		windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
		windowClass.hbrBackground = 0; //CreateSolidBrush(0); //COLOR_TEXT; 
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = windowClassName;
		windowClass.hIconSm = 0;

		RegisterClassEx(&windowClass);

		// Default to fullscreen
		int windowSizeX = GetSystemMetrics( SM_CXSCREEN );
		int windowSizeY = GetSystemMetrics( SM_CYSCREEN );
		int windowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

		RECT rect;
		rect.left = GetSystemMetrics( SM_CXSCREEN ) / 2 - ( width / 2 );
		rect.top =  GetSystemMetrics( SM_CYSCREEN ) / 2 - ( height / 2 );
		rect.bottom = height + rect.top;
		rect.right = width + rect.left;

		BOOL t = AdjustWindowRectEx( &rect, windowStyle, false, 0 );

		windowSizeX = rect.right - rect.left;
		windowSizeY = rect.bottom - rect.top;
		windowStyle = WS_OVERLAPPEDWINDOW;

		if(disableSizing)
			windowStyle = WS_CAPTION | WS_SYSMENU |WS_MINIMIZEBOX;

		windowHandle = CreateWindowEx( 0, windowClassName, title.c_str(), windowStyle, rect.left, rect.top, windowSizeX, windowSizeY, 0, 0, GetModuleHandle(0), this );

		if(maximize)
			ShowWindow( windowHandle, SW_MAXIMIZE );
		else
			ShowWindow( windowHandle, SW_SHOW );
		SetFocus( windowHandle );


	}

	// Windows message proc
	static LRESULT CALLBACK WindowProc( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )
	{
		
		switch( message )
		{
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				break;
			}

			default:
			{

			}
		}
		return DefWindowProc( windowHandle, message, wParam, lParam );
	}

	HWND windowHandle;
};

///////////////////////////////////////////////////////////////////////////////

Window::Window( const std::string& title, int iconId, bool maximize, bool disableSizing, int width, int height )
{
	impl = new WindowImpl( title, iconId, maximize, disableSizing, width, height );
}

//=============================================================================

Window::~Window()
{
	delete impl;
	impl = NULL;
}

//=============================================================================

HWND Window::getWindowHandle() const
{
	return impl->windowHandle;
}

///////////////////////////////////////////////////////////////////////////////

void Window::setSize(int xs, int ys)
{
	// Parameter tells us client area, we need to calculate title bar and borders well
	{
		RECT rcClient = { 0 };
		GetClientRect(impl->windowHandle, &rcClient);

		RECT rcWindow = { 0 };
		GetWindowRect(impl->windowHandle, &rcWindow);

		int xd = rcWindow.right - rcWindow.left - rcClient.right;
		int yd = rcWindow.bottom - rcWindow.top - rcClient.bottom;

		xs += xd;
		ys += yd;
	}

	RECT desktopSize = { 0 };
	GetWindowRect(GetDesktopWindow(), &desktopSize);

	int res_x = desktopSize.right - desktopSize.left;
	int res_y = desktopSize.bottom - desktopSize.top;
	int x = (res_x - xs) / 2;
	int y = (res_y - ys) / 2;

	//RECT windowSize = { 0 };
	//GetWindowRect(impl->windowHandle, &windowSize);

	//MoveWindow(impl->windowHandle, windowSize.left, windowSize.top, xs, ys, TRUE);
	//MoveWindow(impl->windowHandle, windowSize.left, windowSize.top, xs, ys, TRUE);
	MoveWindow(impl->windowHandle, x, y, xs, ys, TRUE);
}

} // end of namespace launcher
} // end of namespace editor