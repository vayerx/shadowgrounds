#include "precompiled.h"

// this is a conversion from C source code and certainly not thread safe

// don't use directly, use the C++ Ogui API instead

/* *********************************************************** */
/* Includes */

//#include <stdio.h>
#include <stdlib.h>
#include <Storm3D_UI.h>
#include <keyb3.h>
#include "orvgui2.h"

#include "../game/options/options_controllers.h"

#include "../system/Logger.h"

#include "../util/UnicodeConverter.h"
#include "../util/Debug_MemoryManager.h"

#include "../storm/storm3dv2/VertexFormats.h"

/* *********************************************************** */
/* Definitions */

#define CURSOR_SIZE_X 32
#define CURSOR_SIZE_Y 32

#define MAX_KEYS 256

#define MAX_KEYREADERS 8

/* *********************************************************** */
/* Global variables */

// the connection to Storm3D
IStorm3D_Scene *og_renderer = NULL;
IStorm3D *og_storm3d = NULL;

// a few dummy variables
int got_keyboard = 1;
int got_mouse = 1;
int got_joystick = 1;

int scr_size_x = 1024;
int scr_size_y = 768;
int og_scale_x = OG_SCALE_MULTIPLIER;
int og_scale_y = OG_SCALE_MULTIPLIER;

float og_mouse_sensitivity_x = 1.0f;
float og_mouse_sensitivity_y = 1.0f;

// and some old bad global stuff
//const char *og_errorfile = "guierrors.txt";

int got_orvgui = 0;

void *og_arg = NULL;

int og_errors = 0;

int og_terminate = 0;

int og_disable_kj_move = 0;

bool og_menu_index_mode = false;

bool og_hotkeys_enabled = true;

short og_cursor_num;
short og_cursor_x;
short og_cursor_y;
short og_cursor_scrx;
short og_cursor_scry;
unsigned int og_cursor_but;
unsigned int og_cursor_obut;
orvgui_but *og_cursor_on;

char og_readchar = 0;
int og_readkey = 0;

int og_escape = 0;

bool og_visualize_windows = false;
void debugMaterialInit(void);

/* *********************************************************** */
/* Internal variables */

static orvgui_win *first_win = NULL;
static orvgui_win *last_win = NULL;

static orvgui_win *error_win = NULL;
static int error_line = 0;

static IStorm3D_Material *cursor_pic[OG_CURSORS];
static int cursor_offset_x[OG_CURSORS];
static int cursor_offset_y[OG_CURSORS];
static int cursor_state[OG_CURSORS];
static unsigned int cursor_control[OG_CURSORS];		
static int cursor_control_keys[OG_CURSORS][6];
int cursor_x[OG_CURSORS];
int cursor_y[OG_CURSORS];
static unsigned int cursor_but[OG_CURSORS];
static int cursor_ox[OG_CURSORS];
static int cursor_oy[OG_CURSORS];
static unsigned int cursor_obut[OG_CURSORS];
static orvgui_but *cursor_last_highlight[OG_CURSORS];
static orvgui_but *cursor_on[OG_CURSORS];
static int cursor_speed[OG_CURSORS];
static orvgui_win *cursor_drag[OG_CURSORS];
static int cursor_drag_offx[OG_CURSORS];
static int cursor_drag_offy[OG_CURSORS];

static int og_mouse_x[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_y[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_but[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_ox[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_oy[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_obut[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_next_offset_x[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int og_mouse_next_offset_y[MAX_MICE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int cursor_button_index = 0;

static orvgui_win *only_active = NULL;

// TODO: define separately for every mouse
static int og_skip_cursor_movement = 0;

static void (*keyreader[MAX_KEYREADERS])(void);
static int keyreader_id[MAX_KEYREADERS];
static void *keyreader_arg[MAX_KEYREADERS];
static int keyreader_cursor[MAX_KEYREADERS];

static int keyhandled[MAX_KEYS];

static char asc_for_key[MAX_KEYS] = { 0, 27, '1', '2', '3', '4', '5', '6', '7', '8',
																			 '9', '0', '+', 0, 0, 0, 'Q', 'W', 'E', 'R',
																			 'T', 'Y', 'U', 'I', 'O', 'P', 'Å', 0, 13, 0,
																			 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ä',
																			 'Ö', '\'', 0, 0, 'Z', 'X', 'C', 'V', 'B', 'N',
																			 'M', ',', '.', '-', 0, '*', 0, ' ', 0, 'a',
																			 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 0,
																			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																			 0, 0, 0, 0, 0, 0, 0, 'k', 'l', 0,
																			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																			 0, 0, 0, 0, 0, 0, 0, 0
																	};


static IStorm3D_Material *debug_window_material = NULL;
static IStorm3D_Material *debug_window_hidden_material = NULL;
static IStorm3D_Material *debug_button_material = NULL;
static IStorm3D_Material *debug_button_disabled_material = NULL;

/* *********************************************************** */
/* Prototypes */

#ifndef OG_HIDE_OWN_CURSORS
static void og_draw_cursors(void);
#endif
static void og_find_cursor_on(int curnum);
static int og_dont_hide_popups = 0;

static bool og_get_button_position_by_index(int index_num, orvgui_but **found_but, int *x, int *y, int *last_index);

//static void og_error_ignore(void);
//static void og_error_terminate(void);

/* *********************************************************** */
/* Functions */

char key_to_asc(int keycode)
{
	return asc_for_key[keycode];
}

/* ----------------------------------------------------------- */

void og_setRendererScene(IStorm3D_Scene *scene)
{
	og_renderer = scene;
}

/* ----------------------------------------------------------- */

void og_setStorm3D(IStorm3D *s3d)
{
	og_storm3d = s3d;
}

/* ----------------------------------------------------------- */

void og_set_mouse_sensitivity(float sensitivity_x, float sensitivity_y)
{
	og_mouse_sensitivity_x = sensitivity_x;
	og_mouse_sensitivity_y = sensitivity_y;
}

/* ----------------------------------------------------------- */

void og_set_skip_cursor_movement()
{
	og_skip_cursor_movement = 1;
}

/* ----------------------------------------------------------- */

int og_get_mouse_ID(unsigned int controlflags)
// RawInput: return ID to the array from the controlflags. 
{
	RawInputDeviceHandler mh;
	if(mh.isInitialized ())
	{
		// RawInput
		if(controlflags & OG_CTRL_MASK_MOUSE)
			return Keyb3_DefaultMouseID ();
		if(controlflags & OG_CTRL_MASK_MOUSE0)
			return 0;
		if(controlflags & OG_CTRL_MASK_MOUSE1) 
			return 1;
		if(controlflags & OG_CTRL_MASK_MOUSE2)
			return 2;
		if(controlflags & OG_CTRL_MASK_MOUSE3)
			return 3;
		if(controlflags & OG_CTRL_MASK_MOUSE4)
			return 4;
	} else
	{
		// DirectInput/Win32api
		return 0;
	}

	// Someone screwed up, return some value which crashes the program (makes it easier to debug, hopefully.)
	return -1;
}


/* ----------------------------------------------------------- */

int og_get_cursor_position_x(int cursornum, bool exact)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if(got_mouse && !exact) {
		if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) return og_mouse_x[og_get_mouse_ID(cursor_control[cursornum])];
	}

	return cursor_x[cursornum];
}

/* ----------------------------------------------------------- */

int og_get_cursor_position_y(int cursornum, bool exact)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if(got_mouse && !exact) {
		if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) return og_mouse_y[og_get_mouse_ID(cursor_control[cursornum])];
	}
	return cursor_y[cursornum];
}

/* ----------------------------------------------------------- */

void og_set_cursor_position_x(int cursornum, int screenX)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) og_mouse_x[og_get_mouse_ID(cursor_control[cursornum])] = screenX;
	cursor_x[cursornum] = screenX;

	og_apply_system_cursor_pos(cursornum);
}

/* ----------------------------------------------------------- */

void og_set_cursor_position_y(int cursornum, int screenY)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) og_mouse_y[og_get_mouse_ID(cursor_control[cursornum])] = screenY;

	cursor_y[cursornum] = screenY;

	og_apply_system_cursor_pos(cursornum);
}

/* ----------------------------------------------------------- */

void og_set_cursor_position_offset_x(int cursornum, int screenOffsetX)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) og_mouse_next_offset_x[og_get_mouse_ID(cursor_control[cursornum])] = screenOffsetX;

}

/* ----------------------------------------------------------- */

void og_set_cursor_position_offset_y(int cursornum, int screenOffsetY)
{
	// hack return the mouse position directly instead of cursor position
	// (as cursor position would be updated later on, causing some lag)
	if (cursor_control[cursornum] & OG_CTRL_MASK_MICE) og_mouse_next_offset_y[og_get_mouse_ID(cursor_control[cursornum])] = screenOffsetY;
}



/* ----------------------------------------------------------- */

void og_apply_system_cursor_pos(int cursornum)
{
	// if mouse cursor, apply position to windows, else, just ignore...
	if ((cursor_control[cursornum] & OG_CTRL_MASK_MICE))
	{
		if (cursor_x[cursornum] >= 1024) cursor_x[cursornum] = 1024 - 1;
		if (cursor_y[cursornum] >= 768) cursor_y[cursornum] = 768 - 1;
		int winmx = (int)(cursor_x[cursornum] / og_mouse_sensitivity_x) * og_scale_x / OG_SCALE_MULTIPLIER;
		int winmy = (int)(cursor_y[cursornum] / og_mouse_sensitivity_y) * og_scale_y / OG_SCALE_MULTIPLIER;
		//if (winmx >= 1024) winmx = 1024 - 1;
		//if (winmy >= 768) winmy = 768 - 1;

		Keyb3_SetMousePos(winmx, winmy, og_get_mouse_ID (cursor_control[cursornum]));
	}
}

/* ----------------------------------------------------------- */

// A bit messy but makes everybody's life a bit easier. See below for usage.
#define OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(n)\
	if(l == n && l != Keyb3_GetNumberOfMouseDevices () - 1) {\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON1)) mousebut |= OG_BUT_1_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON2)) mousebut |= OG_BUT_2_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON3)) mousebut |= OG_BUT_3_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON4)) mousebut |= OG_BUT_4_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON5)) mousebut |= OG_BUT_5_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON6)) mousebut |= OG_BUT_6_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _BUTTON7)) mousebut |= OG_BUT_7_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _WHEEL_UP) && !(og_mouse_obut[l] & OG_BUT_WHEEL_UP_MASK))		mousebut |= OG_BUT_WHEEL_UP_MASK;\
		if (Keyb3_IsKeyDown(KEYCODE_MOUSE ## n ## _WHEEL_DOWN) && !(og_mouse_obut[l] & OG_BUT_WHEEL_DOWN_MASK))	mousebut |= OG_BUT_WHEEL_DOWN_MASK;\
	}


void og_update_cursor_positions()
{
	// this just updates mouse cursor position to reduce lag...

	if (got_mouse)
	{
		for(int l = 0; l < Keyb3_GetNumberOfMouseDevices (); l++)
		{

			int mousex = 0;
			int mousey = 0;
			int mousebut = 0;
//			static int oldmousebut[MAX_MICE] = {0,0,0,0,0,0,0,0,0,0,0,0};

			// Handle separate mice's buttons.
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(0);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(1);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(2);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(3);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(4);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(5);
			OG_UPDATE_CURSOR_POSITONS__HANDLE_MOUSE_BUTTONS(6);

			// Handle the mousedevice which is affected by every mouse. (Or if using DirectInput)
			if(l == Keyb3_GetNumberOfMouseDevices () - 1)
			{
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON1))
					mousebut |= OG_BUT_1_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON2)) mousebut |= OG_BUT_2_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON3)) mousebut |= OG_BUT_3_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON4)) mousebut |= OG_BUT_4_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON5)) mousebut |= OG_BUT_5_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON6)) mousebut |= OG_BUT_6_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_BUTTON7)) mousebut |= OG_BUT_7_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_WHEEL_UP) && !(og_mouse_obut[l] & OG_BUT_WHEEL_UP_MASK)) mousebut |= OG_BUT_WHEEL_UP_MASK;
				if (Keyb3_IsKeyDown(KEYCODE_MOUSE_WHEEL_DOWN) && !(og_mouse_obut[l] & OG_BUT_WHEEL_DOWN_MASK)) mousebut |= OG_BUT_WHEEL_DOWN_MASK;
			}
			
			Keyb3_ReadMouse(&mousex, &mousey, NULL, NULL, l);

			// psd
			static int original_mouse_x[MAX_MICE] = {0,0,0,0,0,0,0,0,0,0,0,0};
			static int original_mouse_y[MAX_MICE] = {0,0,0,0,0,0,0,0,0,0,0,0};
			
			og_mouse_ox[l] = og_mouse_x[l];
			og_mouse_oy[l] = og_mouse_y[l];
			og_mouse_obut[l] = og_mouse_but[l];


			if (og_skip_cursor_movement != 0)
			{
				//Keyb3_SetMousePos(original_mouse_x[l], original_mouse_y[l], l);
			} else {
				// psd
				original_mouse_x[l] = mousex;
				original_mouse_y[l] = mousey;

				og_mouse_x[l] = (int)(mousex * og_mouse_sensitivity_x) * OG_SCALE_MULTIPLIER / og_scale_x;
				og_mouse_y[l] = (int)(mousey * og_mouse_sensitivity_y) * OG_SCALE_MULTIPLIER / og_scale_y;

				if (og_mouse_next_offset_x[l] != 0
					|| og_mouse_next_offset_y[l]!= 0)
				{
					og_mouse_x[l] += og_mouse_next_offset_x[l];
					og_mouse_y[l] += og_mouse_next_offset_y[l];
					og_mouse_next_offset_x[l] = 0;
					og_mouse_next_offset_y[l] = 0;

					if (og_mouse_x[l] >= 1024) og_mouse_x[l] = 1024 - 1;
					if (og_mouse_y[l] >= 768) og_mouse_y[l] = 768 - 1;

					int winmx = (int)(og_mouse_x[l] / og_mouse_sensitivity_x) * og_scale_x / OG_SCALE_MULTIPLIER;
					int winmy = (int)(og_mouse_y[l] / og_mouse_sensitivity_y) * og_scale_y / OG_SCALE_MULTIPLIER;
					// if (winmx >= 1024) winmx = 1024 - 1;
					// if (winmy >= 768) winmy = 768 - 1;

					Keyb3_SetMousePos(winmx, winmy, l);
				}

			}


			if (og_mouse_x[l] < 0) og_mouse_x[l] = 0;
			if (og_mouse_y[l] < 0) og_mouse_y[l] = 0;
			if (og_mouse_x[l] >= 1024) og_mouse_x[l] = 1024 - 1;
			if (og_mouse_y [l]>= 768) og_mouse_y[l] = 768 - 1;

			og_mouse_but[l] = mousebut;
		}
		if(og_skip_cursor_movement != 0)
			og_skip_cursor_movement = 0;
	}
}


/* ----------------------------------------------------------- */

void og_handle_window_hotkeys(orvgui_win *win)
{
	orvgui_but *hotk_but;

	if (win->visible)
	{
		if (win->first_child != NULL)
		{
			hotk_but = (orvgui_but *)(win->first_child);

			while (hotk_but != NULL)
			{
				if (hotk_but->hotKeys[0] != 0
					&& hotk_but->enabled != 0)
				{
					bool hotkeysDown = true;
					for (int i = 0; i < OG_MAX_HOTKEYS; i++)
					{
						if (hotk_but->hotKeys[i] != 0)
						{
							// first hotkey is considered to be the "main key"
							// the others as modifier keys
							if (i == 0)
							{
								if (!Keyb3_IsKeyPressed(hotk_but->hotKeys[i]))
								{
									hotkeysDown = false;
								}
							} else {
								if (!Keyb3_IsKeyDown(hotk_but->hotKeys[i]))
								{
									hotkeysDown = false;
								}
							}
						}
					}
					if (hotkeysDown)
					{
						og_arg = hotk_but->arg_click;
						(hotk_but->proc_click)();
					}
				}

				hotk_but = (orvgui_but *)(hotk_but->next_sister);
			}
		}
	}
}


/* ----------------------------------------------------------- */

void og_handle_hotkeys(void)
{
	// NOTE: does not work properly for multiple cursors (assuming one cursor only)
	// (does not handle each cursor differently)

	if (!og_hotkeys_enabled)
	{
		return;
	}

	orvgui_win *hotk_win;

	hotk_win = last_win;

	while (hotk_win != NULL)
	{
		og_handle_window_hotkeys(hotk_win);
		hotk_win = (orvgui_win *)(hotk_win->prev_sister);
	}

}

/* ----------------------------------------------------------- */


// helper function for processing joystick inputs
static void processJoystick(int joynum, int g, int &add_speed, int joystickMask, int joystickKey)
{
	if (cursor_control[g] & joystickMask)
	{
		if (got_joystick)
		{
			if (og_disable_kj_move == 0 && !og_menu_index_mode)
			{
				int x = 0, y = 0;

				Keyb3_ReadJoystick(joynum, &x, &y, NULL, NULL, NULL, NULL);

				int deadzoneSize = game::SimpleOptions::getInt(DH_OPT_I_JOYSTICK1_DEADZONE + joynum);

				if (  (y < -deadzoneSize)
				   || (y > deadzoneSize) )
				{
					add_speed = 1;
					cursor_y[g] += (cursor_speed[g] >> 2) * (y - deadzoneSize) / (1000 - deadzoneSize);
				}

				if (  (x < -deadzoneSize)
				   || (x > deadzoneSize) )
				{
					add_speed = 1;
					cursor_x[g] += (cursor_speed[g] >> 2) * (x - deadzoneSize) / (1000 - deadzoneSize);
				}

			}
			if (Keyb3_IsKeyDown(joystickKey - KEYCODE_JOY_UP + KEYCODE_JOY_BUTTON1) != 0) cursor_but[g] |= 1;
			if (Keyb3_IsKeyDown(joystickKey - KEYCODE_JOY_UP + KEYCODE_JOY_BUTTON2) != 0) cursor_but[g] |= 2;
			// Added for more sensible pad click buttons
			if (Keyb3_IsKeyDown(joystickKey - KEYCODE_JOY_UP + KEYCODE_JOY_BUTTON5) != 0) cursor_but[g] |= 1;
			if (Keyb3_IsKeyDown(joystickKey - KEYCODE_JOY_UP + KEYCODE_JOY_BUTTON6) != 0) cursor_but[g] |= 2;
		}
	}

}


// notice: does not call Keyb3_UpdateDevices(), call it before calling this!
void og_run_gui(void)
{
	int g;
	int add_speed;
	orvgui_but *cursor_out;

	if (only_active != NULL)
	{
		if (first_win != only_active) og_raise_window(only_active);
	}

	og_update_cursor_positions();

	/*
	if (got_keyboard)
	{
		if (Keyb3_IsKeyDown(KEYCODE_CONTROL_LEFT) && Keyb3_IsKeyDown(KEYCODE_C))
		{
			if (og_terminate != 0)
			{
				//og_show_error("Failed to exit, crashing.");
				//exit(1);
			}
			og_show_error("Exit requested by user.");
			og_terminate = 1;
		}
	}
	*/

	for (g = 0; g < OG_CURSORS; g++)
	{
		cursor_obut[g] = cursor_but[g];
		cursor_but[g] = 0;
		add_speed = 0;

		if ((cursor_control[g] & OG_CTRL_MASK_MICE))
		{
			if (got_mouse)
			{
				int mID = og_get_mouse_ID(cursor_control[g]);
				
				if (og_mouse_x[mID] != og_mouse_ox[mID] || og_mouse_y[mID] != og_mouse_oy[mID])
				{
					cursor_x[g] = og_mouse_x[mID];
					cursor_y[g] = og_mouse_y[mID];
				}
				cursor_but[g] = (unsigned int)og_mouse_but[mID];
			}
		}

		if (cursor_control[g] & OG_CTRL_MASK_KEYBOARD1)
		{
			if (got_keyboard)
			{
				if (og_disable_kj_move == 0)
				{
					if (Keyb3_IsKeyDown(KEYCODE_UP_ARROW) != 0)
					{
						add_speed = 1;
						cursor_y[g]-=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(KEYCODE_LEFT_ARROW) != 0)
					{
						add_speed = 1;
						cursor_x[g]-=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(KEYCODE_RIGHT_ARROW) != 0)
					{
						add_speed = 1;
						cursor_x[g]+=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(KEYCODE_DOWN_ARROW) != 0)
					{
						add_speed = 1;
						cursor_y[g]+=cursor_speed[g]>>2;
					}
				}
				if (Keyb3_IsKeyDown(KEYCODE_ENTER) != 0) cursor_but[g] |= 1;
				if (Keyb3_IsKeyDown(KEYCODE_SHIFT_RIGHT) != 0) cursor_but[g] |= 2;
			}
		}
		if (cursor_control[g] & OG_CTRL_MASK_KEYBOARD2)
		{
			if (got_keyboard)
			{
				int key_left = cursor_control_keys[g][0];
				int key_right = cursor_control_keys[g][1];
				int key_up = cursor_control_keys[g][2];
				int key_down = cursor_control_keys[g][3];
				int key_fire = cursor_control_keys[g][4];
				int key_fire2 = cursor_control_keys[g][5];
				if (og_disable_kj_move == 0)
				{
					if (Keyb3_IsKeyDown(key_up) != 0)
					{
						add_speed = 1;
						cursor_y[g]-=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(key_left) != 0)
					{
						add_speed = 1;
						cursor_x[g]-=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(key_right) != 0)
					{
						add_speed = 1;
						cursor_x[g]+=cursor_speed[g]>>2;
					}
					if (Keyb3_IsKeyDown(key_down) != 0)
					{
						add_speed = 1;
						cursor_y[g]+=cursor_speed[g]>>2;
					}
				}
				if (Keyb3_IsKeyDown(key_fire) != 0) cursor_but[g] |= 1;
				if (Keyb3_IsKeyDown(key_fire2) != 0) cursor_but[g] |= 2;
			}
		}

		processJoystick(0, g, add_speed, OG_CTRL_MASK_JOYSTICK1, KEYCODE_JOY_UP);
		processJoystick(1, g, add_speed, OG_CTRL_MASK_JOYSTICK2, KEYCODE_JOY2_UP);
		processJoystick(2, g, add_speed, OG_CTRL_MASK_JOYSTICK3, KEYCODE_JOY3_UP);
		processJoystick(3, g, add_speed, OG_CTRL_MASK_JOYSTICK4, KEYCODE_JOY4_UP);
		if (og_disable_kj_move == 0)
		{
			if (add_speed == 0)
			{
				cursor_speed[g] = 0;
			} else {
				if (cursor_speed[g] < 4*4) cursor_speed[g]++;
				if (cursor_speed[g] < 1*4) cursor_speed[g] = 1*4;
			}
		}

		// HACK: joystick moves to next/prev button...
		// HACK: cursor 0 only
#ifndef PROJECT_SURVIVOR
		if (got_joystick && og_menu_index_mode
			&& g == 0 && !(cursor_control[g] & OG_CTRL_MASK_DISABLE_JOYSTICK_WARP))
		{
			if (Keyb3_IsKeyDown(KEYCODE_JOY_BUTTON1) != 0)
			{
				cursor_but[g] |= OG_BUT_1_MASK;
			}

			if (Keyb3_IsKeyPressed(KEYCODE_JOY_UP) != 0
				|| Keyb3_IsKeyPressed(KEYCODE_JOY_DOWN) != 0
				|| Keyb3_IsKeyPressed(KEYCODE_JOY_LEFT) != 0
				|| Keyb3_IsKeyPressed(KEYCODE_JOY_RIGHT) != 0)
			{
				if (Keyb3_IsKeyPressed(KEYCODE_JOY_UP) != 0
					|| Keyb3_IsKeyPressed(KEYCODE_JOY_LEFT) != 0)
				{
					cursor_button_index++;
				}
				if (Keyb3_IsKeyPressed(KEYCODE_JOY_DOWN) != 0
					|| Keyb3_IsKeyPressed(KEYCODE_JOY_RIGHT) != 0)
				{
					cursor_button_index--;										
				}
				int x, y, last_index;
				orvgui_but *tmp;
				bool success = og_get_button_position_by_index(cursor_button_index, &tmp, &x, &y, &last_index);
				if (!success)
				{
					if (Keyb3_IsKeyPressed(KEYCODE_JOY_UP) != 0
						|| Keyb3_IsKeyPressed(KEYCODE_JOY_LEFT) != 0)
					{
						cursor_button_index = 0;
					}
					if (Keyb3_IsKeyPressed(KEYCODE_JOY_DOWN) != 0
						|| Keyb3_IsKeyPressed(KEYCODE_JOY_RIGHT) != 0)
					{
						cursor_button_index = last_index; 									
					}
					if (last_index >= 0)
					{
						success = og_get_button_position_by_index(cursor_button_index, &tmp, &x, &y, &last_index);
					}
				}
				if (success)
				{
					cursor_x[g] = x;
					cursor_y[g] = y;
				}
			}
		}
#endif

		if (cursor_x[g] < 0) cursor_x[g] = 0;
		if (cursor_y[g] < 0) cursor_y[g] = 0;
		if (cursor_x[g] >= scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x) 
			cursor_x[g] = (scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x) - 1;
		if (cursor_y[g] >= scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y) 
			cursor_y[g] = (scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y) - 1;


		/*
		// HACK: Wheel can't be hold down. We'll interpret it as click-event instead.
		if (cursor_but[g] & OG_BUT_WHEEL_UP_MASK && !(cursor_obut[g] & OG_BUT_WHEEL_UP_MASK))
		{
			cursor_obut[g] |= OG_BUT_WHEEL_UP_MASK;
			cursor_but[g]  &= 0xFFFFFFFF ^ OG_BUT_WHEEL_UP_MASK;
		}
		else if(cursor_but[g] & OG_BUT_WHEEL_DOWN_MASK)
		{
			cursor_obut[g] |= OG_BUT_WHEEL_DOWN_MASK;
			cursor_but[g]  &= 0xFFFFFFFF ^ OG_BUT_WHEEL_DOWN_MASK;
		}
		*/

		// button clicks, etc.
		if (cursor_but[g] != 0 || cursor_obut[g] != 0)
		{
			if (cursor_drag[g] != NULL)
			{
				if (cursor_x[g] != cursor_drag[g]->put_x+cursor_drag_offx[g]
					|| cursor_y[g] != cursor_drag[g]->put_y+cursor_drag_offy[g])
				{
					og_move_window(cursor_drag[g],
						(short)(cursor_x[g] - cursor_drag_offx[g]),
						(short)(cursor_y[g] - cursor_drag_offy[g]));
				}
				// bad on keyboard - too many keyboard buttons jam...
				// so holding down one button while dragging is enough...
				//			if ((cursor_but[g] & (OG_BUT_1_MASK | OG_BUT_2_MASK))
				//				!= (OG_BUT_1_MASK | OG_BUT_2_MASK))
				if (cursor_but[g] == 0)
				{
					cursor_drag[g]->dragged = 0;
					cursor_drag[g] = NULL;
				}
			} else {
				if (cursor_obut[g] == 0)
				{
					cursor_out = NULL;
				} else {
					cursor_out = cursor_on[g];
				}
				og_find_cursor_on(g);

				if (cursor_on[g] != cursor_out)
				{
					if (cursor_out != NULL)
					{
						if ((cursor_out->react_but) != 0)
							og_unpress_button(cursor_out);
						og_arg = cursor_out->arg_out;
						(cursor_out->proc_out)();
					}
					if (cursor_on[g] != NULL)
					{
						if ((cursor_on[g]->react_but & cursor_but[g]) != 0)
							og_press_button(cursor_on[g]);
						og_arg = cursor_on[g]->arg_press;
						(cursor_on[g]->proc_press)();
					}
				}
				if (cursor_but[g] == 0)
				{
					if (cursor_on[g] != NULL)
					{
						if ((cursor_on[g]->react_but) != 0)
							og_unpress_button(cursor_on[g]);
						og_arg = cursor_on[g]->arg_click;
						(cursor_on[g]->proc_click)();
					}
				}
			}
		}


		// button hold down
		if (cursor_but[g] != 0 && cursor_obut[g] == cursor_but[g] ) // && cursor_control[g] != 0)
		{
			if (cursor_drag[g] == NULL)
			{
				og_dont_hide_popups = 1;
				og_find_cursor_on(g);
				og_dont_hide_popups = 0;
				if (cursor_on[g] != NULL)
				{
					og_arg = cursor_on[g]->arg_hold;
					(cursor_on[g]->proc_hold)();
				}
			}
		}

		// button highlighting (mouseover)
		if (cursor_but[g] == 0 && cursor_obut[g] == 0 && cursor_control[g] != 0)
		{
			if (cursor_drag[g] == NULL)
			{
				og_dont_hide_popups = 1;
				og_find_cursor_on(g);
				og_dont_hide_popups = 0;
				if (cursor_last_highlight[g] != cursor_on[g])
				{
					if (cursor_last_highlight[g] != NULL)
					{
						if ((cursor_last_highlight[g]->react_but & OG_BUT_OVER_MASK) != 0)
							og_unhighlight_button(cursor_last_highlight[g]);
						og_arg = cursor_last_highlight[g]->arg_leave;
						(cursor_last_highlight[g]->proc_leave)();
						cursor_last_highlight[g] = NULL;
					}
					cursor_last_highlight[g] = cursor_on[g];
					if (cursor_on[g] != NULL)
					{
						if ((cursor_on[g]->react_but & OG_BUT_OVER_MASK) != 0)
							og_highlight_button(cursor_on[g]);
						og_arg = cursor_on[g]->arg_over;
						(cursor_on[g]->proc_over)();
					}
				}
			}
		}

	}

	if (got_keyboard)
	{
		if (og_disable_kj_move == 0)
		{
			if (keyreader[0] != NULL)
			{
				for (g = 0; g <= MAX_KEYS; g++)
				{
					if (Keyb3_IsKeyDown(g) != 0)
					{
						og_readchar = key_to_asc(g);
						if (og_readchar == 0)
						{
							og_readkey = g;
						} else {
							og_readkey = 0;
						}
//					if (og_readchar != 0)
//					{
							og_cursor_num = (short)keyreader_cursor[0];
								og_arg = keyreader_arg[0];
							if (keyhandled[g] == 0) keyreader[0]();
//					}
						keyhandled[g] = 1;
					} else {
						keyhandled[g] = 0;
					}
				}
			} else {
				if (Keyb3_IsKeyDown(KEYCODE_ESC) != 0)
				{
					if (keyhandled[KEYCODE_ESC] == 0)
					{
						keyhandled[g] = 1;
						// if only_active is not null, don't escape
						// (the only active should use a keyreader if it
						// wants to be escaped...)
						if (only_active == NULL)
						{
							og_escape = 1;
						} else {
							og_escape = 0;
						}
					}
				} else {
					keyhandled[KEYCODE_ESC] = 0;
					og_escape = 0;
				}
			}
		}

		// NEW: handle hotkeys...
		og_handle_hotkeys();
	}

	//if (g_screen_destroy_requested()) og_terminate = 1;

	return;
}

/* ----------------------------------------------------------- */

void og_warp_cursor_to(int cursor, orvgui_but *to_button)
{
	// assert(cursor >= 0 && cursor <= OG_CURSORS);

	int x, y, last_index;
	orvgui_but *ret;

	cursor_button_index = 0;

	last_index = 0;
	while (cursor_button_index <= last_index)
	{
		bool success = og_get_button_position_by_index(cursor_button_index, &ret, &x, &y, &last_index);
		if (success)
		{
			if (ret == to_button)
			{
				cursor_x[cursor] = x;
				cursor_y[cursor] = y;
				if (got_mouse)
				{
					if ( cursor_control[cursor] & OG_CTRL_MASK_MICE )
					{
						int mID = og_get_mouse_ID(cursor_control[cursor]);

						if (x >= 1024) x = 1024 - 1;
						if (y >= 768) y = 768 - 1;

						int k3_mouse_x = (int)((float)x / og_mouse_sensitivity_x) * og_scale_x / OG_SCALE_MULTIPLIER;
						int k3_mouse_y = (int)((float)y / og_mouse_sensitivity_y) * og_scale_y / OG_SCALE_MULTIPLIER;
						Keyb3_SetMousePos(k3_mouse_x, k3_mouse_y, mID);
					}
				}
				return;
			}
			last_index = cursor_button_index + 1;
		} else {
			last_index = -1;
		}
		cursor_button_index++;
	}

	cursor_button_index = 0;
}

/* ----------------------------------------------------------- */

void og_set_scale(int scale_x, int scale_y)
{
	// values are percentages
	og_scale_x = scale_x;
	og_scale_y = scale_y;
}

/* ----------------------------------------------------------- */

int og_get_scale_x()
{
	return og_scale_x;
}

/* ----------------------------------------------------------- */

int og_get_scale_y()
{
	return og_scale_y;
}

/* ----------------------------------------------------------- */

static void og_find_cursor_on(int curnum)
{
	orvgui_win *win;
	orvgui_win *popupw;
	orvgui_but *but;
	int relx;
	int rely;
	unsigned char curmask;

	curmask = (unsigned char)(1 << curnum);

	cursor_on[curnum] = NULL;

	win = first_win;

	while (win != NULL)
	{
		if ((win->visible) != 0 && ((win->react_cursor) & curmask) != 0)
		{
			if (cursor_x[curnum]>=win->put_x && cursor_y[curnum]>=win->put_y
				&& cursor_x[curnum] < win->put_x+win->sizex
				&& cursor_y[curnum] < win->put_y+win->sizey)
			{
				break;
			}
		}
		win = (orvgui_win*)win->next_sister;
	}

	if (only_active != NULL)
		if (win != only_active && win != error_win)
			win = NULL;

	popupw = NULL;
	if (win == NULL) popupw = win;

#ifdef OG_RAISE_WINDOW_ON_CLICK
	if (win != NULL)
	{
		if (win != first_win)
		{
			if (cursor_obut[curnum] == 0)
			{
				og_raise_window(win);
#ifdef OG_REACT_TOPMOST_WINDOW_ONLY
			} else {
				win = NULL;
#endif
			}
		}
	}
#endif

	if (og_dont_hide_popups == 0)
	{
		if (popupw != NULL || (win != NULL && win->popup == OG_WIN_NOTPOPUP))
		{
			popupw = first_win;

			while (popupw != NULL)
			{
				if ((popupw->popup) == OG_WIN_POPUP && (popupw->visible) != 0)
				{
					og_hide_window(popupw);
				}
				popupw = (orvgui_win*)popupw->next_sister;
			}
		}
	}

	if (win != NULL)
	{
		but = (orvgui_but*)win->first_child;
		relx = cursor_x[curnum] - win->put_x;
		rely = cursor_y[curnum] - win->put_y;

		while (but != NULL)
		{
			if ((but->enabled) != 0)
			{
				if (relx >= but->put_x && rely >= but->put_y
					&& relx < but->put_x+but->sizex
					&& rely < but->put_y+but->sizey)
				{
					og_cursor_num = (short)curnum;
					og_cursor_x = (short)(relx-(short)(but->put_x));
					og_cursor_y = (short)(rely-(short)(but->put_y));
					og_cursor_scrx = (short)cursor_x[curnum];
					og_cursor_scry = (short)cursor_y[curnum];
					og_cursor_but = cursor_but[curnum];
					og_cursor_obut = cursor_obut[curnum];
					og_cursor_on = but;
					break;
				}
			}
			but = (orvgui_but *)but->next_sister;
		}

		if (but == NULL)
		{
			if (win->popup == OG_WIN_POPUPNOCLOSEONBUTTON)
			{
				popupw = first_win;

				while (popupw != NULL)
				{
					if ((popupw->popup) == OG_WIN_POPUP && (popupw->visible) != 0)
					{
						og_hide_window(popupw);
					}
					popupw = (orvgui_win *)popupw->next_sister;
				}
			}
		}

		if (but != NULL)
		{
			cursor_on[curnum] = but;
		} else {
#ifdef OG_MOVE_WITH_BUTTON3
			if ((cursor_but[curnum] & (OG_BUT_1_MASK | OG_BUT_2_MASK))
				== (OG_BUT_1_MASK | OG_BUT_2_MASK)
				|| (cursor_but[curnum] & OG_BUT_3_MASK) == OG_BUT_3_MASK)
			{
				if (win->movable == OG_WIN_MOVABLE
					&& win->dragged == 0)
				{
					win->dragged = 1;
					cursor_drag[curnum] = win;
					cursor_drag_offx[curnum] = relx;
					cursor_drag_offy[curnum] = rely;
				}
			}
#endif
#ifdef OG_MOVE_FROM_TOP_WITH_BUTTON1
			if (cursor_but[curnum] == OG_BUT_1_MASK
				&& cursor_obut[curnum] == 0
				&& rely < OG_TOP_MOVE_HEIGHT)
			{
				if (win->movable == OG_WIN_MOVABLE
					&& win->dragged == 0)
				{
					win->dragged = 1;
					cursor_drag[curnum] = win;
					cursor_drag_offx[curnum] = relx;
					cursor_drag_offy[curnum] = rely;
				}
			}
#endif
		}
	}

	return;
}

/* ----------------------------------------------------------- */

void og_set_only_active(orvgui_win *win)
{
	only_active = win;

	return;
}

/* ----------------------------------------------------------- */

void og_add_keyreader(void (*handler_proc)(void), int id, int cursornum, void *arg)
{
	int g;

	if (handler_proc == NULL)
	{
		return;
	}
	for (g = MAX_KEYREADERS - 1; g > 0; g--)
	{
		keyreader[g] = keyreader[g-1];
		keyreader_id[g] = keyreader_id[g-1];
		keyreader_arg[g] = keyreader_arg[g-1];
		keyreader_cursor[g] = keyreader_cursor[g-1];
	}
	keyreader[0] = handler_proc;
	keyreader_id[0] = id;
	keyreader_arg[0] = arg;
	keyreader_cursor[0] = cursornum;

	return;
}

/* ----------------------------------------------------------- */

void og_remove_keyreader(int id)
{
	int g;
	int delslot;

	for (delslot=0; delslot<MAX_KEYREADERS; delslot++)
	{
		if (keyreader[delslot] != NULL && keyreader_id[delslot] == id)
		{
			for (g=delslot; g<MAX_KEYREADERS-1; g++)
			{
				keyreader[g] = keyreader[g+1];
				keyreader_id[g] = keyreader_id[g+1];
				keyreader_arg[g] = keyreader_arg[g+1];
				keyreader_cursor[g] = keyreader_cursor[g+1];
			}
			keyreader[MAX_KEYREADERS-1] = NULL;
			keyreader_id[MAX_KEYREADERS-1] = -1;
			keyreader_arg[MAX_KEYREADERS-1] = NULL;
			keyreader_cursor[MAX_KEYREADERS-1] = 0;
			break;
		}
	}

	return;
}

/* ----------------------------------------------------------- */

void og_set_cursor_pic(int curnum, IStorm3D_Material *pic)
{
	cursor_pic[curnum] = pic;

	return;
}

/* ----------------------------------------------------------- */

void og_set_cursor_offset(int curnum, int offx, int offy)
{
	cursor_offset_x[curnum] = offx;
	cursor_offset_y[curnum] = offy;

	return;
}

/* ----------------------------------------------------------- */

void og_set_cursor_state(int curnum, int state)
{
	cursor_state[curnum] = state;

	return;
}

/* ----------------------------------------------------------- */

int og_get_cursor_state(int curnum)
{
	return cursor_state[curnum];
}

/* ----------------------------------------------------------- */

void og_set_cursor_control(int curnum, unsigned int ctrlmask)
{

	cursor_control[curnum] = ctrlmask;

	return;
}

unsigned int og_get_cursor_control(int curnum)
{
	return cursor_control[curnum];
}
/* ----------------------------------------------------------- */

void og_set_cursor_control_keyboard(int curnum, int left, int right, int up, int down, int fire, int fire2)
{

	cursor_control[curnum] = OG_CTRL_MASK_KEYBOARD2;
	cursor_control_keys[curnum][0] = left;
	cursor_control_keys[curnum][1] = right;
	cursor_control_keys[curnum][2] = up;
	cursor_control_keys[curnum][3] = down;
	cursor_control_keys[curnum][4] = fire;
	cursor_control_keys[curnum][5] = fire2;
	return;
}

/* ----------------------------------------------------------- */

#ifndef OG_HIDE_OWN_CURSORS
static void og_draw_cursors(void)
{
	int g;
	//int tsizex;
	//int tsizey;

	for (g=0; g<OG_CURSORS; g++)
	{
		if (cursor_control[g] != 0 && cursor_pic[g] != NULL)
		{
			/*
			if (cursor_x[g]+CURSOR_SIZE_X > scr_size_x)
				tsizex=scr_size_x-cursor_x[g];
			else
				tsizex=CURSOR_SIZE_X;
			if (cursor_y[g]+CURSOR_SIZE_Y > scr_size_y)
				tsizey=scr_size_y-cursor_y[g];
			else
				tsizey=CURSOR_SIZE_Y;
			*/

			if (cursor_pic[g] != NULL)
			{
				// note, cursor images are not scaled, position is though
				// now they are...
				og_renderer->Render2D_Picture(cursor_pic[g], 
					VC2((float)(cursor_x[g] * og_scale_x / (float)OG_SCALE_MULTIPLIER + cursor_offset_x[g] * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
					(float)(cursor_y[g] * og_scale_y / (float)OG_SCALE_MULTIPLIER + cursor_offset_y[g] * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
					VC2((float)(CURSOR_SIZE_X * og_scale_x / (float)OG_SCALE_MULTIPLIER), (float)(CURSOR_SIZE_Y * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 1.f, 0.f, false);
					//VC2((float)tsizex, (float)tsizey));
				//og_renderer->Render2D_Picture(cursor_pic[g], 
					//VC2((float)(cursor_x[g] * og_scale_x / OG_SCALE_MULTIPLIER + cursor_offset_x[g]), 
					//(float)(cursor_y[g] * og_scale_y / OG_SCALE_MULTIPLIER + cursor_offset_y[g])), 
					//VC2(CURSOR_SIZE_X, CURSOR_SIZE_Y));
			}
		}
	}

	return;
}
#endif

/* ----------------------------------------------------------- */

bool og_get_button_position_by_index(int index_num, orvgui_but **found_but, int *x, int *y, int *last_index)
{
	orvgui_win *win;
	orvgui_but *but;
	int at_index;

	at_index = 0;
	win = first_win;

	while (win != NULL)
	{
		if (win->visible == 1 && (only_active == NULL
			|| win == only_active))
		{
			but = (orvgui_but *)(win->first_child);

			while (but != NULL)
			{
				if (but->enabled == 1)
				{
					if (at_index == index_num)
					{
						*x = win->put_x + but->put_x + but->sizex / 2;
						*y = win->put_y + but->put_y + but->sizey / 2;
						*found_but = but;
						*last_index = -1;
						return true;
					}
					at_index++;
				}

				but = (orvgui_but *)(but->next_sister);
			}
		}
		win = (orvgui_win *)(win->next_sister);
	}

	*last_index = at_index - 1;
	*found_but = NULL;
	return false;
}

/* ----------------------------------------------------------- */

// call renderscene after this one, as this won't do it
/*
// use og_draw_screen instead
void og_refresh_screen(void)
{
	orvgui_win *rf_win;

#ifdef OG_HIDE_OWN_CURSORS
	// refresh the screen only if some window needs to be refreshed
	int needrefresh = 0;
#endif

	rf_win = last_win;

	while (rf_win != NULL)
	{
		// now we need to draw this always, because that's the way Storm3D works
		//if (rf_win->refresh_flag != 0)
		//{
#ifdef OG_HIDE_OWN_CURSORS
			needrefresh = 1;
#endif
			if (rf_win->visible == 1)
			{
				og_renderer->Render2D_Picture(rf_win->bg_pic, 
					VC2(rf_win->put_x, rf_win->put_y), 
					VC2(rf_win->sizex, rf_win->sizey));
				// not needed anymore
				//og_refresh_overlapping(rf_win);
			}
			rf_win->refresh_flag = 0;
		//}
		rf_win = (orvgui_win *)(rf_win->prev_sister);
	}

#ifndef OG_HIDE_OWN_CURSORS
		og_get_under_cursors();
		og_draw_cursors();
#endif

#ifdef OG_HIDE_OWN_CURSORS
		//if (g_screen_refresh_requested()) needrefresh = 1;

		if (needrefresh)
#endif

	//g_dump_to_scr(vscr);

#ifndef OG_HIDE_OWN_CURSORS
		og_erase_cursors();
#endif

	return;
}
*/

/* ----------------------------------------------------------- */

void og_draw_screen(void)
{
	orvgui_win *rf_win;
	/*
	int repx;
	int repy;
	int repsizex;
	int repsizey;
	*/

	/*
	// this was for the background drawing, no longer needed
	if (vscr_bg_pic != NULL)
	{
		if (vscr_bg_pic->PicSizeX >= scr_size_x
			&& vscr_bg_pic->PicSizeY >= scr_size_y)
				{
					// show the whole bg picture or upper left corner of it
				g_copyimg(vscr_bg_pic, vscr, 0, 0, 0, 0, scr_size_x, scr_size_y);
				} else {
						if (vscr_bg_tile)
						{
							// tile the bg picture aligned to upper left corner
							for (repy = 0; repy < scr_size_y; repy += vscr_bg_pic->PicSizeY)
							{
									for (repx = 0; repx < scr_size_x; repx += vscr_bg_pic->PicSizeX)
									{
										repsizex = vscr_bg_pic->PicSizeX;
										repsizey = vscr_bg_pic->PicSizeY;
											if (repx + repsizex > scr_size_x)
												repsizex = scr_size_x - repx;
											if (repy + repsizey > scr_size_y)
												repsizey = scr_size_y - repy;
								g_copyimg(vscr_bg_pic, vscr, 0, 0, repx, repy, repsizex, repsizey);
									}
							}
						} else {
							// first clear the vscr then center the bg picture
				g_clear_picture(vscr, vscr_bg_col);
					g_copyimg(vscr_bg_pic, vscr, 0, 0,
						(scr_size_x - vscr_bg_pic->PicSizeX) / 2,
						(scr_size_y - vscr_bg_pic->PicSizeY) / 2,
						vscr_bg_pic->PicSizeX, vscr_bg_pic->PicSizeY);
						}
				}
	} else {
		g_clear_picture(vscr, vscr_bg_col);
	}
*/

	rf_win = last_win;

	while (rf_win != NULL)
	{
		//og_refresh_window(rf_win);
		og_draw_window(rf_win);
		rf_win = (orvgui_win *)(rf_win->prev_sister);
	}

#ifndef OG_HIDE_OWN_CURSORS
		og_draw_cursors();
#endif

	return;
}

/* ----------------------------------------------------------- */

/*
void og_set_bg_screen(picture *pic)
{
		vscr_bg_pic = pic;

		og_draw_screen();

	return;
}
*/

/* ----------------------------------------------------------- */

/*
void og_set_default_pics()
{
		og_set_pics(self_loaded_def_pics);
}
*/

/* ----------------------------------------------------------- */

/*
void og_set_pics(picture *pic)
{
		if (pic == NULL) return;

		def_pics = pic;

		def_bg_col = g_pget(def_pics, 0, 0);
		transp_col = g_pget(def_pics, 1, 0);
		def_fg_col = g_pget(def_pics, 2, 0);
		def_selection_col = g_pget(def_pics, 3, 0);
		def_cursor_col = g_pget(def_pics, 4, 0);

		og_draw_screen();
		g_pal_to_scr(def_pics);

	return;
}
*/

/* ----------------------------------------------------------- */

/*
void og_set_bg_tile(int tile)
{
		vscr_bg_tile = tile;

		og_draw_screen();

	return;
}
*/

/* ----------------------------------------------------------- */

orvgui_but *og_create_button(orvgui_win *parent, unsigned char type,
	short x, short y, short sizex, short sizey, bool clip_to_window)
{
	orvgui_but *new_but;

	if (parent == NULL)
	{
		og_show_error("og_create_button - No parent window.");
		return NULL;
	}
	if (clip_to_window && ( sizex > parent->sizex || sizey > parent->sizey ) )
	{
		og_show_error("og_create_button - Button does not fit window.");
		if (sizex > parent->sizex) sizex = parent->sizex;
		if (sizey > parent->sizey) sizey = parent->sizey;
	}
	if (x < 0 && clip_to_window ) x = 0;
	if (y < 0 && clip_to_window ) y = 0;
	if (x+sizex > parent->sizex && clip_to_window ) x = (short)(parent->sizex - sizex);
	if (y+sizey > parent->sizey && clip_to_window ) y = (short)(parent->sizey - sizey);

	new_but = new orvgui_but();
	/*
	// not needed because new throws exception unlike malloc
	if (new_but == NULL)
	{
		og_show_error("og_create_button - Out of memory.");
		return NULL;
	}
	*/

	new_but->type = type;
	new_but->react_but = OG_BUT_ALL_MASK;
	new_but->enabled = 1;
	new_but->pressed = 0;
	new_but->highlighted = 0;
	new_but->font_disabled = NULL;
	new_but->font_down = NULL;
	new_but->font_highlighted = NULL;
	new_but->get_pic = NULL;
	new_but->get_picdown = NULL;
	new_but->get_picdisabled = NULL;
	//new_but->put_pic = parent->get_pic;
	//new_but->get_x = 0;
	//new_but->get_y = 0;
	new_but->put_x = x;
	new_but->put_y = y;
	new_but->text = NULL;
	new_but->text_h_align = OG_H_ALIGN_CENTER;
	new_but->text_v_align = OG_V_ALIGN_MIDDLE;
	new_but->linebreaks = 0;
	new_but->text_pixwidth = 0;
	new_but->text_pixheight = 0;
	new_but->sizex = sizex;
	new_but->sizey = sizey;
	new_but->clipleftx = 0;
	new_but->cliptopy = 0;
	new_but->cliprightx = 100;
	new_but->clipbottomy = 100;
	new_but->repeat_x = 1.0f;
	new_but->repeat_y = 1.0f;
	new_but->scroll_x = 0.0f;
	new_but->scroll_y = 0.0f;
	new_but->parent = parent;
	new_but->wrap = false;
	new_but->next_sister = parent->first_child;
	new_but->prev_sister = NULL;
	new_but->proc_press = og_empty_proc;
	new_but->proc_click = og_empty_proc;
	new_but->proc_out = og_empty_proc;
	new_but->proc_over = og_empty_proc;
	new_but->proc_leave = og_empty_proc;
	new_but->proc_hold = og_empty_proc;
	new_but->arg_press = NULL;
	new_but->arg_click = NULL;
	new_but->arg_out = NULL;
	new_but->arg_over = NULL;
	new_but->arg_leave = NULL;
	new_but->arg_hold = NULL;
	//new_but->proc_on = og_empty_proc;
	new_but->rotation = 0;
	new_but->alpha = 1.0f;
	new_but->clip_to_window = clip_to_window;
	new_but->vertices = NULL;
	new_but->num_vertices = 0;

	if (parent->first_child != NULL)
		((orvgui_but *)(parent->first_child))->prev_sister = new_but;

	parent->first_child = new_but;
	
	for (int i = 0; i < OG_MAX_HOTKEYS; i++)
	{
		new_but->hotKeys[i] = 0;
	}

	// not needed anymore
	//og_refresh_button(new_but);

	return new_but;
}

/* ----------------------------------------------------------- */

void og_delete_button(orvgui_but *but)
{
	int i;
	// can't do this check because the cursor may have the 
	// last_highlight set, even if the button is not highlighted
	// (due to reactmask not allowing that)
	//if (but->highlighted != 0)
	//{
		for (i = 0; i < OG_CURSORS; i++)
		{
			if (cursor_last_highlight[i] == but)
				cursor_last_highlight[i] = NULL;
		}
	//}
	for (i = 0; i < OG_CURSORS; i++)
	{
		if (cursor_on[i] == but)
			cursor_on[i] = NULL;
	}
	if ((but->parent)->first_child == but)
		(but->parent)->first_child = but->next_sister;

	if (but->next_sister != NULL)
		((orvgui_but *)(but->next_sister))->prev_sister = but->prev_sister;
	if (but->prev_sister != NULL)
		((orvgui_but *)(but->prev_sister))->next_sister = but->next_sister;

	//og_draw_window(but->parent);

	if (but->text != NULL)
	{
		delete [] but->text;
		but->text = NULL;
	}

	if(but->vertices != NULL)
	{
		delete [] but->vertices;
		but->vertices = NULL;
		but->num_vertices = 0;
	}

	//free(but);
	delete but;

	return;
}

/* ----------------------------------------------------------- */

/*
// not needed anymore
void og_refresh_button(orvgui_but *but)
{
	short tx;
	short ty;

	if (but->get_pic != NULL)
	{

	if (but->pressed == 1)
		{
		g_copyimg_transp(but->get_pic, but->put_pic,
			but->get_x, but->get_y+(but->sizey)*1,
			but->put_x, but->put_y, but->sizex, but->sizey, transp_col);
	} else {
			if (but->enabled == 0)
				{
			g_copyimg_transp(but->get_pic, but->put_pic,
				but->get_x, but->get_y+(but->sizey)*2,
				but->put_x, but->put_y, but->sizex, but->sizey, transp_col);
		} else {
			g_copyimg_transp(but->get_pic, but->put_pic, but->get_x, but->get_y,
				but->put_x, but->put_y, but->sizex, but->sizey, transp_col);
				}
		}

		}

		if (but->type == OG_BUT_PIC_AND_TEXT)
		{
			if (but->text != NULL)
			{
				tx = 0;
				ty = 0;

				if (but->text_h_align == OG_H_ALIGN_LEFT)
					tx = 0;
				if (but->text_h_align == OG_H_ALIGN_CENTER)
					tx = (but->sizex - strlen(but->text) * OG_FONT_SIZEX) / 2;
				if (but->text_h_align == OG_H_ALIGN_RIGHT)
					tx = (but->sizex - strlen(but->text) * OG_FONT_SIZEX);

				if (but->text_v_align == OG_V_ALIGN_TOP)
					ty = 0;
				if (but->text_v_align == OG_V_ALIGN_MIDDLE)
					ty = (but->sizey - OG_FONT_SIZEY) / 2;
				if (but->text_v_align == OG_V_ALIGN_BOTTOM)
					ty = (but->sizey - OG_FONT_SIZEY);

				og_write_text_transp(but->parent, but->put_x + tx, but->put_y + ty, but->text);
			}
		}

	return;
}
*/

/* ----------------------------------------------------------- */

void og_draw_button(orvgui_but *but)
{
	IStorm3D_Material *tmp = NULL;
	IStorm3D_Font* font = but->font;
	COL font_color = but->font_color;

	if (but->enabled == 0)
	{
		tmp = but->get_picdisabled;
		if( but->font_disabled != NULL )
		{
			font = but->font_disabled;
			font_color = but->font_disabled_color;
		}

	} else {
		if (but->pressed == 0)
		{
			if (but->highlighted != 0)
			{
				tmp = but->get_pichighlighted;
				if( but->font_highlighted != NULL )
				{
					font = but->font_highlighted;
					font_color = but->font_highlighted_color;
				}

			} else {
				tmp = but->get_pic;
			}
		} else {
			tmp = but->get_picdown;
			if( but->font_down != NULL )
			{
				font = but->font_down;
				font_color = but->font_down_color;
			}
		} 
	}
	if (tmp != NULL)
	{
		float tx1 = but->scroll_x + but->repeat_x * ((float)but->clipleftx / 100.0f);
		float ty1 = but->scroll_y + but->repeat_y * ((float)but->cliptopy / 100.0f);
		float tx2 = but->scroll_x + but->repeat_x * ((float)but->cliprightx / 100.0f);
		float ty2 = but->scroll_y + but->repeat_y * ((float)but->clipbottomy / 100.0f);

#ifdef PROJECT_SHADOWGROUNDS
		if (strstr(tmp->GetName(), "video material"))
		{
			tx1 = but->scroll_x;
			ty1 = but->scroll_y;
			tx2 = but->scroll_x + (1.0f * but->repeat_x);
			ty2 = but->scroll_y + (1.0f * but->repeat_y);
		}
#endif

		if(but->vertices == NULL || but->num_vertices == 0)
		{
			og_renderer->Render2D_Picture(tmp, 
			VC2((float)((but->parent->put_x + but->put_x + (float)((but->clipleftx * but->sizex ) / 100)) * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
			(float)((but->parent->put_y + but->put_y + (float)((but->cliptopy * but->sizey ) / 100)) * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
			VC2((float)but->sizex * (but->cliprightx - but->clipleftx) * og_scale_x / 100 / (float)OG_SCALE_MULTIPLIER, 
			(float)but->sizey * (but->clipbottomy-but->cliptopy) * og_scale_y / 100 / (float)OG_SCALE_MULTIPLIER),
			but->parent->alpha * but->alpha, but->rotation, tx1, ty1, tx2, ty2, but->wrap);
		}
		else
		{
			VXFORMAT_2D *vertices = new VXFORMAT_2D[but->num_vertices];
			for(int i = 0; i < but->num_vertices; i++)
			{
				vertices[i].position.x = (float)((but->parent->put_x + but->put_x + but->vertices[i].x) * og_scale_x / (float)OG_SCALE_MULTIPLIER);
				vertices[i].position.y = (float)((but->parent->put_y + but->put_y + but->vertices[i].y) * og_scale_y / (float)OG_SCALE_MULTIPLIER);
				vertices[i].position.z = 0.0f;
				vertices[i].rhw = 1.0f;
				vertices[i].color = but->vertices[i].col;
				vertices[i].texcoords.x = but->scroll_x + but->repeat_x * but->vertices[i].x / (float)but->sizex;
				vertices[i].texcoords.y = but->scroll_y + but->repeat_y * but->vertices[i].y / (float)but->sizey;
			}

			og_renderer->Render2D_Picture(tmp, vertices, but->num_vertices, but->parent->alpha * but->alpha, but->wrap);
			delete[] vertices;
		}
	}

	debugMaterialInit();
	if(og_visualize_windows)
	{
		/*float tx1 = but->scroll_x + but->repeat_x * ((float)but->clipleftx / 100.0f);
		float ty1 = but->scroll_y + but->repeat_y * ((float)but->cliptopy / 100.0f);
		float tx2 = but->scroll_x + but->repeat_x * ((float)but->cliprightx / 100.0f);
		float ty2 = but->scroll_y + but->repeat_y * ((float)but->clipbottomy / 100.0f);*/

		IStorm3D_Material *mat = debug_button_material;
		if(!but->enabled) mat = debug_button_disabled_material;

		VC2 pos((float)((but->parent->put_x + but->put_x + (float)((but->clipleftx * but->sizex ) / 100)) * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
		(float)((but->parent->put_y + but->put_y + (float)((but->cliptopy * but->sizey ) / 100)) * og_scale_y / (float)OG_SCALE_MULTIPLIER));

		VC2 size((float)but->sizex * (but->cliprightx - but->clipleftx) * og_scale_x / 100 / (float)OG_SCALE_MULTIPLIER, 
		(float)but->sizey * (but->clipbottomy-but->cliptopy) * og_scale_y / 100 / (float)OG_SCALE_MULTIPLIER);

		og_renderer->Render2D_Picture(mat, pos, size, 0.1f, but->rotation, 0, 0, 0, 0);

		// top edge
		og_renderer->Render2D_Picture(mat, pos, VC2(size.x, 1.0f), 1.0f, but->rotation, 0, 0, 0, 0);

		// left edge
		og_renderer->Render2D_Picture(mat, pos, VC2(1.0f, size.y), 1.0f, but->rotation, 0, 0, 0, 0);

		// right edge
		og_renderer->Render2D_Picture(mat, pos + VC2(size.x, 0), VC2(1.0f, size.y), 1.0f, but->rotation, 0, 0, 0, 0);

		// bottom edge
		og_renderer->Render2D_Picture(mat, pos + VC2(0, size.y), VC2(size.x, 1.0f), 1.0f, but->rotation, 0, 0, 0, 0);
	}

	if (but->type == OG_BUT_PIC_AND_TEXT)
	{
		if (but->text != NULL && font != NULL)
		{
			int tx = 0;
			int ty = 0;

			if (but->text_h_align == OG_H_ALIGN_LEFT)
			{
				tx = OG_TEXT_H_ALIGN_LEFT_PAD;
			}
			if (but->text_h_align == OG_H_ALIGN_CENTER)
			{
				tx = (but->sizex - but->text_pixwidth) / 2;
			}
			if (but->text_h_align == OG_H_ALIGN_RIGHT)
			{
				tx = (but->sizex - but->text_pixwidth - OG_TEXT_H_ALIGN_RIGHT_PAD);
			}

			if (but->text_v_align == OG_V_ALIGN_TOP)
			{
				ty = OG_TEXT_V_ALIGN_TOP_PAD;
			}
			if (but->text_v_align == OG_V_ALIGN_MIDDLE)
			{
				ty = (but->sizey - but->text_pixheight) / 2;
			}
			if (but->text_v_align == OG_V_ALIGN_BOTTOM)
			{
				ty = (but->sizey - but->text_pixheight - OG_TEXT_V_ALIGN_BOTTOM_PAD);
			}


			if (but->linebreaks)
			{
				int i;
				char *txt = but->text;
				int txtlen = strlen(but->text);
				int brpos = 0;
				int curline = 0;
				for (i = 0; i < txtlen + 1; i++)
				{
					if (txt[i] == '\n' || txt[i] == '\0')
					{
						char tmp = txt[i];
						txt[i] = '\0';
						og_write_text_transp(but->parent, font, 
							(short)(but->put_x + tx), 
							(short)(but->put_y + ty + curline * but->text_pixheight), &txt[brpos],
							(short)(but->fontwidth), 
							(short)(but->fontheight), but->alpha, font_color); 
						txt[i] = tmp;
						curline++;
						brpos = i + 1;
					}
				}
			} else {
				og_write_text_transp(but->parent, font, (short)(but->put_x + tx), 
					(short)(but->put_y + ty), but->text,
					(short)(but->fontwidth), 
					(short)(but->fontheight), but->alpha, font_color); 
			}
		}
	}
}

/* ----------------------------------------------------------- */

void og_move_button(orvgui_but *but, short x, short y)
{
	orvgui_win *parent;

	parent = but->parent;

	if (x < 0 && but->clip_to_window ) x = 0;
	if (y < 0  && but->clip_to_window ) y = 0;
	if (x + but->sizex > parent->sizex  && but->clip_to_window ) x = (short)(parent->sizex - but->sizex);
	if (y + but->sizey > parent->sizey  && but->clip_to_window ) y = (short)(parent->sizey - but->sizey);

	but->put_x = x;
	but->put_y = y;

	//og_draw_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

// added by Pete
void og_move_button_by( orvgui_but *but, short bx, short by )
{
	orvgui_win* parent;

	parent = but->parent;

	int x = but->put_x + bx;
	int y = but->put_y + by;

	if (x < 0  && but->clip_to_window ) x = 0;
	if (y < 0   && but->clip_to_window ) y = 0;
	if (x + but->sizex > parent->sizex  && but->clip_to_window ) x = (short)(parent->sizex - but->sizex);
	if (y + but->sizey > parent->sizey  && but->clip_to_window ) y = (short)(parent->sizey - but->sizey);

	but->put_x = x;
	but->put_y = y;

	return;
}

/* ----------------------------------------------------------- */

void og_resize_button(orvgui_but *but, short sizex, short sizey)
{
	orvgui_win *parent;

	parent = but->parent;

	if (sizex > parent->sizex && but->clip_to_window ) sizex = parent->sizex;
	if (sizey > parent->sizey && but->clip_to_window ) sizey = parent->sizey;

	if (but->put_x + sizex > parent->sizex && but->clip_to_window ) but->put_x=(short)(parent->sizex - sizex);
	if (but->put_y + sizey > parent->sizey && but->clip_to_window ) but->put_y=(short)(parent->sizey - sizey);

	but->sizex = sizex;
	but->sizey = sizey;

	// og_draw_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_transparency_button(orvgui_but *but, int transparency)
{
	but->alpha = 1.0f - float(transparency) / 100.0f;
	if (but->alpha < 0.0f)
		but->alpha = 0.0f;
	if (but->alpha > 1.0f)
		but->alpha = 1.0f;
	return;
}

/* ----------------------------------------------------------- */

void og_rotate_button(orvgui_but *but, float angle)
{
	but->rotation = angle;
}

/* ----------------------------------------------------------- */

void og_enable_button(orvgui_but *but)
{
	but->enabled = 1;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_disable_button(orvgui_but *but)
{
	but->enabled = 0;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_press_button(orvgui_but *but)
{
	but->pressed = 1;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_unpress_button(orvgui_but *but)
{
	but->pressed = 0;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

// note: many cursors may highlight a button...
// this is why ++, not just =1. (exact value needed for proper deletion)
void og_highlight_button(orvgui_but *but)
{
	but->highlighted++;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

// WARNING: calling this for a button that has been highlighted by a cursor
// may have undefined results 
// (may invalidate the cursor_last_highligh pointer)
void og_unhighlight_button(orvgui_but *but)
{
	but->highlighted--;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_react_button(orvgui_but *but, unsigned int mask)
{
	but->react_but = mask;

	return;
}

/* ----------------------------------------------------------- */

void og_set_pic_button(orvgui_but *but, IStorm3D_Material *pic, 
	IStorm3D_Material *picdown, IStorm3D_Material *picdisabled,
	IStorm3D_Material *pichighlighted)
{
	but->get_pic = pic;
	but->get_picdown = picdown;
	but->get_picdisabled = picdisabled;
	but->get_pichighlighted = pichighlighted;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_text_button(orvgui_but *but, IStorm3D_Font *fnt, const COL &fnt_color, unsigned char h_align, unsigned char v_align, const char *text, int pixwidth, int pixheight, int fontwidth, int fontheight)
{
	but->text_h_align = h_align;
	but->text_v_align = v_align;
	but->font = fnt;
	but->font_color = fnt_color;
	og_set_text_button(but, text, pixwidth, pixheight, fontwidth, fontheight);

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_text_button(orvgui_but *but, const char *text, int pixwidth, int pixheight, int fontwidth, int fontheight)
{
	if (but->text != NULL) delete [] but->text;
	if (text == NULL)
	{
		but->text = NULL;
	} else {
		but->text = new char[strlen(text) + 1];
		strcpy(but->text, text);
	}
	but->text_pixwidth = pixwidth;
	but->text_pixheight = pixheight;
	but->fontwidth = fontwidth;
	but->fontheight = fontheight;

	return;
}

/* ----------------------------------------------------------- */

void og_set_linebreaks_button(orvgui_but *but)
{
	but->linebreaks = 1;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_nolinebreaks_button(orvgui_but *but)
{
	but->linebreaks = 0;

	//og_refresh_button(but);
	//og_refresh_window(but->parent);

	return;
}

/* ----------------------------------------------------------- */

void og_set_font_button(orvgui_but *but, IStorm3D_Font *fnt)
{
	but->font = fnt;

	return;
}

/* ----------------------------------------------------------- */

//void og_set_fonts_button(orvgui_but *but, IStorm3D_Font *fnt, IStorm3D_Font *down, IStorm3D_Font *disa, IStorm3D_Font *high )
void og_set_fonts_button(orvgui_but *but, IStorm3D_Font *fnt, const COL &fnt_color, IStorm3D_Font *down, const COL &down_color, IStorm3D_Font *disa, const COL &disa_color, IStorm3D_Font *high, const COL &high_color)
{
	but->font = fnt;
	but->font_down = down;
	but->font_disabled = disa;
	but->font_highlighted = high;

	but->font_color = fnt_color;
	but->font_down_color = down_color;
	but->font_disabled_color = disa_color;
	but->font_highlighted_color = high_color;

	return;
}

/* ----------------------------------------------------------- */

void og_set_h_align_button(orvgui_but *but, unsigned char h_align)
{
	but->text_h_align = h_align;

	return;
}

/* ----------------------------------------------------------- */

void og_set_v_align_button(orvgui_but *but, unsigned char v_align)
{
	but->text_v_align = v_align;

	return;
}

/* ----------------------------------------------------------- */

void og_set_press_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_press = handler_proc;
	but->arg_press = handler_arg;

	return;
}

/* ----------------------------------------------------------- */

void og_set_click_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_click = handler_proc;
	but->arg_click = handler_arg;

	return;
}

/* ----------------------------------------------------------- */

void og_set_out_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_out = handler_proc;
	but->arg_out = handler_arg;

	return;
}

/* ----------------------------------------------------------- */

void og_set_over_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_over = handler_proc;
	but->arg_over = handler_arg;

	return;
}

/* ----------------------------------------------------------- */

void og_set_leave_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_leave = handler_proc;
	but->arg_leave = handler_arg;

	return;
}

/* ----------------------------------------------------------- */

void og_set_hold_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg)
{
	but->proc_hold = handler_proc;
	but->arg_hold = handler_arg;

	return;
}

/* ----------------------------------------------------------- */
/*
void og_set_on_button(orvgui_but *but, void (*handler_proc)())
{
	but->proc_on = handler_proc;

	return;
}
*/
/* ----------------------------------------------------------- */

orvgui_win *og_create_window(unsigned char type, short x, short y,
	short sizex, short sizey, IStorm3D_Material *bg_pic)
{
	orvgui_win *new_win;

	/*
	if (sizex * og_scale_x / OG_SCALE_MULTIPLIER > scr_size_x 
		|| sizey * og_scaly_y / OG_SCALE_MULTIPLIER > scr_size_y)
	{
		og_show_error("og_create_window - Window does not fit in screen.");
		if (sizex * og_scaly_x / OG_SCALE_MULTIPLIER > scr_size_x) 
			sizex = (short)scr_size_x;
		if (sizey * og_scaly_y / OG_SCALE_MULTIPLIER > scr_size_y) sizey = (short)scr_size_y;
	}
	*/
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if ((x+sizex) > (scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x))
		x = (short)(scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x - sizex);
	if ((y+sizey) > (scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y))
		y = (short)(scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y - sizey);
	//if (x+sizex > scr_size_x) x = (short)(scr_size_x - sizex);
	//if (y+sizey > scr_size_y) y = (short)(scr_size_y - sizey);
	//if (x < 0) x = 0;
	//if (y < 0) y = 0;

	new_win = new orvgui_win;

		/*
		// new throws exception unlike malloc, this is not needed anymore
		if (new_win == NULL)
		{
			og_show_error("og_create_window - Out of memory.");
			return NULL;
		}
		*/

		/*
		new_pic = g_alloc_picture(PIC_TYPE_8BIT, PAL_TYPE_NONE, sizex, sizey);

		if (new_pic == NULL)
		{
			og_show_error("og_create_window - Failed to allocate picture.");
			return NULL;
		}
		*/

		/*
		if (bg_pic != NULL)
		{
				if (bg_pic->PicSizeX >= sizex
					&& bg_pic->PicSizeY >= sizey)
				{
			g_copyimg(bg_pic, new_pic, 0, 0, 0, 0, sizex, sizey);
				} else {
			g_clear_picture(new_pic, def_bg_col);
				}
		} else {
		g_clear_picture(new_pic, def_bg_col);
		}
		*/

	new_win->type = type;
	new_win->react_cursor = OG_CURSOR_ALL_MASK;
	new_win->visible = 1;
	new_win->refresh_flag = 0;
	new_win->movable = OG_WIN_UNMOVABLE;
	new_win->popup = OG_WIN_NOTPOPUP;
	new_win->dragged = 0;
	new_win->bg_pic = bg_pic;
	new_win->alpha = 1.0f;
	new_win->scroll_x = 0.0f;
	new_win->scroll_y = 0.0f;
	new_win->bg_repeat_factor_x = 1.0f;
	new_win->bg_repeat_factor_y = 1.0f;
	new_win->wrap = false;

	//new_win->get_pic = new_pic;

	//new_win->put_pic = vscr;
	new_win->put_x = x;
	new_win->put_y = y;
	new_win->sizex = sizex;
	new_win->sizey = sizey;
	new_win->next_sister = first_win;
	new_win->prev_sister = NULL;
	new_win->first_child = NULL;
	new_win->move_bound = OG_MOVE_BOUND_ALL_IN_SCREEN;

	if (first_win != NULL)
		first_win->prev_sister = new_win;
	if (last_win == NULL)
		last_win = new_win;

	first_win = new_win;

		//og_refresh_window(new_win);

	return new_win;
}

/* ----------------------------------------------------------- */

void og_delete_window(orvgui_win *win)
{
	//int crashcounter = 0;

	while (win->first_child != NULL)
	{
		og_delete_button((orvgui_but *)win->first_child);
		//crashcounter++;
		//if (crashcounter > 1000) exit(200);
	}

	if (first_win == win)
		first_win = (orvgui_win *)win->next_sister;
	if (last_win == win)
	 last_win = (orvgui_win *)win->prev_sister;

	if (win->next_sister != NULL)
		((orvgui_win *)(win->next_sister))->prev_sister = win->prev_sister;
	if (win->prev_sister != NULL)
		((orvgui_win *)(win->prev_sister))->next_sister = win->next_sister;

	//og_draw_screen();

	//free(win->get_pic);
	//g_delete_picture(win->get_pic);

	//free(win);
	delete win;

	return;
}

/* ----------------------------------------------------------- */

/*
// not needed anymore
void og_refresh_window(orvgui_win *win)
{
	win->refresh_flag = 1;

	return;
}
*/

/* ----------------------------------------------------------- */

/*
// not needed anymore
void og_refresh_overlapping(orvgui_win *win)
{
	orvgui_win *twin;

	twin = win->prev_sister;

	while (twin != NULL)
	{
		if (twin->put_x < win->put_x+win->sizex
			&& twin->put_y < win->put_y+win->sizey
			&& win->put_x < twin->put_x+twin->sizex
			&& win->put_y < twin->put_y+twin->sizey)
			twin->refresh_flag = 1;
			twin = twin->prev_sister;
	}

	return;
}
*/

/* ----------------------------------------------------------- */

void og_draw_window(orvgui_win *win)
{

	orvgui_but *rf_but;

	debugMaterialInit();
	if(og_visualize_windows)
	{
		IStorm3D_Material *mat = debug_window_material;
		if (!win->visible) mat = debug_window_hidden_material;

		VC2 pos((float)(win->put_x * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
		(float)(win->put_y * og_scale_y / (float)OG_SCALE_MULTIPLIER));

		VC2 size((float)(win->sizex * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
		(float)(win->sizey * og_scale_y / (float)OG_SCALE_MULTIPLIER));

		og_renderer->Render2D_Picture(mat, pos, size, 0.1f, 0.0f, 0, 0, 0, 0);

		// top edge
		og_renderer->Render2D_Picture(mat, pos, VC2(size.x, 1.0f), 1.0f, 0.0f, 0, 0, 0, 0);

		// left edge
		og_renderer->Render2D_Picture(mat, pos, VC2(1.0f, size.y), 1.0f, 0.0f, 0, 0, 0, 0);

		// right edge
		og_renderer->Render2D_Picture(mat, pos + VC2(size.x, 0), VC2(1.0f, size.y), 1.0f, 0.0f, 0, 0, 0, 0);

		// bottom edge
		og_renderer->Render2D_Picture(mat, pos + VC2(0, size.y), VC2(size.x, 1.0f), 1.0f, 0.0f, 0, 0, 0, 0);
	}

	if (win->visible)
	{

		if (win->bg_pic != NULL)
		{
			og_renderer->Render2D_Picture(win->bg_pic, 
				VC2((float)(win->put_x * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)(win->put_y * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
				VC2((float)(win->sizex * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)(win->sizey * og_scale_y / (float)OG_SCALE_MULTIPLIER)),
				win->alpha, 0.0f, win->scroll_x, win->scroll_y, win->scroll_x + (1.0f * win->bg_repeat_factor_x), win->scroll_y + (1.0f * win->bg_repeat_factor_y), win->wrap);
		}
		if (win->first_child != NULL)
		{
			rf_but = (orvgui_but *)(win->first_child);

			while (rf_but->next_sister != NULL)
			{
				rf_but = (orvgui_but *)(rf_but->next_sister);
			}
			while (rf_but != NULL)
			{
				og_draw_button(rf_but);
				//og_refresh_button(rf_but);
				rf_but = (orvgui_but *)(rf_but->prev_sister);
			}
		}
	}

	return;
}

/* ----------------------------------------------------------- */

void og_raise_window(orvgui_win *win)
{
	if (first_win == win) return;

	if (last_win == win)
		last_win = (orvgui_win *)win->prev_sister;

	if (win->next_sister != NULL)
		((orvgui_win *)(win->next_sister))->prev_sister = win->prev_sister;
	if (win->prev_sister != NULL)
		((orvgui_win *)(win->prev_sister))->next_sister = win->next_sister;

	first_win->prev_sister = win;

	win->prev_sister = NULL;
	win->next_sister = first_win;

	first_win = win;

	//og_draw_screen();

	return;
}

/* ----------------------------------------------------------- */

void og_lower_window(orvgui_win *win)
{
	if (last_win == win) return;

	if (first_win == win)
		first_win = (orvgui_win *)win->next_sister;

	if (win->next_sister != NULL)
		((orvgui_win *)(win->next_sister))->prev_sister = win->prev_sister;
	if (win->prev_sister != NULL)
		((orvgui_win *)(win->prev_sister))->next_sister = win->next_sister;

	last_win->next_sister = win;

	win->next_sister = NULL;
	win->prev_sister = last_win;

	last_win = win;

	//og_draw_screen();

	return;
}

/* ----------------------------------------------------------- */

void og_move_window(orvgui_win *win, short x, short y)
{
	if (win->move_bound == OG_MOVE_BOUND_ALL_IN_SCREEN)
	{
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if ((x+win->sizex) > scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x) 
			x = (short)(scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x - win->sizex);
		if ((y+win->sizey)> scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y ) 
			y = (short)(scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y - win->sizey);
	}
	else if (win->move_bound == OG_MOVE_BOUND_PART_IN_SCREEN)
	{
		if (x < -(win->sizex-1)) x = -(win->sizex-1);
		if (y < -(win->sizey-1)) y = -(win->sizey-1);
		if ((x+1) > scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x) 
			x = (short)(scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x - 1);
		if ((y+1) > scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y ) 
			y = (short)(scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y - 1); 	
	}

	win->put_x = x;
	win->put_y = y;

	//og_draw_screen();

	return;
}

/* ----------------------------------------------------------- */

void og_force_move_window(orvgui_win *win, short x, short y)
{
	// does not check for boundaries.

	win->put_x = x;
	win->put_y = y;

	return;
}

/* ----------------------------------------------------------- */

void og_set_background_window(orvgui_win *win, IStorm3D_Material *bg_pic)
{
	win->bg_pic = bg_pic;
}

/* ----------------------------------------------------------- */

void og_resize_window(orvgui_win *win, short sizex, short sizey)
{
	//picture *new_pic;

	if (win->move_bound == OG_MOVE_BOUND_ALL_IN_SCREEN)
	{
		if (win->put_x < 0) win->put_x = 0;
		if (win->put_y < 0) win->put_y = 0;
		if ((win->put_x+sizex) > scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x) 
			win->put_x = (short)(scr_size_x * OG_SCALE_MULTIPLIER / og_scale_x - sizex);
		if ((win->put_y+sizey)> scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y ) 
			win->put_y = (short)(scr_size_y * OG_SCALE_MULTIPLIER / og_scale_y - sizey);
	}
	// TODO: else if (win->move_bound == OG_MOVE_BOUND_PART_IN_SCREEN)


	//if (sizex > scr_size_x) sizex = (short)scr_size_x;
	//if (sizey > scr_size_y) sizey = (short)scr_size_y;

	//if (win->put_x + sizex > scr_size_x) win->put_x = (short)(scr_size_x - sizex);
	//if (win->put_y + sizey > scr_size_y) win->put_y = (short)(scr_size_y - sizey);

	win->sizex = sizex;
	win->sizey = sizey;

	/*
	g_delete_picture(win->get_pic);

	new_pic = g_alloc_picture(PIC_TYPE_8BIT, PAL_TYPE_NONE, sizex, sizey);

	if (new_pic == NULL)
	{
		og_show_error("og_resize_window - Failed to allocate picture.");
			return;
	}
	win->get_pic = new_pic;
	*/

	//og_draw_window(win);
	//og_draw_screen();

	return;
}

/* ----------------------------------------------------------- */

void og_hide_window(orvgui_win *win)
{
	win->visible = 0;

	//og_draw_screen();

	return;
}

/* ----------------------------------------------------------- */

void og_show_window(orvgui_win *win)
{
	win->visible = 1;

	//og_refresh_window(win);

	return;
}

/* ----------------------------------------------------------- */

void og_set_react_window(orvgui_win *win, unsigned char mask)
{
	win->react_cursor = mask;

	return;
}

/* ----------------------------------------------------------- */

void og_set_bg_window(orvgui_win *win, IStorm3D_Material *bg)
{
	win->bg_pic = bg;

	//og_draw_window(win);
	//og_refresh_window(win);

	return;
}

/* ----------------------------------------------------------- */

void og_set_transparency_window(orvgui_win *win, int transparency)
{
	win->alpha = 1.0f - float(transparency) / 100.0f;
	if (win->alpha < 0.0f)
		win->alpha = 0.0f;
	if (win->alpha > 1.0f)
		win->alpha = 1.0f;
	return;
}

/* ----------------------------------------------------------- */

void og_set_movable_window(orvgui_win *win, unsigned char movable)
{
	win->movable = movable;

	return;
}

/* ----------------------------------------------------------- */

void og_set_popup_window(orvgui_win *win, unsigned char popup)
{
	win->popup = popup;

	return;
}

/* ----------------------------------------------------------- */

/*
void og_write_text(orvgui_win *win, short x, short y, char *text)
{
	if (def_pics != NULL)
	{
		g_write_text_fixed(win->get_pic, def_pics, text,
		x, y, OG_FONT_SIZEX, OG_FONT_SIZEY, OG_FONTS_PER_ROW);
	}

	//og_refresh_window(win);

	return;
}
*/

/* ----------------------------------------------------------- */

void og_write_text_transp(orvgui_win *win, IStorm3D_Font *font, 
	short x, short y, char *text, short fontwidth, short fontheight, float alpha, const COL &color )
{
	// TODO: should actually change the text alpha, not just make
	// it disappear/appear
	
	//if (win->alpha > 0.5f)
	{
		// added the * alpha - Pete
		float f = ( win->alpha * alpha );

		/*if(font && !font->isUnicode())
		{*/
			og_renderer->Render2D_Text(font, 
				VC2((float)((win->put_x + x) * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)((win->put_y + y) * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
				VC2((float)(fontwidth * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)(fontheight * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
				text,f,color);
		/*}
		else
		{
			std::wstring uniText;
			frozenbyte::util::convertToWide(text, uniText);

			og_renderer->Render2D_Text(font, 
				VC2((float)((win->put_x + x) * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)((win->put_y + y) * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
				VC2((float)(fontwidth * og_scale_x / (float)OG_SCALE_MULTIPLIER), 
				(float)(fontheight * og_scale_y / (float)OG_SCALE_MULTIPLIER)), 
				uniText.c_str(),f,color);
		}*/
	}


//	og_renderer->Render2D_Text(font, 
//		VC2((float)((win->put_x + x) * og_scale_x / OG_SCALE_MULTIPLIER), 
//		(float)((win->put_y + y) * og_scale_y / OG_SCALE_MULTIPLIER)), 
//		VC2((float)(OG_FONT_SIZEX * og_scale_x / OG_SCALE_MULTIPLIER), 
//		(float)(OG_FONT_SIZEY * og_scale_y / OG_SCALE_MULTIPLIER)), 
//		text);
//	og_renderer->Render2D_Text(font, VC2((float)win->put_x + x, (float)win->put_y + y), 
//		VC2((float)win->sizex - x, (float)win->sizey - y), text);

	// now this written text is just temporary... and should be.
	// need a label object (inactive button, with just text)

	/*
	if (def_pics != NULL)
	{
		g_write_text_fixed_transp(win->get_pic, def_pics, text,
			x, y, OG_FONT_SIZEX, OG_FONT_SIZEY, OG_FONTS_PER_ROW, def_bg_col);
	}
	*/

	//og_refresh_window(win);

	return;
}

/* ----------------------------------------------------------- */

/*
void og_box(orvgui_win *win, short x, short y, short x2, short y2, color col)
{
	g_box(win->get_pic, x, y, x2, y2, col);

	og_refresh_window(win);

	return;
}
*/

/* ----------------------------------------------------------- */

/*
void og_fbox(orvgui_win *win, short x, short y, short x2, short y2, color col)
{
	g_fbox(win->get_pic, x, y, x2, y2, col);

	og_refresh_window(win);

	return;
}
*/

/* ----------------------------------------------------------- */

/*
void og_line(orvgui_win *win, short x, short y, short x2, short y2, color col)
{
	g_line(win->get_pic, x, y, x2, y2, col);

	og_refresh_window(win);

	return;
}
*/

/* ----------------------------------------------------------- */

/*
color og_get_fg_color(void)
{
	return def_fg_col;
}
*/

/* ----------------------------------------------------------- */

/*
color og_get_cursor_color(void)
{
	return def_cursor_col;
}
*/

/* ----------------------------------------------------------- */

/*
color og_get_selection_color(void)
{
	return def_selection_col;
}
*/

/* ----------------------------------------------------------- */

/*
color og_get_bg_color(void)
{
	return def_bg_col;
}
*/

/* ----------------------------------------------------------- */

/*
static void og_error_ignore(void)
{
	og_hide_window(error_win);

	//og_draw_window(error_win);
	error_line = 0;
}
*/

/* ----------------------------------------------------------- */

/*
static void og_error_terminate(void)
{
	og_terminate = 1;
}
*/

/* ----------------------------------------------------------- */

// TODO
void og_show_error(const char *msg)
{
	//FILE *f;

	// warning removal
	//msg = msg;

Logger::getInstance()->error(msg);

	if (error_win == NULL || got_orvgui == 0)
	{
		//printf(msg);
		//printf("\n");
	} else {
		// TODO
		/*
		g_write_text_fixed(error_win->get_pic, def_pics, msg,
			1, 21+error_line*8, 8, 8, 64);
		*/

		error_line++;
		if (error_line > 15) error_line = 0;
		//g_fbox(error_win->get_pic, 1, 21+error_line*8,
		//	error_win->sizex-2, 28+error_line*8, def_bg_col);

		og_show_window(error_win);

		og_raise_window(error_win);

		//og_refresh_screen();
	}

	/*
	if (og_errors == 0)
	{
		if (og_errorfile != NULL && (f = fopen(og_errorfile, "wt")) != NULL)
		{
			fprintf(f, "Ogui error listing (You may delete this file)\n\n");
			fprintf(f, msg);
			fprintf(f, "\n");
			fclose(f);
		}
	} else {
		if (og_errorfile != NULL && (f = fopen(og_errorfile, "at")) != NULL)
		{
			fprintf(f, msg);
			fprintf(f, "\n");
			fclose(f);
		}
	}
	*/

	og_errors++;
//	if (og_errors > 999)
//	{
//#ifdef _DEBUG
//		abort();
//#else
//		exit(1);
//#endif
//	}

	return;
}

/* ----------------------------------------------------------- */

void og_empty_proc(void)
{
	return;
}

/* ----------------------------------------------------------- */

void og_proc_hide_parent(void)
{
	if (cursor_on[og_cursor_num] != NULL)
	{
		if (only_active == (cursor_on[og_cursor_num])->parent)
			only_active = NULL;
			og_hide_window((cursor_on[og_cursor_num])->parent);
	}
	return;
}

/* ----------------------------------------------------------- */

void og_proc_delete_parent(void)
{
	if (cursor_on[og_cursor_num] != NULL)
	{
		if (only_active == (cursor_on[og_cursor_num])->parent)
			only_active = NULL;
			og_delete_window((cursor_on[og_cursor_num])->parent);
	}
	return;
}

/* ----------------------------------------------------------- */

void og_proc_delete_self(void)
{
	if (cursor_on[og_cursor_num] != NULL)
	{
		og_delete_button(cursor_on[og_cursor_num]);
	}
	return;
}

/* ----------------------------------------------------------- */

void init_orvgui(void)
{
	int g;

	Storm3D_SurfaceInfo screenInfo = og_storm3d->GetScreenSize();
	scr_size_x = screenInfo.width;
	scr_size_y = screenInfo.height;

	//vscr = g_alloc_picture(PIC_TYPE_8BIT,PAL_TYPE_NONE,scr_size_x,scr_size_y);
	first_win = NULL;
	last_win = NULL;
	//vscr_bg_pic = NULL;
	//vscr_bg_col = g_col_black;

	//def_pics = g_load_pcx(og_packname, "defpics.pcx");
	//self_loaded_def_pics = def_pics;
	//if (def_pics != NULL)
	//{
	//	def_bg_col = g_pget(def_pics, 0, 0);
	//	transp_col = g_pget(def_pics, 1, 0);
	//	def_fg_col = g_pget(def_pics, 2, 0);
	//	def_cursor_col = g_pget(def_pics, 3, 0);
	//	def_selection_col = g_pget(def_pics, 4, 0);
	//	g_pal_to_scr(def_pics);
	//}

	error_win = og_create_window(OG_WIN_SIMPLE,7,7,(short)(scr_size_x-14),154,NULL);
	if (error_win != NULL)
	{
		og_hide_window(error_win);
		og_set_movable_window(error_win, OG_WIN_MOVABLE);
		//og_box(error_win, 0, 0, scr_size_x-15, 153, def_fg_col);
		error_win->visible = 0;
	}

	//if (vscr == NULL || error_win == NULL || def_pics == NULL)
	if (error_win == NULL)
	{
		got_orvgui = 0;
		return;
	}

	/*
	error_but1 = og_create_button(error_win,OG_BUT_PIC,58,2,64,16);
	error_but2 = og_create_button(error_win,OG_BUT_PIC,122,2,88,16);
	error_but3 = og_create_button(error_win,OG_BUT_PIC,2,2,56,16);
	og_disable_button(error_but3);

	if (error_but1 == NULL || error_but2 == NULL || error_but3 == NULL)
	{
		got_orvgui = 0;
		return;
	}

	error_but1->get_pic = NULL;
	//error_but1->get_x = 0;
	//error_but1->get_y = 32;
	error_but2->get_pic = NULL;
	//error_but2->get_x = 64;
	//error_but2->get_y = 32;
	error_but3->get_pic = NULL;
	//error_but3->get_x = 152;
	//error_but3->get_y = 32;

	error_but1->proc_click = og_error_ignore;
	error_but2->proc_click = og_error_terminate;
	error_but3->react_but = 0;
	*/

//		g_write_text_fixed(error_win->get_pic, def_pics, "ERROR", 8, 4, 8, 8, 64);
	//og_refresh_button(error_but1);
	//og_refresh_button(error_but2);
	//og_refresh_button(error_but3);
	//og_refresh_window(error_win);

	og_errors = 0;
	og_terminate = 0;

	for (g=0; g<OG_CURSORS; g++)
	{
		cursor_pic[g] = NULL;
		//cursor_get_x[g] = DEFAULT_CURSOR_GET_X + g*CURSOR_SIZE_X;
		//cursor_get_y[g] = DEFAULT_CURSOR_GET_Y;
		cursor_state[g] = 0;
		cursor_control[g] = 0;
		cursor_offset_x[g] = 0;
		cursor_offset_y[g] = 0;
		cursor_x[g] = (scr_size_x>>1);
		cursor_y[g] = (scr_size_y>>1);
		cursor_but[g] = 0;
		cursor_ox[g] = (scr_size_x>>1);
		cursor_oy[g] = (scr_size_y>>1);
		cursor_obut[g] = 0;
		cursor_on[g] = NULL;
		cursor_last_highlight[g] = NULL;
		cursor_speed[g] = 0;
		cursor_drag[g] = NULL;
	}

	only_active = NULL;

	for (g=0; g<MAX_KEYREADERS; g++)
	{
		keyreader[g] = NULL;
		keyreader_id[g] = -1;
		keyreader_cursor[g] = 0;
	}
	for (g=0; g<MAX_KEYS; g++)
	{
		keyhandled[g] = 0;
	}

	got_orvgui = 1;

	return;
}

/* ----------------------------------------------------------- */

void uninit_orvgui(void)
{
	if (got_orvgui != 0)
	{
		og_delete_window(error_win);
		//g_delete_picture(self_loaded_def_pics);
		//g_delete_picture(vscr);
	}

	got_orvgui = 0;

	return;
}

/* ----------------------------------------------------------- */

void debugMaterialInit(void)
{
	static bool inited = false;
	if(!inited && og_visualize_windows)
	{
		debug_window_material = og_storm3d->CreateNewMaterial("gui_debug_window");
		debug_window_material->SetColor(COL(0,0,1.0f));
		debug_window_material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		debug_window_material->SetTransparency(0.0f);

		debug_window_hidden_material = og_storm3d->CreateNewMaterial("gui_debug_window_hidden");
		debug_window_hidden_material->SetColor(COL(1.0f,1.0f,1.0f));
		debug_window_hidden_material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		debug_window_hidden_material->SetTransparency(0.5f);


		debug_button_material = og_storm3d->CreateNewMaterial("gui_debug_button");
		debug_button_material->SetColor(COL(0,1.0f,0));
		debug_button_material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		debug_button_material->SetTransparency(0.0f);

		debug_button_disabled_material = og_storm3d->CreateNewMaterial("gui_debug_button_disabled");
		debug_button_disabled_material->SetColor(COL(1.0f,0,0));
		debug_button_disabled_material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		debug_button_disabled_material->SetTransparency(0.0f);

		inited = true;
	}
	else if(inited && !og_visualize_windows)
	{
		delete debug_window_material;
		debug_window_material = NULL;

		delete debug_window_hidden_material;
		debug_window_hidden_material = NULL;

		delete debug_button_material;
		debug_button_material = NULL;

		delete debug_button_disabled_material;
		debug_button_disabled_material = NULL;

		inited = false;
	}
}

int og_get_scr_size_x(void)
{
	return scr_size_x;
}

int og_get_scr_size_y(void)
{
	return scr_size_y;
}
