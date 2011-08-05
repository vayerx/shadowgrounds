
#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <string>

#include "IGameControllerKeyreader.h"

#include "../storm/keyb3/RawInputMouseHandler.h"

#include "igios.h"

// Note: these camera ctrl values equal the camera move values in GameCamera
// They do not need to be the same, but keeping it so may be a good idea.

#define RUN_SCRIPT_CTRL_AMOUNT 16
#define WEAPON_CTRL_AMOUNT 16

#define GAMECONTROLLER_MAX_KEYREADERS 8
#define GAMECONTROLLER_KEYREADER_BUFFER_SIZE 64


#define DH_CTRL_DUMMY 0

#define DH_CTRL_CAMERA_MOVE_FORWARD 1
#define DH_CTRL_CAMERA_MOVE_BACKWARD 2
#define DH_CTRL_CAMERA_MOVE_LEFT 3
#define DH_CTRL_CAMERA_MOVE_RIGHT 4
#define DH_CTRL_CAMERA_MOVE_ROTATE_LEFT 5
#define DH_CTRL_CAMERA_MOVE_ROTATE_RIGHT 6
#define DH_CTRL_CAMERA_MOVE_ORBIT_LEFT 7
#define DH_CTRL_CAMERA_MOVE_ORBIT_RIGHT 8
#define DH_CTRL_CAMERA_MOVE_ZOOM_IN 9
#define DH_CTRL_CAMERA_MOVE_ZOOM_OUT 10
#define DH_CTRL_CAMERA_MOVE_UP 11
#define DH_CTRL_CAMERA_MOVE_DOWN 12
#define DH_CTRL_CAMERA_MOVE_ROTATE_UP 13
#define DH_CTRL_CAMERA_MOVE_ROTATE_DOWN 14
#define DH_CTRL_CAMERA_MOVE_ZOOM_NEXT 15

#define DH_CTRL_CAMERA_TO_SELECTED_UNIT 16
#define DH_CTRL_CAMERA_TO_NEXT_UNIT 17
#define DH_CTRL_CAMERA_UNIT_LOCK_TOGGLE 18
#define DH_CTRL_CAMERA_BEHIND_SELECTED_UNIT 19
#define DH_CTRL_CAMERA_ALIGN_TO_SELECTED_UNIT 20
#define DH_CTRL_CAMERA_FIRST_PERSON_TOGGLE 21
#define DH_CTRL_CAMERA_TO_ACTION 22
#define DH_CTRL_CAMERA_ACTION_LOCK_TOGGLE 23
#define DH_CTRL_CAMERA_TERRAIN_LOCK_TOGGLE 24

#define DH_CTRL_GUI_TOGGLE 25
#define DH_CTRL_GUI_NEXT_LAYOUT 26

#define DH_CTRL_MULTIPLE_UNIT_SELECT 27
#define DH_CTRL_FORCE_ATTACK 28

#define DH_CTRL_ATTACK 29
#define DH_CTRL_CHANGE_NEXT_WEAPON 30

#define DH_CTRL_UNIT_MODE_TOGGLE 31

#define DH_CTRL_SELECT_ALL_UNITS 32
#define DH_CTRL_SELECT_UNIT_1 33
#define DH_CTRL_SELECT_UNIT_2 34
#define DH_CTRL_SELECT_UNIT_3 35
#define DH_CTRL_SELECT_UNIT_4 36
#define DH_CTRL_SELECT_UNIT_5 37
#define DH_CTRL_SELECT_UNIT_6 38
#define DH_CTRL_SELECT_TEAM_1 39
#define DH_CTRL_SELECT_TEAM_2 40
#define DH_CTRL_SELECT_TEAM_3 41

#define DH_CTRL_QUIT 42
#define DH_CTRL_CAMERA_MOVE_ZOOM_MODE 43

#define DH_CTRL_GUI_RADAR_TOGGLE 44
#define DH_CTRL_GUI_UNITS_TOGGLE 45

#define DH_CTRL_NEXT_CAMERA_MODE 46
#define DH_CTRL_CAMERA_LOOK_MODE 47

#define DH_CTRL_SCREENSHOT 48

#define DH_CTRL_CAMERA_UNIT_ALIGN_LOCK_TOGGLE 49

#define DH_CTRL_PAUSE 50
#define DH_CTRL_TACTICAL_MODE 51

#define DH_CTRL_CAMERA_MOVE_FOV_IN 52
#define DH_CTRL_CAMERA_MOVE_FOV_OUT 53

#define DH_CTRL_NEXT_CAMERA_ANGLE_BOUNDARY 54
#define DH_CTRL_SPECIAL_MOVE 55
#define DH_CTRL_STOP 56
#define DH_CTRL_CEASE_FIRE 57
#define DH_CTRL_STOP_AND_CEASE_FIRE 58

#define DH_CTRL_CONSOLE_TOGGLE 59

#define DH_CTRL_GRENADE 60
#define DH_CTRL_CHANGE_PREV_WEAPON 61

#define DH_CTRL_RELOAD 62
#define DH_CTRL_FLASHLIGHT 63
#define DH_CTRL_EXECUTE 64

#define DH_CTRL_ATTACK_SECONDARY 65
#define DH_CTRL_USE_MEDIKIT 66

#define DH_CTRL_WEAPON_SELECT_MODE 67

#define DH_CTRL_CONTROL_MODE_SWITCH 68
#define DH_CTRL_OTHER_UNIT_CONTROL 69
#define DH_CTRL_SELECT_NEXT_UNIT 70

#define DH_CTRL_RESERVED_71 71	// Seems like these are used in occlusion debugging..
#define DH_CTRL_RESERVED_72 72
#define DH_CTRL_RESERVED_73 73
#define DH_CTRL_RESERVED_74 74

// NOTE: these must ne incremental in id values:
#define DH_CTRL_RUN_SCRIPT_1 75
#define DH_CTRL_RUN_SCRIPT_2 76
#define DH_CTRL_RUN_SCRIPT_3 77
#define DH_CTRL_RUN_SCRIPT_4 78
#define DH_CTRL_RUN_SCRIPT_5 79
#define DH_CTRL_RUN_SCRIPT_6 80
#define DH_CTRL_RUN_SCRIPT_7 81
#define DH_CTRL_RUN_SCRIPT_8 82
#define DH_CTRL_RUN_SCRIPT_9 83
#define DH_CTRL_RUN_SCRIPT_10 84
#define DH_CTRL_RUN_SCRIPT_11 85
#define DH_CTRL_RUN_SCRIPT_12 86
#define DH_CTRL_RUN_SCRIPT_13 87
#define DH_CTRL_RUN_SCRIPT_14 88
#define DH_CTRL_RUN_SCRIPT_15 89
#define DH_CTRL_RUN_SCRIPT_16 90

// NOTE: these must ne incremental in id values:
#define DH_CTRL_WEAPON_1 91
#define DH_CTRL_WEAPON_2 92
#define DH_CTRL_WEAPON_3 93
#define DH_CTRL_WEAPON_4 94
#define DH_CTRL_WEAPON_5 95
#define DH_CTRL_WEAPON_6 96
#define DH_CTRL_WEAPON_7 97
#define DH_CTRL_WEAPON_8 98
#define DH_CTRL_WEAPON_9 99
#define DH_CTRL_WEAPON_10 100
#define DH_CTRL_WEAPON_11 101
#define DH_CTRL_WEAPON_12 102
#define DH_CTRL_WEAPON_13 103
#define DH_CTRL_WEAPON_14 104
#define DH_CTRL_WEAPON_15 105
#define DH_CTRL_WEAPON_16 106

#define DH_CTRL_OPEN_ANIRECORDER 107
#define DH_CTRL_OPEN_UPGRADE 108
#define DH_CTRL_USE_SELECTED_ITEM 109
#define DH_CTRL_OPEN_MAP 110
#define DH_CTRL_OPEN_LOG 111

#define DH_CTRL_CLOSE_LOADING_WINDOW 112

#define DH_CTRL_CAMERA_TARGET_OFFSET_MODE 113
#define DH_CTRL_CAMERA_POSITION_OFFSET_MODE 114

#define DH_CTRL_SPRINT 115

#define DH_CTRL_OPEN_MENU 116

#define DH_CTRL_AMOUNT 117

// 2 alternative keys can be assigned for each control
#define DH_CTRL_BINDS_PER_CONTROL 2

//#define KEYCODE_NAME_AMOUNT 534
#define KEYCODE_NAME_AMOUNT (534 + ADDITIONAL_KEYBOARD_KEYS_AMOUNT)

class Ogui;

namespace ui
{
struct JoystickValues;
 
class GameController
{
public:

	enum JOYSTICK_AXIS
	{
		JOYSTICK_AXIS_UNKNOWN = 0,
		JOYSTICK_AXIS_X,
		JOYSTICK_AXIS_Y,
		JOYSTICK_AXIS_RX,
		JOYSTICK_AXIS_RY,
		JOYSTICK_AXIS_THROTTLE,
		JOYSTICK_AXIS_RUDDER

	};

	enum CONTROLLER_TYPE
	{
		CONTROLLER_TYPE_KEYBOARD_ONLY = 100,
		CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 = 101,
		CONTROLLER_TYPE_KEYBOARD_AND_MOUSE2 = 102,
		CONTROLLER_TYPE_KEYBOARD_AND_MOUSE3 = 103,
		CONTROLLER_TYPE_KEYBOARD_AND_MOUSE4 = 104,
		CONTROLLER_TYPE_JOYSTICK1 = 105,
		CONTROLLER_TYPE_JOYSTICK2 = 106,
		CONTROLLER_TYPE_JOYSTICK3 = 107,
		CONTROLLER_TYPE_JOYSTICK4 = 108,
	};

	// NEW: depends on ogui (due to need for hotkey disabling)
	GameController(Ogui *ogui);
	virtual ~GameController();

	// load and save configuration (TODO at the time of writing this text)
	void reloadConfiguration();
	void loadConfiguration( const char *filename);
	void saveConfiguration( const char *filename);
	void unloadConfiguration();

	// returns a string name for the given control
	const char *getControlName(int controlNum);

	// returns the name of the key
	const char* getKeycodeName( int keycode );

	// returns true if the button bind to this control is down
	bool isKeyDown(int controlNum);

	bool isKeyDownByKeyCode( int keycode );

	// returns true if the button bind to this control was clicked since last 
	// call (the key was pressed down and then released)
	bool wasKeyClicked(int controlNum);

	// return true if at least one key is assigned for this control
	bool isAnyKeyBound(int controlNum);

	// returns the keycode bound to given control 
	// alternativeNum defines which alternative is returned 
	// (alternativeNum values start from 0)
	int getBoundKey(int controlNum, int alternativeNum);

	// binds a keycode to given control
	// if alternativeNum is not given (or it is -1), it is chosen automagically
	// if clearFromOthers is set, bind will be removed from all other controls
	void bindKey(int controlNum, int keycode, int alternativeNum = -1, 
		bool clearFromOthers = true);

	// unbinds a keycode to given control
	// if alternativeNum is not given (or it is -1), all alternatives are cleared
	void unbindKey(int controlNum, int alternativeNum = -1);

	// unbinds a given keycode from all controls
	void unbindKeyByKeycode(int keycode);

	// returns relative mouse movement or joystick movement if autodetection is on
	// (sets values pointed by given params)			- hacked by Pete
	void getMouseDelta(int *deltaX, int *deltaY, int mouseID = MOUSEHANDLER_ALL_MOUSES_ID);
	
	// sets the autodetection on / off
	void setMouseJoystickAutodetection( bool value );

	// returns the relative position of the main controller ( mouse / joystick / other )
	// void getControllerDelta( int *deltaX, int *deltaY );

	// can be used to enable/disable controls.
	// quit is the only one that will never be disabled.
	void setControlsEnabled(bool enabled);

	// NOTE: this has been removed because use of it is dangerous, it does not return a value that
	// could be sensibly used for anything... --jpk
	// haxored controller_enabled
	//bool getControlsEnabled() const;

	// returns value set with setControlsEnabled
	bool getUserControlsEnabled() const;

	// set "forced control enabled" status for this specific control
	// so that this control will still be used even if setControlsEnabled(false) was called.
	// NOTE: forced status will be cleared when setControlsEnabled state changes
	void setForcedEnable(int controlNum, bool ctrlEnabled);

	void updateEnabledStatus();

	int addKeyreader(IGameControllerKeyreader *keyreader);

	void removeKeyreader(int id);

	// supply either ascii or keycode (and the other _may_ be filled in by the keyreader)
	// leave the other to 0 value.
	virtual void addReadKey(char ascii, int keycode);

	// user signalled quit
	virtual void suicide() __attribute__((noreturn));

	void run();

	// resets the axis information for detection
	void			startJoystickDetection();

	// returns != 0 when movement is detected in the joystick axis
	JOYSTICK_AXIS	getDetectedAxis();

	void getJoystickValues(int joynum, int *x, int *y, int *rx, int *ry, int *throttle, int *rudder);

	void getJoystickValues(int *moveX, int *moveY, int *dirX, int *dirY );
	void getJoystickValues(int joynum, int *moveX, int *moveY, int *dirX, int *dirY );

	void setJoystickMoveXAxis( JOYSTICK_AXIS axis );
	void setJoystickMoveYAxis( JOYSTICK_AXIS axis );
	void setJoystickDirXAxis( JOYSTICK_AXIS axis );
	void setJoystickDirYAxis( JOYSTICK_AXIS axis );

	JOYSTICK_AXIS getJoystickMoveXAxis() const;
	JOYSTICK_AXIS getJoystickMoveYAxis() const;
	JOYSTICK_AXIS getJoystickDirXAxis() const;
	JOYSTICK_AXIS getJoystickDirYAxis() const;

	std::string getJoystickAxisName( JOYSTICK_AXIS axis ) const;

	// returns the type of controller for that given key
	CONTROLLER_TYPE getControllerTypeByKey( int key ) const;

	// converts key to other controller (joystick only)
	int convertKeyToController( int key, CONTROLLER_TYPE type ) const;
	
	// returns the type of the controller
	CONTROLLER_TYPE getControllerType() const;
	bool controllerTypeHasMouse() const;
	bool controllerTypeHasJoystick() const;

	void			setControllerType( CONTROLLER_TYPE type );


	// returns control number for given name, or -1 if no such control name
	int getControlNumberForName(const char *forCtrlName);

	// returns keycode number for given name, or -1 if no such keycode name
	int getKeycodeNumberForName(const char *forKeycodeName);

	void setControlOn(int controlNum);

	void setControlOff(int controlNum);

private:

	Ogui *ogui;

	char convertKeycodeToAscii(int keycode, bool shiftDown, bool altDown);

	int **binds;

	// mouse wheel is a special case...
	int wheelUpStartTime;
	int wheelDownStartTime;

	int lastKeyReadTime;
	int lastNoKeysDownTime;
	bool noKeysDownAtLastRead;
	bool mouseJoystickAutodetection;

	bool controlsEnabled;
	bool userControlsEnabled;
	IGameControllerKeyreader *keyreader;
	int keyreaderId;

	IGameControllerKeyreader *keyreaderStack[GAMECONTROLLER_MAX_KEYREADERS];
	int keyreaderIdStack[GAMECONTROLLER_MAX_KEYREADERS];
	char readKeyAsciiBuf[GAMECONTROLLER_KEYREADER_BUFFER_SIZE];
	int readKeyKeycodeBuf[GAMECONTROLLER_KEYREADER_BUFFER_SIZE];
	int readKeyWritePos;
	int readKeyReadPos;

	bool ctrlOn[DH_CTRL_AMOUNT];
	bool prevCtrlOn[DH_CTRL_AMOUNT];

	bool ctrlForceEnabled[DH_CTRL_AMOUNT];

	std::string loadedConfFromHere;

	CONTROLLER_TYPE	controllerType;

	JOYSTICK_AXIS	joystickMoveXAxis;
	JOYSTICK_AXIS	joystickMoveYAxis;
	JOYSTICK_AXIS	joystickDirXAxis;
	JOYSTICK_AXIS	joystickDirYAxis;
	JoystickValues*	joystickMaxiumValues[4];


};

}

#endif
