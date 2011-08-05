// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <windows.h>
#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// User_MessageProc
//------------------------------------------------------------------
LRESULT (WINAPI *User_MessageProc)(struct HWND__ * hWnd,UINT msg,WPARAM wParam,LPARAM lParam)=NULL;



//------------------------------------------------------------------
// RenderWindow_MessageProc
//------------------------------------------------------------------
LRESULT WINAPI RenderWindow_MessageProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// First run the user messageproc!
	if (User_MessageProc) User_MessageProc(hWnd,msg,wParam,lParam);

	switch(msg)
	{
		// Quit program, if window is closed
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
			
		case WM_SETCURSOR:
			SetCursor(NULL);
			return 0;
	}

	// Do defaut window code
	return DefWindowProc(hWnd,msg,wParam,lParam );
}


