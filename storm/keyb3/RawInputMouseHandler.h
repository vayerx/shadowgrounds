
#ifndef __MOUSE_HANDLER
#define __MOUSE_HANDLER

#include <string>

#define MOUSEHANDLER_DEFAULT_MOUSE_ID -1
#define MOUSEHANDLER_ALL_MOUSES_ID -2

#define MOUSEHANDLER_ALL_KEYBOARDS_ID -2

#define MAX_MICE 24
#define MAX_KEYBOARDS 5
#define MAX_KEYS 256


class RawInputDeviceHandler
{

	//
	// RawInputDeviceHandler
	//
	// Initializes and handles mouse and keyboard using RawInput -interface. Makes it possible
	// to use multiple mice and keyboards. Should work with the old keyb3 -routines (almost) flawlessly.
	//
	// Some notes:
	//		* The last mouse in list of mouses is sum of all mouses. getMouseInfo(MOUSEHANDLER_ALL_MOUSES_ID)
	//		  returns this one. Notice that getMouseInfo(numMice - 1) returns it as well, so keep this in
	//		  mind when looping through mice or doing something like that.
	//		* Seems like RawInput gives handle(s) to some device(s) that doesn't seem to 
	//		  physically exist. So you can't assume that mouse 0 is first mouse, mouse 1 second,
	//		  etc.. Instead, I think the best way to solve which mouse ID is which, is to let
	//		  the user bind the mouses from configuration or something.
	//		* wndproc_ptr Eventhandler MUST be called from the window event handler at some point like this:
	//		  if(RawInputDeviceHandler::Eventhandler) 
	//				RawInputDeviceHandler::Eventhandler( hWnd, msg, wParam, lParam);
	//		  Otherwise input devices won't give any response when rawinput is enabled.
	//

	static bool loadDynamicSymbols();

	static std::string error;

public:
	struct MouseInfo
	{
		int X, Y;
		int dX, dY;

		bool leftButton, middleButton, rightButton;
		bool button1, button2, button3, button4, button5;
		int wheel, dwheel;

		// Do not use these manually.
		int oldX, oldY;
		int oldWheel;
	};
	struct KeyboardInfo
	{
		bool keyDown[ MAX_KEYS ];
	};

	static int lm;

	static unsigned int numMice, numKeyboards;
	static bool initialized;
	static bool mouseInitialized, keyboardInitialized;

	static bool init( bool initMice = true, bool initKeyboards = false );
	static void free();
	static MouseInfo * getMouseInfo ( int mouseID = MOUSEHANDLER_ALL_MOUSES_ID );
	static KeyboardInfo * getKeyboardInfo ( int keybID = MOUSEHANDLER_ALL_KEYBOARDS_ID );

	// Initialize an instance of this class if you want to access these.
	int getNumOfMouses();
	int getNumOfKeyboards(void);
	bool isInitialized();
	std::string getError();		// When something fails, this might help.

	// Used mostly to wrap this class to keyb3 more smoothly.
	static int getMouseArrayID( int mouseID );

private:

	static MouseInfo m_info[MAX_MICE];
	static KeyboardInfo k_info[MAX_KEYBOARDS];

};

#endif



