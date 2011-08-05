
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include "../include/Storm3D_Common.h"
#include "../storm3dv2/RenderWindow.h"


#include "WinUser_RawInput.h"

//extern LRESULT (WINAPI *User_MessageProc)(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);


#include "RawInputMouseHandler.h"

//wndproc_ptr MouseHandler::formerMessageProc = NULL;
unsigned int RawInputDeviceHandler::numMice = 0;
unsigned int RawInputDeviceHandler::numKeyboards = 0;
RawInputDeviceHandler::MouseInfo RawInputDeviceHandler::m_info[MAX_MICE];
RawInputDeviceHandler::KeyboardInfo RawInputDeviceHandler::k_info[MAX_KEYBOARDS];
bool RawInputDeviceHandler::initialized = false;
std::string RawInputDeviceHandler::error = "Not initialized";
wndproc_ptr RawInputDeviceHandler::Eventhandler = NULL;
int RawInputDeviceHandler::lm = -1;
bool RawInputDeviceHandler::mouseInitialized = false;
bool RawInputDeviceHandler::keyboardInitialized = false;


// dynamic symbols to be loaded from user32.dll
RegisterRawInputDevices_ptr RawInputDeviceHandler::RegisterRawInputDevices_c = NULL;
GetRawInputData_ptr RawInputDeviceHandler::GetRawInputData_c = NULL;
GetRawInputDeviceList_ptr RawInputDeviceHandler::GetRawInputDeviceList_c = NULL;
GetRawInputDeviceInfoA_ptr RawInputDeviceHandler::GetRawInputDeviceInfoA_c = NULL;
SendInput_ptr RawInputDeviceHandler::SendInput_c = NULL;

LRESULT WINAPI MouseHandler_MessageProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	int retval = -1;	// Temporarily for debugging purproses.

	// Mouse event?
	if(msg == WM_INPUT && RawInputDeviceHandler::initialized)
	{
		UINT dataSize;
		RawInputDeviceHandler::GetRawInputData_c((HRAWINPUT)lParam, RID_INPUT, NULL, &dataSize, 
			sizeof(RAWINPUTHEADER));
		unsigned char *rawData = new unsigned char [ dataSize ];

		RawInputDeviceHandler::GetRawInputData_c((HRAWINPUT)lParam, RID_INPUT, rawData, &dataSize, 
			sizeof(RAWINPUTHEADER));
		RAWINPUT *rawInput = (RAWINPUT*)rawData;

		int mouseID = -1;
		// Mouse event?
		if(rawInput->header.dwType == RIM_TYPEMOUSE && RawInputDeviceHandler::mouseInitialized)
		{
			// Get the right mouse.
			for(unsigned int l = 0; l < RawInputDeviceHandler::numMice; l++)
				if( rawInput->header.hDevice == RawInputDeviceHandler::getMouseInfo(l)->mouseHandle )
					mouseID = l;

			if( mouseID == -1 )
			{
				// no correct mouse found??
				assert(!"Mouse event occured to a nonexistent mouse.");
				return 0;
			}

			RawInputDeviceHandler::MouseInfo *m_info = RawInputDeviceHandler::getMouseInfo(mouseID);
			RawInputDeviceHandler::MouseInfo *am_info = RawInputDeviceHandler::getMouseInfo(MOUSEHANDLER_ALL_MOUSES_ID);
			// movement
			m_info->oldX = m_info->X;
			m_info->oldY = m_info->Y;
			am_info->oldX = am_info->X;
			am_info->oldY = am_info->Y;
			if( (rawInput->data.mouse.usFlags & 1) == MOUSE_MOVE_ABSOLUTE )
			{
				m_info->X = rawInput->data.mouse.lLastX;
				m_info->Y = rawInput->data.mouse.lLastY;
				m_info->dX = m_info->X - m_info->oldX;
				m_info->dY = m_info->Y - m_info->oldY;
				
				// Update our "all mouses" -mouse as well.
				am_info->X = rawInput->data.mouse.lLastX;
				am_info->Y = rawInput->data.mouse.lLastY;
				am_info->dX = am_info->X - am_info->oldX;
				am_info->dY = am_info->Y - am_info->oldY;
			} else if( (rawInput->data.mouse.usFlags & 1) == MOUSE_MOVE_RELATIVE )
			{
				m_info->X+= rawInput->data.mouse.lLastX;
				m_info->Y+= rawInput->data.mouse.lLastY;
				m_info->dX = rawInput->data.mouse.lLastX;
				m_info->dY = rawInput->data.mouse.lLastY;

				// Update our "all mouses" -mouse as well.
				am_info->X+= rawInput->data.mouse.lLastX;
				am_info->Y+= rawInput->data.mouse.lLastY;
				am_info->dX = rawInput->data.mouse.lLastX;
				am_info->dY = rawInput->data.mouse.lLastY;
			}
			// buttons&wheel
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)		m_info->leftButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_UP)		m_info->leftButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)	m_info->rightButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_UP)		m_info->rightButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)	m_info->middleButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_UP)		m_info->middleButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_1_DOWN)			m_info->button1 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_2_DOWN)			m_info->button2 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_3_DOWN)			m_info->button3 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_DOWN)			m_info->button4 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_DOWN)			m_info->button5 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_1_UP)			m_info->button1 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_2_UP)			m_info->button2 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_3_UP)			m_info->button3 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_UP)			m_info->button4 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_UP)			m_info->button5 = false;

			if(rawInput->data.mouse.ulButtons & RI_MOUSE_WHEEL)
				m_info->dwheel = -(signed short)rawInput->data.mouse.usButtonData;

			// ...and the "all mouses"
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)		am_info->leftButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_UP)		am_info->leftButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)	am_info->rightButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_UP)		am_info->rightButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)	am_info->middleButton = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_UP)		am_info->middleButton = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_1_DOWN)			am_info->button1 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_2_DOWN)			am_info->button2 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_3_DOWN)			am_info->button3 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_DOWN)			am_info->button4 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_DOWN)			am_info->button5 = true;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_1_UP)			am_info->button1 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_2_UP)			am_info->button2 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_3_UP)			am_info->button3 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_UP)			am_info->button4 = false;
			if(rawInput->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_UP)			am_info->button5 = false;

			if(rawInput->data.mouse.ulButtons & RI_MOUSE_WHEEL)
				am_info->dwheel = -(signed short)rawInput->data.mouse.usButtonData;

		}
		else if( rawInput->header.dwType == RIM_TYPEKEYBOARD && RawInputDeviceHandler::keyboardInitialized )
		{

			// Get the right Keyboard.
			int keybID = -1;
			for(unsigned int l = 0; l < RawInputDeviceHandler::numKeyboards; l++)
				if( rawInput->header.hDevice == RawInputDeviceHandler::getKeyboardInfo(l)->keyboardHandle )
					keybID = l;

			if( keybID == -1 )
			{
				// no correct keyboard found??
				assert(!"Keyboard event occured to a nonexistent keyboard.");
				return 0;
			}

			static bool simulatedEscPressed = false;

			if(rawInput->data.keyboard.Message == WM_KEYDOWN)
			{
				RawInputDeviceHandler::getKeyboardInfo(keybID)->keyDown[rawInput->data.keyboard.MakeCode] = true;
				RawInputDeviceHandler::getKeyboardInfo(0)->keyDown[rawInput->data.keyboard.MakeCode] = true;

/*				// Handle ESC button as a special case so user is able to skip videos by pressing ESC.
				if(rawInput->data.keyboard.MakeCode == 1) // 1 == KEYCODE_ESC
				{
					INPUT input;
					input.type = INPUT_KEYBOARD;
					input.ki.wVk = VK_ESCAPE;
					input.ki.wScan = 1;
					input.ki.dwFlags = 0;
					input.ki.time = 0;
					input.ki.dwExtraInfo = 0;
					RawInputDeviceHandler::SendInput_c( 1, &input, sizeof(INPUT));
					simulatedEscPressed = true;
				}
*/
			}
			if(rawInput->data.keyboard.Message == WM_KEYUP)
			{
				RawInputDeviceHandler::getKeyboardInfo(keybID)->keyDown[rawInput->data.keyboard.MakeCode] = false;
				RawInputDeviceHandler::getKeyboardInfo(0)->keyDown[rawInput->data.keyboard.MakeCode] = false;

/*				if(rawInput->data.keyboard.MakeCode == 1) // 1 == KEYCODE_ESC
				{
					INPUT input;
					input.type = INPUT_KEYBOARD;
					input.ki.wVk = VK_ESCAPE;
					input.ki.wScan = 1;
					input.ki.dwFlags = KEYEVENTF_KEYUP;
					input.ki.time = 0;
					input.ki.dwExtraInfo = 0;
					RawInputDeviceHandler::SendInput_c( 1, &input, sizeof(INPUT));
				}
*/
			}

		} else
		{
			// this shouldn't happen
			assert(!"Invalid RawInput event type.");
			return 0;
		}

		return retval;
	} 
	return retval;
}

// defined in keyb.cpp.
extern RECT window_rect;
extern int	window_center_x, window_center_y;

bool RawInputDeviceHandler::init( HWND hwnd, bool initmouse, bool initkeyb )
{

	unsigned int numDevices = 0;
	PRAWINPUTDEVICELIST ri_deviceList = NULL;
	if(!initialized)
	{

		if(!loadDynamicSymbols ())
			return false;

		Eventhandler = MouseHandler_MessageProc;

		// Get number of devices.
		if ( GetRawInputDeviceList_c( NULL, &numDevices, sizeof(RAWINPUTDEVICELIST) ) )
		{
			error = "Failed querying number of devices.";
			return false;
		}

		ri_deviceList = new RAWINPUTDEVICELIST [numDevices];
		if ( GetRawInputDeviceList_c( ri_deviceList, &numDevices, sizeof(RAWINPUTDEVICELIST) ) < 1 )
		{
			error = "Failed querying RawInput devicelist.";
			return false;
		}

	}

	// MOUSE INIT //////////////////////////////////
	if(initmouse)
	{
		numMice = 0;

		// Read handles.
		int l2 = 0;
		for( unsigned int l = 0; l < numDevices; l++)
		{
			// device is not a mouse
			if( !ri_deviceList[l].hDevice || ri_deviceList[l].dwType != RIM_TYPEMOUSE )
			{
				continue;
			}

			// get mouse name
			bool is_rdp_mouse = false;
			unsigned int name_length = 0;
			if(GetRawInputDeviceInfoA_c(ri_deviceList[l].hDevice, RIDI_DEVICENAME, NULL, &name_length) >= 0
				&& name_length > 0 && name_length < 1024)
			{
				char *buffer = (char *)malloc(name_length + 1);
				if(GetRawInputDeviceInfoA_c(ri_deviceList[l].hDevice, RIDI_DEVICENAME, buffer, &name_length) >= 0)
				{
					// this is a remote desktop mouse
					if(strncmp(buffer, "\\??\\Root#RDP_MOU#", 17) == 0)
					{
						is_rdp_mouse = true;
					}
				}
				::free(buffer);
			}

			if(is_rdp_mouse)
			{
				// ignore remote desktop mouse
				continue;
			}

			// add new mouse
			ZeroMemory(&m_info[l2], sizeof(MouseInfo));
			m_info[l2].mouseHandle = ri_deviceList[l].hDevice;
			l2++;
			numMice++;
		}

		if(numMice == 0) {
			error = "No mouses found.";
			return false;
		}

		// For the "sum mouse," which contains sum of all mouses. (access with mouseID = -2)
		numMice++;
		ZeroMemory( &m_info[l2], sizeof(MouseInfo) );

		mouseInitialized = true;
	}

	// KEYBOARD INIT /////////////////////////////
	if(initkeyb)
	{


		numKeyboards = 0;
		// We do pretty much the same things as with the mouse.
		for( unsigned int l = 0; l < numDevices; l++)
			if ( ri_deviceList[l].hDevice && ri_deviceList[l].dwType == RIM_TYPEKEYBOARD )
				numKeyboards++;

		if(numKeyboards == 0)
		{
			error = "No keyboards found.";
			return false;
		}
		int l2 = 0;

		numKeyboards++;
		ZeroMemory( &k_info[l2++], sizeof(KeyboardInfo) );

		for( unsigned int l = 0; l < numDevices; l++)
			if ( ri_deviceList[l].hDevice && ri_deviceList[l].dwType == RIM_TYPEKEYBOARD )
			{
				ZeroMemory(&k_info[l2], sizeof(KeyboardInfo));
				k_info[l2].keyboardHandle = ri_deviceList[l].hDevice;
				l2++;
			}

		



		keyboardInitialized = true;
	}

	// Register devices.

	// Mouse
	int t = 0;
	RAWINPUTDEVICE rid[2];
	if(initmouse)
	{
		rid[t].usUsagePage	= 0x01;
		rid[t].usUsage			= 0x02;
		rid[t].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
		rid[t].hwndTarget = hwnd;	
		t++;
	}
	// Keyboard
	if(initkeyb)
	{
		rid[t].usUsagePage	= 0x01;
		rid[t].usUsage			= 0x06;
		rid[t].dwFlags = 0;
		rid[t].hwndTarget = hwnd;	
		t++;
	}

	if(t > 0)
	{
		if ( RegisterRawInputDevices_c ( rid, t, sizeof(RAWINPUTDEVICE) ) == FALSE )
		{
			error = "Failed registering RawInput device(s).";
			return false;
		}
	} else
	{
		error = "Neither mouse or keyboard not requested for initialization?";
		return false;
	}

	if(ri_deviceList)
		delete [] ri_deviceList;

	// Took this from the old mouse init from keyb.cpp 
	{
		int width = GetSystemMetrics (SM_CXSCREEN);
		int height = GetSystemMetrics (SM_CYSCREEN);

		GetWindowRect(hwnd, &window_rect);
		if (window_rect.left < 0)
			window_rect.left = 0;
		if (window_rect.top < 0)
			window_rect.top = 0;
		if (window_rect.right >= width)
			window_rect.right = width-1;
		if (window_rect.bottom >= height-1)
			window_rect.bottom = height-1;

		window_center_x = (window_rect.right + window_rect.left)/2;
		window_center_y = (window_rect.top + window_rect.bottom)/2;

		SetCursorPos (window_center_x, window_center_y);
		ClipCursor(&window_rect);
		while(ShowCursor (FALSE) >= 0);
	}

	error = "All OK.";
	initialized = true;
	return true;
}

void RawInputDeviceHandler::free()
{
//	if(m_info)
//		delete [] m_info;
}

RawInputDeviceHandler::MouseInfo *RawInputDeviceHandler::getMouseInfo ( int mouseID )
{
	if(mouseID == MOUSEHANDLER_DEFAULT_MOUSE_ID)
		mouseID = numMice - 2;
	if(mouseID == MOUSEHANDLER_ALL_MOUSES_ID || mouseID > (int)numMice || mouseID < -2)
		mouseID = numMice - 1;

	return &m_info[mouseID];
}

RawInputDeviceHandler::KeyboardInfo *RawInputDeviceHandler::getKeyboardInfo ( int mouseID )
{
	if(mouseID == MOUSEHANDLER_DEFAULT_MOUSE_ID)
		mouseID = numKeyboards - 2;
	if(mouseID == MOUSEHANDLER_ALL_MOUSES_ID || mouseID > (int)numKeyboards || mouseID < -2)
		mouseID = numKeyboards - 1;
	return &k_info[mouseID];
}

int RawInputDeviceHandler::getMouseArrayID ( int mouseID )
{
	if(mouseID == MOUSEHANDLER_DEFAULT_MOUSE_ID)
		return numMice - 2;
	if(mouseID == MOUSEHANDLER_ALL_MOUSES_ID || mouseID > (int)numMice || mouseID < -2)
		return numMice - 1;
	return mouseID;
}

int __thiscall RawInputDeviceHandler::getNumOfMouses() { return numMice; }
int __thiscall RawInputDeviceHandler::getNumOfKeyboards(void) { return numKeyboards; }
bool __thiscall RawInputDeviceHandler::isInitialized() { return initialized; }
std::string __thiscall RawInputDeviceHandler::getError() { return error; }

#define LOAD_SYMBOL(a, s)\
	if ( (s ## _c = (s ## _ptr)GetProcAddress(a, # s)) == NULL)\
	{\
		assert(strlen(#s) < 1024 - 80); \
		char msg[1024]; sprintf_s(msg, 1024, "Failed getting procedure address for function %s.", # s );\
		error = msg;\
		return false;\
	}


bool RawInputDeviceHandler::loadDynamicSymbols()
{
	HMODULE user32_dll = LoadLibrary("user32.dll");

	if(user32_dll == NULL)
	{
		error = "Failed loading user32.dll.";
		return false;
	}

	LOAD_SYMBOL(user32_dll, RegisterRawInputDevices);
	LOAD_SYMBOL(user32_dll, GetRawInputData);
	LOAD_SYMBOL(user32_dll, GetRawInputDeviceList);
	LOAD_SYMBOL(user32_dll, GetRawInputDeviceInfoA);
	LOAD_SYMBOL(user32_dll, SendInput);

	return true;
}

