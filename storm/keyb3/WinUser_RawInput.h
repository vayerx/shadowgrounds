
#pragma once

#include <WinUser.h>



// HACK: Copypasted from WinUser.h, couldn't get it to work any other way.

#define WM_INPUT                        0x00FF

/*
* Raw Input Messages.
*/

DECLARE_HANDLE(HRAWINPUT);

/*
* WM_INPUT wParam
*/

/*
* Use this macro to get the input code from wParam.
*/
#define GET_RAWINPUT_CODE_WPARAM(wParam)    ((wParam) & 0xff)

/*
* The input is in the regular message flow,
* the app is required to call DefWindowProc
* so that the system can perform clean ups.
*/
#define RIM_INPUT       0

/*
* The input is sink only. The app is expected
* to behave nicely.
*/
#define RIM_INPUTSINK   1


/*
* Raw Input data header
*/
typedef struct tagRAWINPUTHEADER {
	DWORD dwType;
	DWORD dwSize;
	HANDLE hDevice;
	WPARAM wParam;
} RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

/*
* Type of the raw input
*/
#define RIM_TYPEMOUSE       0
#define RIM_TYPEKEYBOARD    1
#define RIM_TYPEHID         2

/*
* Raw format of the mouse input
*/
typedef struct tagRAWMOUSE {
	/*
	* Indicator flags.
	*/
	USHORT usFlags;

	/*
	* The transition state of the mouse buttons.
	*/
	union {
		ULONG ulButtons;
		struct  {
			USHORT  usButtonFlags;
			USHORT  usButtonData;
		};
	};


	/*
	* The raw state of the mouse buttons.
	*/
	ULONG ulRawButtons;

	/*
	* The signed relative or absolute motion in the X direction.
	*/
	LONG lLastX;

	/*
	* The signed relative or absolute motion in the Y direction.
	*/
	LONG lLastY;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ulExtraInformation;

} RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;

/*
* Define the mouse button state indicators.
*/

#define RI_MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define RI_MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define RI_MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define RI_MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define RI_MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define RI_MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

#define RI_MOUSE_BUTTON_1_DOWN      RI_MOUSE_LEFT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_1_UP        RI_MOUSE_LEFT_BUTTON_UP
#define RI_MOUSE_BUTTON_2_DOWN      RI_MOUSE_RIGHT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_2_UP        RI_MOUSE_RIGHT_BUTTON_UP
#define RI_MOUSE_BUTTON_3_DOWN      RI_MOUSE_MIDDLE_BUTTON_DOWN
#define RI_MOUSE_BUTTON_3_UP        RI_MOUSE_MIDDLE_BUTTON_UP

#define RI_MOUSE_BUTTON_4_DOWN      0x0040
#define RI_MOUSE_BUTTON_4_UP        0x0080
#define RI_MOUSE_BUTTON_5_DOWN      0x0100
#define RI_MOUSE_BUTTON_5_UP        0x0200

/*
* If usButtonFlags has RI_MOUSE_WHEEL, the wheel delta is stored in usButtonData.
* Take it as a signed value.
*/
#define RI_MOUSE_WHEEL              0x0400

/*
* Define the mouse indicator flags.
*/
#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02  // the coordinates are mapped to the virtual desktop
#define MOUSE_ATTRIBUTES_CHANGED 0x04  // requery for mouse attributes


/*
* Raw format of the keyboard input
*/
typedef struct tagRAWKEYBOARD {
	/*
	* The "make" scan code (key depression).
	*/
	USHORT MakeCode;

	/*
	* The flags field indicates a "break" (key release) and other
	* miscellaneous scan code information defined in ntddkbd.h.
	*/
	USHORT Flags;

	USHORT Reserved;

	/*
	* Windows message compatible information
	*/
	USHORT VKey;
	UINT   Message;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ExtraInformation;


} RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;


/*
* Define the keyboard overrun MakeCode.
*/

#define KEYBOARD_OVERRUN_MAKE_CODE    0xFF

/*
* Define the keyboard input data Flags.
*/
#define RI_KEY_MAKE             0
#define RI_KEY_BREAK            1
#define RI_KEY_E0               2
#define RI_KEY_E1               4
#define RI_KEY_TERMSRV_SET_LED  8
#define RI_KEY_TERMSRV_SHADOW   0x10


/*
* Raw format of the input from Human Input Devices
*/
typedef struct tagRAWHID {
	DWORD dwSizeHid;    // byte size of each report
	DWORD dwCount;      // number of input packed
	BYTE bRawData[1];
} RAWHID, *PRAWHID, *LPRAWHID;

/*
* RAWINPUT data structure.
*/
typedef struct tagRAWINPUT {
	RAWINPUTHEADER header;
	union {
		RAWMOUSE    mouse;
		RAWKEYBOARD keyboard;
		RAWHID      hid;
	} data;
} RAWINPUT, *PRAWINPUT, *LPRAWINPUT;

#ifdef _WIN64
#define RAWINPUT_ALIGN(x)   (((x) + sizeof(QWORD) - 1) & ~(sizeof(QWORD) - 1))
#else   // _WIN64
#define RAWINPUT_ALIGN(x)   (((x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))
#endif  // _WIN64

#define NEXTRAWINPUTBLOCK(ptr) ((PRAWINPUT)RAWINPUT_ALIGN((ULONG_PTR)((PBYTE)(ptr) + (ptr)->header.dwSize)))

/*
* Flags for GetRawInputData
*/

#define RID_INPUT               0x10000003
#define RID_HEADER              0x10000005

WINUSERAPI
UINT
WINAPI
GetRawInputData(
					 __in HRAWINPUT hRawInput,
					 __in UINT uiCommand,
					 __out_bcount_part_opt(*pcbSize, return) LPVOID pData,
					 __inout PUINT pcbSize,
					 __in UINT cbSizeHeader);

/*
* Raw Input Device Information
*/
#define RIDI_PREPARSEDDATA      0x20000005
#define RIDI_DEVICENAME         0x20000007  // the return valus is the character length, not the byte size
#define RIDI_DEVICEINFO         0x2000000b

typedef struct tagRID_DEVICE_INFO_MOUSE {
	DWORD dwId;
	DWORD dwNumberOfButtons;
	DWORD dwSampleRate;
} RID_DEVICE_INFO_MOUSE, *PRID_DEVICE_INFO_MOUSE;

typedef struct tagRID_DEVICE_INFO_KEYBOARD {
	DWORD dwType;
	DWORD dwSubType;
	DWORD dwKeyboardMode;
	DWORD dwNumberOfFunctionKeys;
	DWORD dwNumberOfIndicators;
	DWORD dwNumberOfKeysTotal;
} RID_DEVICE_INFO_KEYBOARD, *PRID_DEVICE_INFO_KEYBOARD;

typedef struct tagRID_DEVICE_INFO_HID {
	DWORD dwVendorId;
	DWORD dwProductId;
	DWORD dwVersionNumber;

	/*
	* Top level collection UsagePage and Usage
	*/
	USHORT usUsagePage;
	USHORT usUsage;
} RID_DEVICE_INFO_HID, *PRID_DEVICE_INFO_HID;

typedef struct tagRID_DEVICE_INFO {
	DWORD cbSize;
	DWORD dwType;
	union {
		RID_DEVICE_INFO_MOUSE mouse;
		RID_DEVICE_INFO_KEYBOARD keyboard;
		RID_DEVICE_INFO_HID hid;
	};
} RID_DEVICE_INFO, *PRID_DEVICE_INFO, *LPRID_DEVICE_INFO;

WINUSERAPI
UINT
WINAPI
GetRawInputDeviceInfoA(
							  __in_opt HANDLE hDevice,
							  __in UINT uiCommand,
							  __inout_bcount_part_opt(*pcbSize, return) LPVOID pData,
							  __inout PUINT pcbSize);
WINUSERAPI
UINT
WINAPI
GetRawInputDeviceInfoW(
							  __in_opt HANDLE hDevice,
							  __in UINT uiCommand,
							  __inout_bcount_part_opt(*pcbSize, return) LPVOID pData,
							  __inout PUINT pcbSize);
#ifdef UNICODE
#define GetRawInputDeviceInfo  GetRawInputDeviceInfoW
#else
#define GetRawInputDeviceInfo  GetRawInputDeviceInfoA
#endif // !UNICODE


/*
* Raw Input Bulk Read: GetRawInputBuffer
*/
WINUSERAPI
UINT
WINAPI
GetRawInputBuffer(
						__out_bcount_opt(*pcbSize) PRAWINPUT pData,
						__inout PUINT pcbSize,
						__in UINT cbSizeHeader);

/*
* Raw Input request APIs
*/
typedef struct tagRAWINPUTDEVICE {
	USHORT usUsagePage; // Toplevel collection UsagePage
	USHORT usUsage;     // Toplevel collection Usage
	DWORD dwFlags;
	HWND hwndTarget;    // Target hwnd. NULL = follows keyboard focus
} RAWINPUTDEVICE, *PRAWINPUTDEVICE, *LPRAWINPUTDEVICE;

typedef CONST RAWINPUTDEVICE* PCRAWINPUTDEVICE;

#define RIDEV_REMOVE            0x00000001
#define RIDEV_EXCLUDE           0x00000010
#define RIDEV_PAGEONLY          0x00000020
#define RIDEV_NOLEGACY          0x00000030
#define RIDEV_INPUTSINK         0x00000100
#define RIDEV_CAPTUREMOUSE      0x00000200  // effective when mouse nolegacy is specified, otherwise it would be an error
#define RIDEV_NOHOTKEYS         0x00000200  // effective for keyboard.
#define RIDEV_APPKEYS           0x00000400  // effective for keyboard.
#define RIDEV_EXMODEMASK        0x000000F0

#define RIDEV_EXMODE(mode)  ((mode) & RIDEV_EXMODEMASK)

WINUSERAPI
BOOL
WINAPI
RegisterRawInputDevices(
								__in_ecount(uiNumDevices) PCRAWINPUTDEVICE pRawInputDevices,
								__in UINT uiNumDevices,
								__in UINT cbSize);

WINUSERAPI
UINT
WINAPI
GetRegisteredRawInputDevices(
									  __out_ecount_opt( *puiNumDevices) PRAWINPUTDEVICE pRawInputDevices,
									  __inout PUINT puiNumDevices,
									  __in UINT cbSize);


typedef struct tagRAWINPUTDEVICELIST {
	HANDLE hDevice;
	DWORD dwType;
} RAWINPUTDEVICELIST, *PRAWINPUTDEVICELIST;

WINUSERAPI
UINT
WINAPI
GetRawInputDeviceList(
							 __out_ecount_opt(*puiNumDevices) PRAWINPUTDEVICELIST pRawInputDeviceList,
							 __inout PUINT puiNumDevices,
							 __in UINT cbSize);

WINUSERAPI
LRESULT
WINAPI
DefRawInputProc(
					 __in_ecount(nInput) PRAWINPUT* paRawInput,
					 __in INT nInput,
					 __in UINT cbSizeHeader);

// Extracted from other part of the file. Used to fake some keypresses because RawInput makes windows to ignore some.

typedef struct tagMOUSEINPUT {
	LONG    dx;
	LONG    dy;
	DWORD   mouseData;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, FAR* LPMOUSEINPUT;

typedef struct tagKEYBDINPUT {
	WORD    wVk;
	WORD    wScan;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, FAR* LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT {
	DWORD   uMsg;
	WORD    wParamL;
	WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2


typedef struct tagINPUT {
	DWORD   type;

	union
	{
		MOUSEINPUT      mi;
		KEYBDINPUT      ki;
		HARDWAREINPUT   hi;
	};
} INPUT, *PINPUT, FAR* LPINPUT;

WINUSERAPI
UINT
WINAPI
SendInput(
			 __in UINT cInputs,                     // number of input in the array
			 __in_ecount(cInputs) LPINPUT pInputs,  // array of inputs
			 __in int cbSize);                      // sizeof(INPUT)





typedef WINUSERAPI BOOL (WINAPI *RegisterRawInputDevices_ptr)(
								__in_ecount(uiNumDevices) PCRAWINPUTDEVICE pRawInputDevices,
								__in UINT uiNumDevices,
								__in UINT cbSize);

typedef WINUSERAPI UINT (WINAPI *GetRawInputData_ptr) (
					 __in HRAWINPUT hRawInput,
					 __in UINT uiCommand,
					 __out_bcount_part_opt(*pcbSize, return) LPVOID pData,
					 __inout PUINT pcbSize,
					 __in UINT cbSizeHeader);

typedef WINUSERAPI UINT (WINAPI *GetRawInputDeviceList_ptr) (
							 __out_ecount_opt(*puiNumDevices) PRAWINPUTDEVICELIST pRawInputDeviceList,
							 __inout PUINT puiNumDevices,
							 __in UINT cbSize);

typedef WINUSERAPI UINT (WINAPI * GetRawInputDeviceInfoA_ptr)(
							  __in_opt HANDLE hDevice,
							  __in UINT uiCommand,
							  __inout_bcount_part_opt(*pcbSize, return) LPVOID pData,
							  __inout PUINT pcbSize);

typedef WINUSERAPI UINT (WINAPI * SendInput_ptr)(
	__in UINT cInputs,                     
	__in_ecount(cInputs) LPINPUT pInputs,  
	__in int cbSize);                      

