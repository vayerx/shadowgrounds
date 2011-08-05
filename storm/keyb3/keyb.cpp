#if 1

#include <SDL.h>
#include "../include/igios.h"
#include "keyb3.h"

#include "../../game/SimpleOptions.h"
#include "../../game/options/options_controllers.h"

// Nappien maksimim‰‰r‰t
#define MAX_MOUSEBUTTONS 8
#define MAX_JOYBUTTONS 32
#define MAX_JOYSTICKS 4

int JoyNum=0;	// Joystikkejen m‰‰r‰

static int keylookup[] = {
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	KEYCODE_BACKSPACE, /* backspace */
	KEYCODE_TAB, /* tab */
	0, /* unknown key */
	0, /* unknown key */
	0, /* clear */
	KEYCODE_ENTER, /* return */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* pause */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	KEYCODE_ESC, /* escape */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	KEYCODE_SPACE, /* space */
	0, /* ! */
	0, /* " */
	0, /* # */
	0, /* $ */
	0, /* unknown key */
	0, /* & */
	0, /* ' */
	0, /* ( */
	0, /* ) */
	0, /* * */
	0, /* + */
	0, /* , */
	0, /* - */
	0, /* . */
	0, /* / */
	KEYCODE_0, /* 0 */
	KEYCODE_1, /* 1 */
	KEYCODE_2, /* 2 */
	KEYCODE_3, /* 3 */
	KEYCODE_4, /* 4 */
	KEYCODE_5, /* 5 */
	KEYCODE_6, /* 6 */
	KEYCODE_7, /* 7 */
	KEYCODE_8, /* 8 */
	KEYCODE_9, /* 9 */
	0, /* : */
	0, /* ; */
	0, /* < */
	0, /* = */
	0, /* > */
	0, /* ? */
	0, /* @ */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* [ */
	0, /* \ */
	0, /* ] */
	0, /* ^ */
	0, /* _ */
	0, /* ` */
	KEYCODE_A, /* a */
	KEYCODE_B, /* b */
	KEYCODE_C, /* c */
	KEYCODE_D, /* d */
	KEYCODE_E, /* e */
	KEYCODE_F, /* f */
	KEYCODE_G, /* g */
	KEYCODE_H, /* h */
	KEYCODE_I, /* i */
	KEYCODE_J, /* j */
	KEYCODE_K, /* k */
	KEYCODE_L, /* l */
	KEYCODE_M, /* m */
	KEYCODE_N, /* n */
	KEYCODE_O, /* o */
	KEYCODE_P, /* p */
	KEYCODE_Q, /* q */
	KEYCODE_R, /* r */
	KEYCODE_S, /* s */
	KEYCODE_T, /* t */
	KEYCODE_U, /* u */
	KEYCODE_V, /* v */
	KEYCODE_W, /* w */
	KEYCODE_X, /* x */
	KEYCODE_Y, /* y */
	KEYCODE_Z, /* z */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	KEYCODE_DELETE, /* delete */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	0, /* world 0 */
	0, /* world 1 */
	0, /* world 2 */
	0, /* world 3 */
	0, /* world 4 */
	0, /* world 5 */
	0, /* world 6 */
	0, /* world 7 */
	0, /* world 8 */
	0, /* world 9 */
	0, /* world 10 */
	0, /* world 11 */
	0, /* world 12 */
	0, /* world 13 */
	0, /* world 14 */
	0, /* world 15 */
	0, /* world 16 */
	0, /* world 17 */
	0, /* world 18 */
	0, /* world 19 */
	0, /* world 20 */
	0, /* world 21 */
	0, /* world 22 */
	0, /* world 23 */
	0, /* world 24 */
	0, /* world 25 */
	0, /* world 26 */
	0, /* world 27 */
	0, /* world 28 */
	0, /* world 29 */
	0, /* world 30 */
	0, /* world 31 */
	0, /* world 32 */
	0, /* world 33 */
	0, /* world 34 */
	0, /* world 35 */
	0, /* world 36 */
	0, /* world 37 */
	0, /* world 38 */
	0, /* world 39 */
	0, /* world 40 */
	0, /* world 41 */
	0, /* world 42 */
	0, /* world 43 */
	0, /* world 44 */
	0, /* world 45 */
	0, /* world 46 */
	0, /* world 47 */
	0, /* world 48 */
	0, /* world 49 */
	0, /* world 50 */
	0, /* world 51 */
	0, /* world 52 */
	0, /* world 53 */
	0, /* world 54 */
	0, /* world 55 */
	0, /* world 56 */
	0, /* world 57 */
	0, /* world 58 */
	0, /* world 59 */
	0, /* world 60 */
	0, /* world 61 */
	0, /* world 62 */
	0, /* world 63 */
	0, /* world 64 */
	0, /* world 65 */
	0, /* world 66 */
	0, /* world 67 */
	0, /* world 68 */
	0, /* world 69 */
	0, /* world 70 */
	0, /* world 71 */
	0, /* world 72 */
	0, /* world 73 */
	0, /* world 74 */
	0, /* world 75 */
	0, /* world 76 */
	0, /* world 77 */
	0, /* world 78 */
	0, /* world 79 */
	0, /* world 80 */
	0, /* world 81 */
	0, /* world 82 */
	0, /* world 83 */
	0, /* world 84 */
	0, /* world 85 */
	0, /* world 86 */
	0, /* world 87 */
	0, /* world 88 */
	0, /* world 89 */
	0, /* world 90 */
	0, /* world 91 */
	0, /* world 92 */
	0, /* world 93 */
	0, /* world 94 */
	0, /* world 95 */
	KEYCODE_KEYPAD_0, /* [0] */
	KEYCODE_KEYPAD_1, /* [1] */
	KEYCODE_KEYPAD_2, /* [2] */
	KEYCODE_KEYPAD_3, /* [3] */
	KEYCODE_KEYPAD_4, /* [4] */
	KEYCODE_KEYPAD_5, /* [5] */
	KEYCODE_KEYPAD_6, /* [6] */
	KEYCODE_KEYPAD_7, /* [7] */
	KEYCODE_KEYPAD_8, /* [8] */
	KEYCODE_KEYPAD_9, /* [9] */
	KEYCODE_KEYPAD_DOT, /* [.] */
	KEYCODE_KEYPAD_DIVIDE, /* [/] */
	KEYCODE_KEYPAD_MULTIPLY, /* [*] */
	KEYCODE_KEYPAD_MINUS, /* [-] */
	KEYCODE_KEYPAD_PLUS, /* [+] */
	KEYCODE_ENTER, /* enter */
	0, /* equals */
	KEYCODE_UP_ARROW, /* up */
	KEYCODE_DOWN_ARROW, /* down */
	KEYCODE_RIGHT_ARROW, /* right */
	KEYCODE_LEFT_ARROW, /* left */
	KEYCODE_INSERT, /* insert */
	0, /* home */
	0, /* end */
	KEYCODE_PAGE_UP, /* page up */
	KEYCODE_PAGE_DOWN, /* page down */
	KEYCODE_F1, /* f1 */
	KEYCODE_F2, /* f2 */
	KEYCODE_F3, /* f3 */
	KEYCODE_F4, /* f4 */
	KEYCODE_F5, /* f5 */
	KEYCODE_F6, /* f6 */
	KEYCODE_F7, /* f7 */
	KEYCODE_F8, /* f8 */
	KEYCODE_F9, /* f9 */
	KEYCODE_F10, /* f10 */
	KEYCODE_F11, /* f11 */
	11, /* f12 */
	0, /* f13 */
	0, /* f14 */
	0, /* f15 */
	0, /* unknown key */
	0, /* unknown key */
	0, /* unknown key */
	KEYCODE_NUMLOCK, /* numlock */
	KEYCODE_CAPSLOCK, /* caps lock */
	KEYCODE_SCROLLLOCK, /* scroll lock */
	KEYCODE_SHIFT_RIGHT, /* right shift */
	KEYCODE_SHIFT_LEFT, /* left shift */
	KEYCODE_CONTROL_RIGHT, /* right ctrl */
	KEYCODE_CONTROL_LEFT, /* left ctrl */
	0, /* right alt */
	0, /* left alt */
	0, /* right meta */
	0, /* left meta */
	0, /* left super */
	0, /* right super */
	KEYCODE_ALT_GR, /* alt gr */
	0, /* compose */
	0, /* help */
	KEYCODE_PRINTSCREEN, /* print screen */
	0, /* sys req */
	0, /* break */
	0, /* menu */
	0, /* power */
	0, /* e*-uro */
	0, /* undo */
};

#define MAX_KBS BASIC_KEYCODE_AMOUNT + ADDITIONAL_KEYBOARD_KEYS_AMOUNT //500
// FIXME: this is fucking ugly and breaks all kinds of encapsulation rules...
#define JOYSTICK_THRESHOLD game::SimpleOptions::getInt(DH_OPT_I_JOYSTICK1_DEADZONE + i)

struct nappis1
{
	unsigned char keysdown[MAX_KBS];	// mitk‰ napit pohjassa
	unsigned char exkd[MAX_KBS];		// entiset napit, viime
										// UpdateInputDevices:n k‰yttˆ‰ 
										// ennen, t‰t‰ voit k‰ytt‰‰
										// painallusten/irrotusten seuraamiseen
	unsigned char getpress[MAX_KBS];	// mink‰ nappien painallukset on jo "k‰sitelty" (GetKeypress)
	unsigned char getrelease[MAX_KBS];	// mink‰ nappien releaset on jo "k‰sitelty" (GetRelease)
};

struct hiiri1
{
	int x,y;	//kursorin koordinaatit
    int oldx, oldy;
	int dx,dy;	//kursorin deltakoordinaatit (vauhdit)

	int rulla;	//rullan delta "koordinaatti"
	
	int max_x,max_y;	// Hiiren reunat

	unsigned char nappi[MAX_MOUSEBUTTONS];	//napit
};

struct joy1
{
	SDL_Joystick *sdljoy;
	int x,y;	//tikun koordinaatit

	// Lis‰-akselit
	int rot_x,rot_y;
	int throttle,rudder;		

	unsigned char nappi[MAX_JOYBUTTONS];	//napit 
};

// Globaalit devicet
nappis1 nappis;
hiiri1 hiiri;
joy1 *joys;
int joyCount;
ui::GameController *gc;


// Inits/frees Keyb3 Control System
int Keyb3_Init(int CAPS) {		// Returns: TRUE=ok, FALSE=error
	// FIXME: ignores CAPS

	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	joyCount = SDL_NumJoysticks();
	if (joyCount > 4) // handle max 4 joysticks
		joyCount = 4;
	joys = new joy1[joyCount];

	for (int i = 0; i < joyCount; i++ ){
		joys[i].sdljoy = SDL_JoystickOpen(i);
		joys[i].x = 0;
		joys[i].y = 0;
		joys[i].rot_x = 0;
		joys[i].rot_y = 0;
		joys[i].throttle = 0;
		joys[i].rudder = 0;
		memset(joys[i].nappi, 0, sizeof(unsigned char) * MAX_JOYBUTTONS);
		//memset(joys[i].pov, 0, sizeof(int) * MAX_JOYPOV);

		// Debug
		/*
		LOG_INFO(strPrintf("Joystick info for %s", SDL_JoystickName(i)).c_str());
		LOG_INFO(strPrintf("Number of axes: %i", SDL_JoystickNumAxes(joys[i].sdljoy)).c_str());
		LOG_INFO(strPrintf("Number of balls: %i", SDL_JoystickNumBalls(joys[i].sdljoy)).c_str());
		LOG_INFO(strPrintf("Number of hats: %i", SDL_JoystickNumHats(joys[i].sdljoy)).c_str());
		LOG_INFO(strPrintf("Number of buttons: %i", SDL_JoystickNumButtons(joys[i].sdljoy)).c_str());
		*/
	}
	SDL_JoystickEventState(SDL_ENABLE);

	JoyNum = joyCount;

	return true;
}

void Keyb3_Free() {
	if (joys != NULL) {
		for (int i = 0; i < joyCount; i++)
			if (SDL_JoystickOpened(i))
				SDL_JoystickClose(joys[i].sdljoy);

		joyCount = 0;
		delete[] joys;
		joys = NULL;
	}
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

// Activates/Deactivates Keyb3 Control System.
// Deactivate Keyb3 when your window gets minimized.
// Activate Keyb3 when your window gets active again.
void Keyb3_SetActive(bool on) {
	SDL_WM_GrabInput(on ? SDL_GRAB_ON : SDL_GRAB_OFF);
	// for debugging so we don't steal focus from gdb
	SDL_ShowCursor(on ? SDL_DISABLE : SDL_ENABLE);
}

#define JOY_UP      (KEYCODE_JOY_UP      - KEYCODE_JOY_UP)
#define JOY_DOWN    (KEYCODE_JOY_DOWN    - KEYCODE_JOY_UP)
#define JOY_LEFT    (KEYCODE_JOY_LEFT    - KEYCODE_JOY_UP)
#define JOY_RIGHT   (KEYCODE_JOY_RIGHT   - KEYCODE_JOY_UP)
#define JOY_BUTTON1 (KEYCODE_JOY_BUTTON1 - KEYCODE_JOY_UP)

void Keyb3_AddController(ui::GameController *_gc) {
	gc = _gc;
	SDL_EnableUNICODE((gc != NULL) ? 1 : 0);
}

// Updates all devices
// Use this function frequently (it's best to be placed in your programs main loop)
void Keyb3_UpdateDevices() {
	memset(nappis.getpress, 0, MAX_KBS * sizeof(unsigned char));
	memset(nappis.getrelease, 0, MAX_KBS * sizeof(unsigned char));
	memcpy(nappis.exkd, nappis.keysdown, MAX_KBS * sizeof(unsigned char));

	nappis.keysdown[KEYCODE_MOUSE_WHEEL_DOWN] = false;
	nappis.keysdown[KEYCODE_MOUSE_WHEEL_UP] = false;

	hiiri.oldx = hiiri.x; hiiri.oldy = hiiri.y;

	SDL_Event event;
	int joyNum;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_RETURN) {
				SDLMod m = SDL_GetModState();
				if ((m & KMOD_ALT) != 0) {
					SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
					continue;
				}
			} else if (event.key.keysym.sym == SDLK_g) {
				SDLMod m = SDL_GetModState();
				if ((m & KMOD_CTRL) != 0) {
					SDL_GrabMode curr = SDL_WM_GrabInput(SDL_GRAB_QUERY);
					Keyb3_SetActive(curr == SDL_GRAB_OFF);
					continue;
				}
			} else if (event.key.keysym.sym == SDLK_c) {
				SDLMod m = SDL_GetModState();
				if ((m & KMOD_CTRL) != 0) {
					SDL_Quit();
					gc->suicide();
				}
			}

			if (gc != NULL)
			{
				if (keylookup[event.key.keysym.sym] != 0)
					gc->addReadKey(0, keylookup[event.key.keysym.sym]);
				else
					gc->addReadKey(event.key.keysym.unicode, 0);
			}
			nappis.keysdown[keylookup[event.key.keysym.sym]] = 1;
			break;
		case SDL_KEYUP:
			nappis.keysdown[keylookup[event.key.keysym.sym]] = 0;
			break;
		case SDL_JOYAXISMOTION:
			joyNum = event.jaxis.which;
			if (joyNum >= 0 && joyNum < joyCount) {
				int joyValue = (int) ((float(event.jaxis.value)) / 32768.0 * 1000.0);
				if (event.jaxis.axis == 0) {
					joys[joyNum].x = joyValue;
				} else if (event.jaxis.axis == 1) {
					joys[joyNum].y = joyValue;
				} else if (event.jaxis.axis == 2) {
					joys[joyNum].rot_x = joyValue;
				} else if (event.jaxis.axis == 3) {
					joys[joyNum].rot_y = joyValue;
				} else if (event.jaxis.axis == 4) {
					joys[joyNum].throttle = joyValue;
				} else if (event.jaxis.axis == 5) {
					joys[joyNum].rudder = joyValue;
				}
			}
			break;
		case SDL_JOYHATMOTION:
			joyNum = event.jhat.which;
			if (joyNum >= 0 && joyNum < joyCount) {
				/*
				for(int ii=0; ii < MAX_JOYPOV; ii++)
					joys[joyNum].pov[ii] = 0;
				if ((event.jhat.value & SDL_HAT_UP) != 0) {
					joys[joyNum].pov[0] = 1;
				} else if ((event.jhat.value & SDL_HAT_DOWN) != 0) {
					joys[joyNum].pov[1] = 1;
				} else if ((event.jhat.value & SDL_HAT_LEFT) != 0) {
					joys[joyNum].pov[2] = 1;
				} else if ((event.jhat.value & SDL_HAT_RIGHT) != 0) {
					joys[joyNum].pov[3] = 1;
				}
				*/
			}
			break;
		case SDL_JOYBUTTONDOWN: // fallthrough
		case SDL_JOYBUTTONUP:
      {
        joyNum = event.jbutton.which;
        int buttonNum = event.jbutton.button;
        if (joyNum >= 0 && joyNum < joyCount && buttonNum >= 0 && buttonNum < MAX_JOYBUTTONS)
          joys[joyNum].nappi[buttonNum] = (event.jbutton.state == SDL_PRESSED) ? 1 : 0;
        break;
      }
		case SDL_MOUSEMOTION:
			{
				hiiri.x = event.motion.x; hiiri.y = event.motion.y;
				//hiiri.dx = event.motion.xrel; hiiri.dy = event.motion.yrel;

				break;
			}

		case SDL_MOUSEBUTTONDOWN: // fallthrough
		case SDL_MOUSEBUTTONUP:
			{
				static unsigned int call = 0;
				++call;
				unsigned int b;
				switch (event.button.button) {
				case SDL_BUTTON_MIDDLE:
					b = KEYCODE_MOUSE_BUTTON2;
					break;

				case SDL_BUTTON_RIGHT:
					b = KEYCODE_MOUSE_BUTTON3;
					break;

				case SDL_BUTTON_WHEELDOWN:
					b = KEYCODE_MOUSE_WHEEL_DOWN;
					event.button.state = SDL_PRESSED;
					break;

				case SDL_BUTTON_WHEELUP:
					b = KEYCODE_MOUSE_WHEEL_UP;
					event.button.state = SDL_PRESSED;
					break;

				case SDL_BUTTON_LEFT: // fallthrough
				default:
					b = KEYCODE_MOUSE_BUTTON1;
					break;
				}
				nappis.keysdown[b] = (event.button.state == SDL_PRESSED);
				break;
			}
    case SDL_QUIT:
      exit(0);
		}
	}

	// comment this out to break shit
	hiiri.dx = hiiri.x - hiiri.oldx; hiiri.dy = hiiri.y - hiiri.oldy;

	nappis.keysdown[KEYCODE_MOUSE_UP]         = (hiiri.dy < 0);
	nappis.keysdown[KEYCODE_MOUSE_DOWN]       = (hiiri.dy > 0);
	nappis.keysdown[KEYCODE_MOUSE_LEFT]       = (hiiri.dx < 0);
	nappis.keysdown[KEYCODE_MOUSE_RIGHT]      = (hiiri.dx > 0);

	for (int i = 0; i < joyCount; i++) {
		int baseKeyCode = KEYCODE_JOY_UP;

		switch (i) {
			case 0:
				baseKeyCode = KEYCODE_JOY_UP;
				break;
			case 1:
				baseKeyCode = KEYCODE_JOY2_UP;
				break;
			case 2:
				baseKeyCode = KEYCODE_JOY3_UP;
				break;
			case 3:
				baseKeyCode = KEYCODE_JOY4_UP;
				break;
		}

		nappis.keysdown[baseKeyCode + JOY_UP] = (joys[i].y < -JOYSTICK_THRESHOLD);
		nappis.keysdown[baseKeyCode + JOY_DOWN] = (joys[i].y > JOYSTICK_THRESHOLD);
		nappis.keysdown[baseKeyCode + JOY_LEFT] = (joys[i].x < -JOYSTICK_THRESHOLD);
		nappis.keysdown[baseKeyCode + JOY_RIGHT] = (joys[i].x > JOYSTICK_THRESHOLD);

        /*
		LOG_DEBUG(strPrintf("joy %d x y %d %d buttons %d %d %d %d"
							, i, joys[i].x, joys[i].y
							, nappis.keysdown[baseKeyCode + JOY_UP]
							, nappis.keysdown[baseKeyCode + JOY_DOWN]
							, nappis.keysdown[baseKeyCode + JOY_LEFT]
							, nappis.keysdown[baseKeyCode + JOY_RIGHT]
						   ).c_str()
				 );
        */
		for (int j = 0; j < 16; j++)
			nappis.keysdown[baseKeyCode + JOY_BUTTON1 + j] = joys[i].nappi[j];
		nappis.keysdown[KEYCODE_JOY_ROT_UP] = joys[i].rot_y < -JOYSTICK_THRESHOLD;
		nappis.keysdown[KEYCODE_JOY_ROT_DOWN] = joys[i].rot_y > JOYSTICK_THRESHOLD;
		nappis.keysdown[KEYCODE_JOY_ROT_LEFT] = joys[i].rot_x < -JOYSTICK_THRESHOLD;
		nappis.keysdown[KEYCODE_JOY_ROT_RIGHT] = joys[i].rot_x > JOYSTICK_THRESHOLD;

		// POV
		/*
		nappis.keysdown[KEYCODE_JOY_POV_UP] = joys[i].pov[0];
		nappis.keysdown[KEYCODE_JOY_POV_DOWN] = joys[i].pov[1];
		nappis.keysdown[KEYCODE_JOY_POV_LEFT] = joys[i].pov[2];
		nappis.keysdown[KEYCODE_JOY_POV_RIGHT] = joys[i].pov[3];
		*/
	}

}

// Updates all devices. Updating devices uses a lot of processor time, so this
// function updates devices only if "int time" amount of time is elapsed.
void Keyb3_UpdateDevices_Optimized(int time) {	// time in 1/1000 secs (1000=1 second)
	static int last = 0;
	int now = SDL_GetTicks();

	if ((now - last) > time) {
		Keyb3_UpdateDevices();
		last = now;
	}
}

// Waits for a keypress, and returns keycode 
int Keyb3_WaitKeypress(bool returnIndividualMouse) {
	int OK = -1;
	Keyb3_UpdateDevices();

	do {
		Keyb3_UpdateDevices();

		for (int i = 0; i < MAX_KBS; i++)
			if (nappis.keysdown[i] > nappis.exkd[i]) OK = i;
	} while (OK < 0);

	return OK;
}

// Checks if key is down (returns TRUE if down, FALSE is not)
int Keyb3_IsKeyDown(unsigned int keynum) {
	assert(keynum < MAX_KBS);
	if (nappis.keysdown[keynum]) return 1;

	return 0;
}

// Gets keypress(/release), and clears the keypress(/release) mark. (returns TRUE if pressed, FALSE is not)
int Keyb3_GetKeyPress(int keynum) {
	int ok = 0;

	if (nappis.exkd[keynum]) return 0;
	if (nappis.keysdown[keynum]) ok = 1;
	if (nappis.getpress[keynum]) ok = 0;
	nappis.getpress[keynum] = 1;

	return ok;
}

int Keyb3_GetKeyRelease(int keynum) {
	int ok = 0;

	if (nappis.keysdown[keynum]) return 0;
	if (nappis.exkd[keynum]) ok = 1;
	if (nappis.getrelease[keynum]) ok = 0;
	nappis.getrelease[keynum] = 1;

	return ok;
}

// Checks is key state is changed from last update (returns TRUE if changed, FALSE is not)
int Keyb3_IsKeyPressed(int keynum) {
	if (nappis.exkd[keynum]) return 0;
	if (nappis.keysdown[keynum]) return 1;

	return 0;
}

int Keyb3_IsKeyReleased(int keynum) {
	if (nappis.keysdown[keynum]) return 0;
	if (nappis.exkd[keynum]) return 1;

	return 0;
}

// Mouse setup
void Keyb3_SetMouseBorders(int max_x, int max_y, int mouseID) {
	hiiri.max_x = max_x; hiiri.max_y = max_y;
}

void Keyb3_SetMousePos(int x, int y, int mouseID) {
	SDL_GrabMode curr = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	if (curr == SDL_GRAB_ON) {
		hiiri.x = x; hiiri.y = y;
		hiiri.oldx = x; hiiri.oldy = y;
		hiiri.dx = 0; hiiri.dy = 0;
		SDL_WarpMouse(x, y);
	}
}

void Keyb3_ReleaseMouseBorders() {
}

// Direct mouse read (give a NULL-pointer if you don't want some information) 
void Keyb3_ReadMouse(int *x, int *y, int *dx, int *dy, int mouseID) {
	if (x != NULL) *x = hiiri.x;
	if (y != NULL) *y = hiiri.y;
	if (dx != NULL) *dx = hiiri.dx;
	if (dy != NULL) *dy = hiiri.dy;
}

// Direct joystick read. (Axis: [-1000,1000], 0=center)
// (joynum: 0=first joystick, 1=second joystick)
// (give a NULL-pointer if you don't want some information) 
void Keyb3_ReadJoystick(int joynum, int *x, int *y, int *rx, int *ry, int *throttle, int *rudder) {
	if (joynum >= 0 && joynum < joyCount) {
		if (x != NULL) (*x) = joys[joynum].x;
		if (y != NULL) (*y) = joys[joynum].y;
		if (rx != NULL) (*rx) = joys[joynum].rot_x;
		if (ry != NULL) (*ry) = joys[joynum].rot_y;
		if (throttle != NULL) (*throttle) = joys[joynum].throttle;
		if (rudder != NULL) (*rudder) = joys[joynum].rudder;
	}
}

int Keyb3_GetNumberOfMouseDevices() {
	return 1;
    // NOTICE: counts the "every mouse" -handle as a device as well,
    // which is the last mouseID. see MouseHandler.h for more info.
}


int Keyb3_DefaultMouseID() {
	// Mouse ID that is controlled by every mouse for RawInput. 0 for DirectInput.
	return 0;
}


bool RawInputDeviceHandler::isInitialized() {
	return false;
}

std::string RawInputDeviceHandler::getError() {
	igios_unimplemented();
	return "";
}

int RawInputDeviceHandler::getNumOfMouses() {
	igios_unimplemented();
	return 1;
}

int RawInputDeviceHandler::getNumOfKeyboards() {
	igios_unimplemented();
	return 1;
}

#else

// KEYB for Windows v3.6 (C) Sebastian Aaltonen 2000
// -------------------------------------------------

// Hoitelee kaikki ohjainlaitteet k‰ytt‰m‰ll‰ 
// DX8:n DirectInput-rajapintaa. 




// Windowsin perus
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#include <assert.h>

// DX8:n
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "keyb3.h"
#include <atlbase.h>

// RawInput -mousehandleri.
#include "RawInputMouseHandler.h"

//-----------------------------------------------------------------------------
// Definet, globaalit
//-----------------------------------------------------------------------------

#define CONTROLLER_TYPE_KEYBOARD	1
#define CONTROLLER_TYPE_JOYSTICK	2
#define CONTROLLER_TYPE_MOUSE		3

// magical number for joystick treshold :) -jpk
#define JOYSTICK_TRESHOLD 5


HWND MGHwnd;	//P‰‰ohjelman HWND

LPDIRECTINPUT8			DI=NULL;				// DInput objekti
LPDIRECTINPUTDEVICE8	Device_Keyboard=NULL;	// N‰ppis-device
LPDIRECTINPUTDEVICE8	Device_Mouse=NULL;		// Hiiri-device
LPDIRECTINPUTDEVICE8	Device_Joystick1=NULL;	// Joystick device
GUID					GUID_Joystick1;		// Joystick GUID
LPDIRECTINPUTDEVICE8	Device_Joystick2=NULL;	// Joystick2 device
GUID					GUID_Joystick2;		// Joystick2 GUID
LPDIRECTINPUTDEVICE8	Device_Joystick3=NULL;	// Joystick device
GUID					GUID_Joystick3;		// Joystick GUID
LPDIRECTINPUTDEVICE8	Device_Joystick4=NULL;	// Joystick2 device
GUID					GUID_Joystick4;		// Joystick2 GUID

// Mitk‰ laitteet on k‰ytˆss‰
BOOL UseKeyboard = FALSE;
BOOL UseMouse    = FALSE;
BOOL UseJoystick1 = FALSE;
BOOL UseJoystick2 = FALSE;
BOOL UseJoystick3 = FALSE;
BOOL UseJoystick4 = FALSE;
int JoyNum=0;	// Joystikkejen m‰‰r‰

// "Nappejen" m‰‰r‰
#define MAX_KBS BASIC_KEYCODE_AMOUNT + ADDITIONAL_KEYBOARD_KEYS_AMOUNT

// Nappejen maksimim‰‰r‰t
#define MAX_MOUSEBUTTONS 8
#define MAX_JOYBUTTONS 32
#define MAX_JOYSTICKS 4

// Onko RawInput k‰ytˆss‰? Jos ei, niin DirectInput (jolloin ei ole tukea useammalle hiirelle).
BOOL UseRawInput = TRUE;
bool multkeyb_old[MAX_KEYBOARDS][MAX_KEYS];
bool multkeyb[MAX_KEYBOARDS][MAX_KEYS];


struct nappis1
{
	unsigned char keysdown[MAX_KBS];	// mitk‰ napit pohjassa
	unsigned char exkd[MAX_KBS];		// entiset napit, viime 
										// UpdateInputDevices:n k‰yttˆ‰ 
										// ennen, t‰t‰ voit k‰ytt‰‰
										// painallusten/irrotusten seuraamiseen
	unsigned char getpress[MAX_KBS];	// mink‰ nappejen painallukset on jo "k‰sitelty" (GetKeypress)
	unsigned char getrelease[MAX_KBS];	// mink‰ nappejen releaset on jo "k‰sitelty" (GetRelease)
};

struct hiiri1
{
	int x,y;	//kursorin koordinaatit
	int dx,dy;	//kursorin deltakoordinaatit (vauhdit)

	int rulla;	//rullan delta "koordinaatti"
	
	int max_x,max_y;	// Hiiren reunat

	unsigned char nappi[MAX_MOUSEBUTTONS];	//napit
};

struct joy1
{
	int x,y;	//tikun koordinaatit

	// Lis‰-akselit
	int rot_x,rot_y;
	int throttle,rudder;		

	unsigned char nappi[MAX_JOYBUTTONS];	//napit 
};

// Globaalit devicet
nappis1 nappis;
hiiri1 hiiri[MAX_MICE];
joy1 joy[MAX_JOYSTICKS];	// 2 joystikki‰ MAX


int mouse_x = 0, mouse_y = 0, old_mouse_x = 0, old_mouse_y = 0;
int	window_center_x = 0, window_center_y = 0;
POINT current_pos = { 0 };
RECT window_rect = { 0 };
bool window_active = true;

// there seems to be some hacks here that clip the cursor to window area, even when not requested.
// thus, need this hack to counter the previous hacks. --jpk
bool supposed_to_clip_cursor = false;

//-----------------------------------------------------------------------------
// DisplayError()
// N‰ytt‰‰ DInputin:n virheilmoitukset
//-----------------------------------------------------------------------------
void DisplayError( CHAR* strMessage )
{
    MessageBox( NULL, strMessage, "Keyb3 ERROR", MB_OK );
}



//-----------------------------------------------------------------------------
// EnumJoysticksCallback()
// Jokaiselle joystikille k‰ytet‰‰n t‰t‰. Jos joystikki on olemassa, niin 
// saadaan device, jota k‰ytet‰‰n sitten myˆhemmin
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( LPCDIDEVICEINSTANCE pInst, 
                                     VOID* pvContext )
{
	if (JoyNum==0) memcpy( pvContext, &pInst->guidInstance, sizeof(GUID) );
	if (JoyNum==1) memcpy( &GUID_Joystick2, &pInst->guidInstance, sizeof(GUID) );
	if (JoyNum==2) memcpy( &GUID_Joystick3, &pInst->guidInstance, sizeof(GUID) );
	if (JoyNum==3) memcpy( &GUID_Joystick4, &pInst->guidInstance, sizeof(GUID) );
	
	JoyNum++;


    //return DIENUM_STOP;
	return DIENUM_CONTINUE;		// Etsit‰‰n kaikki joystikit
}


//-----------------------------------------------------------------------------
// CreateDInput()
// Tekee DirectInput objektin ja tallentaan joystikin GUID:in, jos on.
//-----------------------------------------------------------------------------
HRESULT CreateDInput(HWND hWnd)
{
    // Create the main DirectInput object
	if (FAILED(DirectInput8Create((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
		DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&DI,NULL)))
	{ 
        DisplayError( "Failed to create DirectInput" );
        return E_FAIL;
	}

	// Etsit‰‰n kiinnetetyt joystikit...
    ZeroMemory( &GUID_Joystick1, sizeof(GUID) );
    ZeroMemory( &GUID_Joystick2, sizeof(GUID) );
    ZeroMemory( &GUID_Joystick3, sizeof(GUID) );
    ZeroMemory( &GUID_Joystick4, sizeof(GUID) );
    
	JoyNum=0;	// Joystikkejen m‰‰r‰
	DI->EnumDevices(DI8DEVCLASS_GAMECTRL,EnumJoysticksCallback,
		&GUID_Joystick1,DIEDFL_ATTACHEDONLY);

    return S_OK;
}



//-----------------------------------------------------------------------------
// CreateKeyboard()
// Laittaa n‰ppiksen kuntoon
//-----------------------------------------------------------------------------
HRESULT CreateKeyboard( HWND hWnd )
{
    // Obtain an interface to the input device
    if(FAILED(DI->CreateDevice(GUID_SysKeyboard,/*IID_IDirectInputDevice2,
		                               (VOID**)*/&Device_Keyboard,NULL)))
    {
        return E_FAIL;
    }

    // Set the device data format.
    if( FAILED( Device_Keyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
    {
        return E_FAIL;
    }

    // Set the cooperativity level to let DirectInput know how this device
    // should interact with the system and with other DirectInput applications.
    //if( FAILED( Device_Keyboard->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE|DISCL_FOREGROUND ) ) )
    if( FAILED( Device_Keyboard->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY ) ) )
    {
        return E_FAIL;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// CreateMouse()
// Laittaa hiiren kuntoon
//-----------------------------------------------------------------------------
HRESULT CreateMouseDInput( HWND hWnd )
{

	// Obtain an interface to the input device
	if(FAILED(DI->CreateDevice(GUID_SysMouse,/*IID_IDirectInputDevice2,
														  (VOID**)*/&Device_Mouse,NULL)))
	{
		return E_FAIL;
	}

	// Set the device data format.
	if( FAILED( Device_Mouse->SetDataFormat( &c_dfDIMouse2 ) ) )
	{
		return E_FAIL;
	}

	// Set the cooperativity level to let DirectInput know how this device
	// should interact with the system and with other DirectInput applications.
	//if( FAILED( Device_Mouse->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE|DISCL_FOREGROUND ) ) )
	//if( FAILED( Device_Mouse->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND ) ) )
	if( FAILED( Device_Mouse->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND ) ) )
	{
		return E_FAIL;
	}

	int width = GetSystemMetrics (SM_CXSCREEN);
	int height = GetSystemMetrics (SM_CYSCREEN);

	GetWindowRect(hWnd, &window_rect);
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

	//SetCapture(hWnd);

	if (supposed_to_clip_cursor)
	{
		ClipCursor(&window_rect);
	}

	while(ShowCursor (FALSE) >= 0);

	return S_OK;
}





//-----------------------------------------------------------------------------
// CreateJoystick()
// Laittaa joystikin kuntoon
//-----------------------------------------------------------------------------
HRESULT CreateJoystick(HWND hWnd,GUID *JoyGUID,LPDIRECTINPUTDEVICE8 *JoyDevice)
{
    // Obtain an interface to the input device
    if( FAILED( DI->CreateDevice(*JoyGUID,/*IID_IDirectInputDevice2,
 		                               (VOID**)*/JoyDevice,NULL)))
    {
        return E_FAIL;
    }

    // Set the device data format.
    if( FAILED( (*JoyDevice)->SetDataFormat( &c_dfDIJoystick ) ) )
    {
        return E_FAIL;
    }

    // Set the cooperativity level to let DirectInput know how this device
    // should interact with the system and with other DirectInput applications.
    if( FAILED( (*JoyDevice)->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE|DISCL_FOREGROUND ) ) )
    {
        return E_FAIL;
    }


    // Set the range of the joystick axes tp [-1000,+1000]
    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYOFFSET; 
    diprg.lMin              = -10; 
    diprg.lMax              = +10; 

	// Asetetaan kaikkien akselejen liikealue
    diprg.diph.dwObj = DIJOFS_X;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    diprg.diph.dwObj = DIJOFS_Y;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    diprg.diph.dwObj = DIJOFS_Z;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    diprg.diph.dwObj = DIJOFS_RX;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    diprg.diph.dwObj = DIJOFS_RY;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    diprg.diph.dwObj = DIJOFS_RZ;
    (*JoyDevice)->SetProperty( DIPROP_RANGE, &diprg.diph );

    // Set the dead zone for the joystick axes (because many joysticks
    // aren't perfectly calibrated to be zero when centered).
    DIPROPDWORD dipdw; 
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD); 
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	dipdw.diph.dwHow        = DIPH_DEVICE; 
	dipdw.dwData            = 1000; // Here, 1000 = 10%

	/*
	dipdw.diph.dwObj = DIJOFS_X;    // Set the x-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	dipdw.diph.dwObj = DIJOFS_Y;    // Set the y-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	dipdw.diph.dwObj = DIJOFS_Z;    // Set the z-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	dipdw.diph.dwObj = DIJOFS_RX;    // Set the x-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	dipdw.diph.dwObj = DIJOFS_RY;    // Set the y-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	dipdw.diph.dwObj = DIJOFS_RZ;    // Set the z-axis deadzone
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );
	*/

	dipdw.diph.dwObj = 0;
	(*JoyDevice)->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

    return S_OK;
}



//-----------------------------------------------------------------------------
// DestroyInputDevices()
// Tuhoaa kaikki ohjaindevicet, jotka on olemassa
//-----------------------------------------------------------------------------
void DestroyInputDevices()
{
	// vapautetaan n‰ppis, joystick ja hiiri
    
	if( Device_Keyboard )
    {
        Device_Keyboard->Unacquire();
        Device_Keyboard->Release();
        Device_Keyboard = NULL;
    }
        
	if( Device_Mouse )
    {
        Device_Mouse->Unacquire();
        Device_Mouse->Release();
        Device_Mouse = NULL;
    }

	if( Device_Joystick1 )
    {
        Device_Joystick1->Unacquire();
        Device_Joystick1->Release();
        Device_Joystick1 = NULL;
    }
	if( Device_Joystick2 )
    {
        Device_Joystick2->Unacquire();
        Device_Joystick2->Release();
        Device_Joystick2 = NULL;
    }

	if( Device_Joystick3 )
    {
        Device_Joystick3->Unacquire();
        Device_Joystick3->Release();
        Device_Joystick3 = NULL;
    }

	if( Device_Joystick4 )
    {
        Device_Joystick4->Unacquire();
        Device_Joystick4->Release();
        Device_Joystick4 = NULL;
    }

	ClipCursor(0);
	supposed_to_clip_cursor = false;
}



//-----------------------------------------------------------------------------
// DestroyDInput()
// Tuhoaa ja vapauttaa DirectInputin
//-----------------------------------------------------------------------------
void DestroyDInput()
{
    // Destroy the DInput object
    if( DI )
        DI->Release();
    DI = NULL;
}



//-----------------------------------------------------------------------------
// Keyb3_UpdateDevices_Optimized()
// P‰ivitt‰‰ joystikin, hiiren ja n‰ppiksen omiin struckteihin, jos tietty aika
// on kulunut viime p‰ivityksest‰
//-----------------------------------------------------------------------------
void Keyb3_UpdateDevices_Optimized(int time)
{
	static DWORD timelastupd=timeGetTime()-1000;
	DWORD timenow=timeGetTime();

	// P‰ivitet‰‰n vain jos haluttu aika kulunut viime p‰ivityksest‰
	if ((timenow-timelastupd)>(UINT)time)
	{
		Keyb3_UpdateDevices();
		timelastupd=timenow;
	}
}


	
//-----------------------------------------------------------------------------
// Keyb3_UpdateDevices()
// P‰ivitt‰‰ joystikin, hiiren ja n‰ppiksen omiin struckteihin.
//-----------------------------------------------------------------------------

#define KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(n)\
	if(RawInputDeviceHandler::numMice - 1 > n)\
	{\
		am_info = RawInputDeviceHandler::getMouseInfo(n);\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _UP]			= ( am_info->dY < 0 );\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _DOWN]		= ( am_info->dY > 0 );\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _LEFT]		= ( am_info ->dX < 0 );\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _RIGHT]		= ( am_info ->dX > 0 );\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _WHEEL_UP]=(am_info->dwheel<0);\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _WHEEL_DOWN]=(am_info->dwheel>0);\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON1]=am_info->leftButton;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON2]=am_info->rightButton;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON3]=am_info->middleButton;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON4]=am_info->button1;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON5]=am_info->button2;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON6]=am_info->button3;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON7]=am_info->button4;\
		nappis.keysdown[KEYCODE_MOUSE ## n ## _BUTTON8]=am_info->button5;\
		am_info->dwheel = 0;\
	}



void Keyb3_UpdateDevices()
{
	int i;

    HRESULT      hr;
    BYTE			diks[256];		// N‰ppis-strukti
    DIMOUSESTATE2	dims;			// Hiiri
    DIJOYSTATE		dijs;			// Joystikki

	for (i=0;i<MAX_KBS;i++)
	{
		// Tyhjennet‰‰n Getpress ja release
		nappis.getpress[i]=0;
		nappis.getrelease[i]=0;

		// Tallennetaan EX-napit
		nappis.exkd[i]=nappis.keysdown[i];
	}

	for(int i = 0; i < MAX_KEYBOARDS; i++)
		for(int j = 0 ; j < MAX_KEYS; j++)
			multkeyb_old[i][j] = multkeyb[i][j];

	// psdtest
	{
		bool window_now_active = (GetFocus() == MGHwnd) && (GetActiveWindow() == MGHwnd) && (GetForegroundWindow() == MGHwnd);
		//bool window_now_active = (GetForegroundWindow() == MGHwnd);
		if(window_now_active != window_active)
		{
			if(!window_now_active)
			{
				ClipCursor(0);
			} else {
				if (supposed_to_clip_cursor)
					ClipCursor(&window_rect);
			}
		}

		window_active = window_now_active;
	}

	// n‰ppis
	// ------

	// umm.. apparently, the returns below have been removed, and thus,
	// if keyboard is not properly acquired or something, things are going to suck ass big time. 
	// (so added this new bool to fix that) --jpk
	bool keyboardFailure = false;

	// DirectInput -implementaatio
	if(!RawInputDeviceHandler::keyboardInitialized && Device_Keyboard) 
	if(UseKeyboard)
    {

		// POLL (tarvitaan ainakin joystikeille)
        if( FAILED( Device_Keyboard->Poll() ) )
				{
					keyboardFailure = true;
            //return;
				}

		hr = Device_Keyboard->GetDeviceState( sizeof(diks), &diks );

		// Jos virhe, niin kokeillaan uusiksi...
		if (FAILED(hr))
		{
			hr=Device_Keyboard->Acquire();
			if(FAILED(hr)) 
			{
				assert(!"Keyb3 - Failed to re-acquire keyboard.");
				keyboardFailure = true;
				//return;
			}
			else
			{
				hr=Device_Keyboard->GetDeviceState(sizeof(diks),&diks);
				if (FAILED(hr))
				{
					keyboardFailure = true;
				}
			}
		}
		//if(FAILED(hr)) return;

		// p‰ivit‰ n‰ppis-strukti
		if (!keyboardFailure)
		{
			for (i=0;i<256;i++) nappis.keysdown[i]=((diks[i]&0x80)!=0);
		}
	}

	// Rawinput.
	if(RawInputDeviceHandler::keyboardInitialized) 
	{
		for(int i = 0; i < 256; i++)
		{
			nappis.keysdown[i] = 0;
		}
		for(unsigned int i = 0; i < RawInputDeviceHandler::numKeyboards; i++)
			for(int j = 0 ; j < MAX_KEYS; j++)
			{
				multkeyb[i][j] = RawInputDeviceHandler::getKeyboardInfo(i)->keyDown[j];
				if(multkeyb[i][j])
					nappis.keysdown[j] = 1;
			}
	}
 


	// hiiri
	// -----
    
	// RawInput -implementaatio
	if(UseRawInput && UseMouse && RawInputDeviceHandler::mouseInitialized)
	{
		for(unsigned int l = 0 ;l < RawInputDeviceHandler::numMice; l++)
		{
			hiiri[l].dx=RawInputDeviceHandler::getMouseInfo (l)->dX;
			hiiri[l].dy=RawInputDeviceHandler::getMouseInfo (l)->dY;
			hiiri[l].rulla=0;
		}

		for(unsigned int l = 0; l < RawInputDeviceHandler::numMice; l++)
		{
			RawInputDeviceHandler::MouseInfo *m_info = RawInputDeviceHandler::getMouseInfo (l);
			hiiri[l].nappi[0] = (int)m_info->leftButton;
			hiiri[l].nappi[1] = (int)m_info->rightButton;
			hiiri[l].nappi[2] = (int)m_info->middleButton;
			hiiri[l].nappi[3] = (int)m_info->button1;
			hiiri[l].nappi[4] = (int)m_info->button2;
			hiiri[l].nappi[5] = (int)m_info->button3;
			hiiri[l].nappi[6] = (int)m_info->button4;
			hiiri[l].nappi[7] = (int)m_info->button5;
		}

		if(!window_active)
		{
			for(unsigned int l = 0; l < RawInputDeviceHandler::numMice; l++) {
				RawInputDeviceHandler::getMouseInfo (l)->dX = 0;
				RawInputDeviceHandler::getMouseInfo (l)->dY = 0;
				RawInputDeviceHandler::getMouseInfo (l)->leftButton = false;
				RawInputDeviceHandler::getMouseInfo (l)->middleButton = false;
				RawInputDeviceHandler::getMouseInfo (l)->rightButton = false;
				RawInputDeviceHandler::getMouseInfo (l)->wheel = 0;
				RawInputDeviceHandler::getMouseInfo (l)->oldWheel = 0;
				RawInputDeviceHandler::getMouseInfo (l)->dwheel = 0;
//				ZeroMemory(RawInputDeviceHandler::getMouseInfo (l), sizeof(RawInputDeviceHandler::MouseInfo));
			}


			/*
			for(unsigned int l = 0; l < MouseHandler::numMice; l++)
				for(i=0; i < MAX_MOUSEBUTTONS;i++)	
					hiiri[l].nappi[i] = 0;*/
		}

		// hiiren rajat
		for(unsigned int l = 0; l < RawInputDeviceHandler::numMice; l++) {
			if (RawInputDeviceHandler::getMouseInfo (l)->X > hiiri[l].max_x) RawInputDeviceHandler::getMouseInfo (l)->X=hiiri[l].max_x;
			if (RawInputDeviceHandler::getMouseInfo (l)->Y > hiiri[l].max_y) RawInputDeviceHandler::getMouseInfo (l)->Y=hiiri[l].max_y;
			if (RawInputDeviceHandler::getMouseInfo (l)->X < 0) RawInputDeviceHandler::getMouseInfo (l)->X=0;
			if (RawInputDeviceHandler::getMouseInfo (l)->Y < 0) RawInputDeviceHandler::getMouseInfo (l)->Y=0; 

			if (hiiri[l].x>hiiri[l].max_x) hiiri[l].x=hiiri[l].max_x;
			if (hiiri[l].y>hiiri[l].max_y) hiiri[l].y=hiiri[l].max_y;
			if (hiiri[l].x<0) hiiri[l].x=0;
			if (hiiri[l].y<0) hiiri[l].y=0; 
		}

		// p‰ivit‰ "extra"-nappis
		RawInputDeviceHandler::MouseInfo *am_info = RawInputDeviceHandler::getMouseInfo(MOUSEHANDLER_ALL_MOUSES_ID);
		nappis.keysdown[KEYCODE_MOUSE_UP]			= ( am_info->dY < 0 );
		nappis.keysdown[KEYCODE_MOUSE_DOWN]			= ( am_info->dY > 0 );
		nappis.keysdown[KEYCODE_MOUSE_LEFT]			= ( am_info ->dX < 0 );
		nappis.keysdown[KEYCODE_MOUSE_RIGHT]		= ( am_info ->dX > 0 );
		nappis.keysdown[KEYCODE_MOUSE_WHEEL_UP]	= (am_info -> dwheel < 0);
		nappis.keysdown[KEYCODE_MOUSE_WHEEL_DOWN]	= (am_info -> dwheel > 0);
		nappis.keysdown[KEYCODE_MOUSE_BUTTON1]		= am_info->leftButton;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON2]		= am_info->rightButton;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON3]		= am_info->middleButton;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON4]		= am_info->button1;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON5]		= am_info->button2;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON6]		= am_info->button3;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON7]		= am_info->button4;
		nappis.keysdown[KEYCODE_MOUSE_BUTTON8]		= am_info->button5;

		// !HACK!
		am_info->dwheel = 0;
		//assert(am_info->dwheel == 0);

		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(0);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(1);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(2);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(3);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(4);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(5);
		KEYB3_UPDATEDEVICE_HANDLE_MOUSE_BUTTONS(6);

//		assert( !nappis.keysdown[KEYCODE_MOUSE2_BUTTON1] );
//		assert( RawInputDeviceHandler::getMouseInfo (1)->dY == 0);

		//if(window_active)
		//{
			RawInputDeviceHandler::MouseInfo *m_info = RawInputDeviceHandler::getMouseInfo();
			current_pos.x = m_info->X;
			current_pos.y = m_info->Y;
			mouse_x = m_info->X;
			mouse_y = m_info->Y;

			for(unsigned int l = 0; l < RawInputDeviceHandler::numMice; l++)
			{
				hiiri[l].x = RawInputDeviceHandler::getMouseInfo (l)->X;
				hiiri[l].y = RawInputDeviceHandler::getMouseInfo (l)->Y;
			}
		//}

	}

	// DirectInput -implementaatio
	if(Device_Mouse) 
	if(UseMouse)
    {

		// POLL (tarvitaan ainakin joystikeille)
        if( FAILED( Device_Mouse->Poll() ) )
				{
            //return;
				}

		hr = Device_Mouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims );

		// Jos virhe, niin kokeillaan uusiksi...
		if (FAILED(hr))
		{
			hr=Device_Mouse->Acquire();
			if(FAILED(hr)) 
			{
				assert(!"Keyb3 - Failed to re-acquire mouse.");
				return;
			}
			else
			{
				hr = Device_Mouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims );
			}
		}
		if(FAILED(hr)) return;

		// p‰ivit‰ hiiri-strukti
/*		
		// UUs versio, kiihtyy...
		{
			static float speed=1;
			hiiri.x+=dims.lX*(int)speed;
			hiiri.y+=dims.lY*(int)speed;
			speed+=0.4f*sqrtf(((float)dims.lX*(float)dims.lX)+((float)dims.lY*(float)dims.lY));
			speed*=0.4f;
			if (speed<1) speed=1;
			if ((abs(dims.lX)<2)&&(abs(dims.lY)<2)) speed=1.0f;
		}
*/
		hiiri[0].dx=dims.lX;
		hiiri[0].dy=dims.lY;
		hiiri[0].rulla=dims.lZ;
		for (i=0;i<MAX_MOUSEBUTTONS;i++) hiiri[0].nappi[i]=((dims.rgbButtons[i] & 0x80)!=0);

		if(!window_active)
		{
			hiiri[0].dx = 0;
			hiiri[0].dy = 0;
			hiiri[0].rulla = 0;

			for(i=0;i<MAX_MOUSEBUTTONS;i++) 
				hiiri[0].nappi[i] = 0;
		}

		// hiiren rajat
		if (hiiri[0].x>hiiri[0].max_x) hiiri[0].x=hiiri[0].max_x;
		if (hiiri[0].y>hiiri[0].max_y) hiiri[0].y=hiiri[0].max_y;
		if (hiiri[0].x<0) hiiri[0].x=0;
		if (hiiri[0].y<0) hiiri[0].y=0; 
		
		// p‰ivit‰ "extra"-nappis
		nappis.keysdown[KEYCODE_MOUSE_UP]=(dims.lY<0);
		nappis.keysdown[KEYCODE_MOUSE_DOWN]=(dims.lY>0);
		nappis.keysdown[KEYCODE_MOUSE_LEFT]=(dims.lX<0);
		nappis.keysdown[KEYCODE_MOUSE_RIGHT]=(dims.lX>0);
		nappis.keysdown[KEYCODE_MOUSE_WHEEL_UP]=(dims.lZ>0);
		nappis.keysdown[KEYCODE_MOUSE_WHEEL_DOWN]=(dims.lZ<0);
		nappis.keysdown[KEYCODE_MOUSE_BUTTON1]=hiiri[0].nappi[0];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON2]=hiiri[0].nappi[1];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON3]=hiiri[0].nappi[2];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON4]=hiiri[0].nappi[3];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON5]=hiiri[0].nappi[4];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON6]=hiiri[0].nappi[5];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON7]=hiiri[0].nappi[6];
		nappis.keysdown[KEYCODE_MOUSE_BUTTON8]=hiiri[0].nappi[7];
		if(window_active)
		{
			// update
			GetCursorPos(&current_pos);

			old_mouse_x = mouse_x;
			old_mouse_y = mouse_y;

			mouse_x = hiiri[0].x;
			mouse_y = hiiri[0].y;
		}
		else
		{
			old_mouse_x = mouse_x;
			old_mouse_y = mouse_y;
		}
	}


    // joystickit
	// ----------

	// K‰yd‰‰n molemmat joystikit l‰pi (max 2)
	for (int jn=0;jn<MAX_JOYSTICKS;jn++)	
	{
		int ok=1;

		if (jn==0)	// Eka joystikki
		{
			if (!Device_Joystick1) ok=0; 
			if (!UseJoystick1) ok=0;
		}
		else if (jn==1)	// Toka joystikki
		{
			if (!Device_Joystick2) ok=0; 
			if (!UseJoystick2) ok=0;
		}
		else if (jn==2)	// Toka joystikki
		{
			if (!Device_Joystick3) ok=0; 
			if (!UseJoystick3) ok=0;
		}
		else if (jn==3)	// Toka joystikki
		{
			if (!Device_Joystick4) ok=0; 
			if (!UseJoystick4) ok=0;
		}

		if (ok)
		{

			if (jn==0)	// Eka joystikki
			{
				// POLL (tarvitaan ainakin joystikeille)
				if( FAILED( Device_Joystick1->Poll() ) )
				{
					//return;
				}

				hr = Device_Joystick1->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
	
				// Jos virhe, niin kokeillaan uusiksi...
				if (FAILED(hr))
				{
					hr=Device_Joystick1->Acquire();
					if(FAILED(hr)) 
						continue;
					else
					{
						hr = Device_Joystick1->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
					}
				}
				if(FAILED(hr)) 
					continue;
			}
			else if (jn==1)	// Toka joystikki
			{
				// POLL (tarvitaan ainakin joystikeille)
				if( FAILED( Device_Joystick2->Poll()))
				{
					//return;
				}

				hr = Device_Joystick2->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
	
				// Jos virhe, niin kokeillaan uusiksi...
				if (FAILED(hr))
				{
					hr=Device_Joystick2->Acquire();
					if(FAILED(hr)) 
						continue;
					else
					{
						hr = Device_Joystick2->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
					}
				}
				if(FAILED(hr)) 
					continue;
			}
			else if (jn==2)
			{
				// POLL (tarvitaan ainakin joystikeille)
				if( FAILED( Device_Joystick3->Poll()))
				{
					//return;
				}

				hr = Device_Joystick3->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
	
				// Jos virhe, niin kokeillaan uusiksi...
				if (FAILED(hr))
				{
					hr=Device_Joystick3->Acquire();
					if(FAILED(hr)) 
						continue;
					else
					{
						hr = Device_Joystick3->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
					}
				}
				if(FAILED(hr)) 
					continue;
			}
			else if (jn==3)	// Toka joystikki
			{
				// POLL (tarvitaan ainakin joystikeille)
				if( FAILED( Device_Joystick4->Poll()))
				{
					//return;
				}

				hr = Device_Joystick4->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
	
				// Jos virhe, niin kokeillaan uusiksi...
				if (FAILED(hr))
				{
					hr=Device_Joystick4->Acquire();
					if(FAILED(hr)) 
						continue;
					else
					{
						hr = Device_Joystick4->GetDeviceState( sizeof(DIJOYSTATE), &dijs );
					}
				}
				if(FAILED(hr)) 
					continue;
			}

			// p‰ivit‰ joystikin-strukti
			joy[jn].x=dijs.lX;
			joy[jn].y=dijs.lY;
			for (i=0;i<MAX_JOYBUTTONS;i++) joy[jn].nappi[i]=((dijs.rgbButtons[i] & 0x80)!=0);		
			joy[jn].throttle=dijs.lZ;
			joy[jn].rudder=dijs.lRz;
			joy[jn].rot_x=dijs.lRx;
			joy[jn].rot_y=dijs.lRy;

			// p‰ivit‰ "extra"-nappis
			if (jn==0)
			{
				nappis.keysdown[KEYCODE_JOY_UP]=(joy[jn].y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_DOWN]=(joy[jn].y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_LEFT]=(joy[jn].x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_RIGHT]=(joy[jn].x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_ROT_UP]=(joy[jn].rot_y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_ROT_DOWN]=(joy[jn].rot_y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_ROT_LEFT]=(joy[jn].rot_x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_ROT_RIGHT]=(joy[jn].rot_x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_THROTTLE_UP]=(joy[jn].throttle<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_THROTTLE_DOWN]=(joy[jn].throttle>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_RUDDER_LEFT]=(joy[jn].rudder<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_RUDDER_RIGHT]=(joy[jn].rudder>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY_BUTTON1]=joy[jn].nappi[0];
				nappis.keysdown[KEYCODE_JOY_BUTTON2]=joy[jn].nappi[1];
				nappis.keysdown[KEYCODE_JOY_BUTTON3]=joy[jn].nappi[2];
				nappis.keysdown[KEYCODE_JOY_BUTTON4]=joy[jn].nappi[3];
				nappis.keysdown[KEYCODE_JOY_BUTTON5]=joy[jn].nappi[4];
				nappis.keysdown[KEYCODE_JOY_BUTTON6]=joy[jn].nappi[5];
				nappis.keysdown[KEYCODE_JOY_BUTTON7]=joy[jn].nappi[6];
				nappis.keysdown[KEYCODE_JOY_BUTTON8]=joy[jn].nappi[7];
				nappis.keysdown[KEYCODE_JOY_BUTTON9]=joy[jn].nappi[8];
				nappis.keysdown[KEYCODE_JOY_BUTTON10]=joy[jn].nappi[9];
				nappis.keysdown[KEYCODE_JOY_BUTTON11]=joy[jn].nappi[10];
				nappis.keysdown[KEYCODE_JOY_BUTTON12]=joy[jn].nappi[11];
				nappis.keysdown[KEYCODE_JOY_BUTTON13]=joy[jn].nappi[12];
				nappis.keysdown[KEYCODE_JOY_BUTTON14]=joy[jn].nappi[13];
				nappis.keysdown[KEYCODE_JOY_BUTTON15]=joy[jn].nappi[14];
				nappis.keysdown[KEYCODE_JOY_BUTTON16]=joy[jn].nappi[15];
			}
			else if (jn==1)
			{
				nappis.keysdown[KEYCODE_JOY2_UP]=(joy[jn].y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_DOWN]=(joy[jn].y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_LEFT]=(joy[jn].x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_RIGHT]=(joy[jn].x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_ROT_UP]=(joy[jn].rot_y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_ROT_DOWN]=(joy[jn].rot_y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_ROT_LEFT]=(joy[jn].rot_x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_ROT_RIGHT]=(joy[jn].rot_x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_THROTTLE_UP]=(joy[jn].throttle<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_THROTTLE_DOWN]=(joy[jn].throttle>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_RUDDER_LEFT]=(joy[jn].rudder<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_RUDDER_RIGHT]=(joy[jn].rudder>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY2_BUTTON1]=joy[jn].nappi[0];
				nappis.keysdown[KEYCODE_JOY2_BUTTON2]=joy[jn].nappi[1];
				nappis.keysdown[KEYCODE_JOY2_BUTTON3]=joy[jn].nappi[2];
				nappis.keysdown[KEYCODE_JOY2_BUTTON4]=joy[jn].nappi[3];
				nappis.keysdown[KEYCODE_JOY2_BUTTON5]=joy[jn].nappi[4];
				nappis.keysdown[KEYCODE_JOY2_BUTTON6]=joy[jn].nappi[5];
				nappis.keysdown[KEYCODE_JOY2_BUTTON7]=joy[jn].nappi[6];
				nappis.keysdown[KEYCODE_JOY2_BUTTON8]=joy[jn].nappi[7];
				nappis.keysdown[KEYCODE_JOY2_BUTTON9]=joy[jn].nappi[8];
				nappis.keysdown[KEYCODE_JOY2_BUTTON10]=joy[jn].nappi[9];
				nappis.keysdown[KEYCODE_JOY2_BUTTON11]=joy[jn].nappi[10];
				nappis.keysdown[KEYCODE_JOY2_BUTTON12]=joy[jn].nappi[11];
				nappis.keysdown[KEYCODE_JOY2_BUTTON13]=joy[jn].nappi[12];
				nappis.keysdown[KEYCODE_JOY2_BUTTON14]=joy[jn].nappi[13];
				nappis.keysdown[KEYCODE_JOY2_BUTTON15]=joy[jn].nappi[14];
				nappis.keysdown[KEYCODE_JOY2_BUTTON16]=joy[jn].nappi[15];
			}
			else if (jn==2)
			{
				nappis.keysdown[KEYCODE_JOY3_UP]=(joy[jn].y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_DOWN]=(joy[jn].y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_LEFT]=(joy[jn].x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_RIGHT]=(joy[jn].x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_ROT_UP]=(joy[jn].rot_y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_ROT_DOWN]=(joy[jn].rot_y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_ROT_LEFT]=(joy[jn].rot_x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_ROT_RIGHT]=(joy[jn].rot_x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_THROTTLE_UP]=(joy[jn].throttle<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_THROTTLE_DOWN]=(joy[jn].throttle>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_RUDDER_LEFT]=(joy[jn].rudder<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_RUDDER_RIGHT]=(joy[jn].rudder>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY3_BUTTON1]=joy[jn].nappi[0];
				nappis.keysdown[KEYCODE_JOY3_BUTTON2]=joy[jn].nappi[1];
				nappis.keysdown[KEYCODE_JOY3_BUTTON3]=joy[jn].nappi[2];
				nappis.keysdown[KEYCODE_JOY3_BUTTON4]=joy[jn].nappi[3];
				nappis.keysdown[KEYCODE_JOY3_BUTTON5]=joy[jn].nappi[4];
				nappis.keysdown[KEYCODE_JOY3_BUTTON6]=joy[jn].nappi[5];
				nappis.keysdown[KEYCODE_JOY3_BUTTON7]=joy[jn].nappi[6];
				nappis.keysdown[KEYCODE_JOY3_BUTTON8]=joy[jn].nappi[7];
				nappis.keysdown[KEYCODE_JOY3_BUTTON9]=joy[jn].nappi[8];
				nappis.keysdown[KEYCODE_JOY3_BUTTON10]=joy[jn].nappi[9];
				nappis.keysdown[KEYCODE_JOY3_BUTTON11]=joy[jn].nappi[10];
				nappis.keysdown[KEYCODE_JOY3_BUTTON12]=joy[jn].nappi[11];
				nappis.keysdown[KEYCODE_JOY3_BUTTON13]=joy[jn].nappi[12];
				nappis.keysdown[KEYCODE_JOY3_BUTTON14]=joy[jn].nappi[13];
				nappis.keysdown[KEYCODE_JOY3_BUTTON15]=joy[jn].nappi[14];
				nappis.keysdown[KEYCODE_JOY3_BUTTON16]=joy[jn].nappi[15];
			}
			else if (jn==3)
			{
				nappis.keysdown[KEYCODE_JOY4_UP]=(joy[jn].y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_DOWN]=(joy[jn].y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_LEFT]=(joy[jn].x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_RIGHT]=(joy[jn].x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_ROT_UP]=(joy[jn].rot_y<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_ROT_DOWN]=(joy[jn].rot_y>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_ROT_LEFT]=(joy[jn].rot_x<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_ROT_RIGHT]=(joy[jn].rot_x>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_THROTTLE_UP]=(joy[jn].throttle<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_THROTTLE_DOWN]=(joy[jn].throttle>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_RUDDER_LEFT]=(joy[jn].rudder<-JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_RUDDER_RIGHT]=(joy[jn].rudder>JOYSTICK_TRESHOLD);
				nappis.keysdown[KEYCODE_JOY4_BUTTON1]=joy[jn].nappi[0];
				nappis.keysdown[KEYCODE_JOY4_BUTTON2]=joy[jn].nappi[1];
				nappis.keysdown[KEYCODE_JOY4_BUTTON3]=joy[jn].nappi[2];
				nappis.keysdown[KEYCODE_JOY4_BUTTON4]=joy[jn].nappi[3];
				nappis.keysdown[KEYCODE_JOY4_BUTTON5]=joy[jn].nappi[4];
				nappis.keysdown[KEYCODE_JOY4_BUTTON6]=joy[jn].nappi[5];
				nappis.keysdown[KEYCODE_JOY4_BUTTON7]=joy[jn].nappi[6];
				nappis.keysdown[KEYCODE_JOY4_BUTTON8]=joy[jn].nappi[7];
				nappis.keysdown[KEYCODE_JOY4_BUTTON9]=joy[jn].nappi[8];
				nappis.keysdown[KEYCODE_JOY4_BUTTON10]=joy[jn].nappi[9];
				nappis.keysdown[KEYCODE_JOY4_BUTTON11]=joy[jn].nappi[10];
				nappis.keysdown[KEYCODE_JOY4_BUTTON12]=joy[jn].nappi[11];
				nappis.keysdown[KEYCODE_JOY4_BUTTON13]=joy[jn].nappi[12];
				nappis.keysdown[KEYCODE_JOY4_BUTTON14]=joy[jn].nappi[13];
				nappis.keysdown[KEYCODE_JOY4_BUTTON15]=joy[jn].nappi[14];
				nappis.keysdown[KEYCODE_JOY4_BUTTON16]=joy[jn].nappi[15];
			}
		}
	}
}



//-----------------------------------------------------------------------------
// Keyb3_SetSync()
// Laittaa devicet p‰‰lle ja pois
//-----------------------------------------------------------------------------
void Keyb3_SetActive(int on)
{
	if(on)
	{

		if(Device_Keyboard) Device_Keyboard->Acquire();	
		if(Device_Mouse) Device_Mouse->Acquire();	
		if(Device_Joystick1) Device_Joystick1->Acquire();	
		if(Device_Joystick2) Device_Joystick2->Acquire();	
		if(Device_Joystick3) Device_Joystick3->Acquire();	
		if(Device_Joystick4) Device_Joystick4->Acquire();	
	}
	else
	{
		if(Device_Keyboard) Device_Keyboard->Unacquire();	
		if(Device_Mouse) Device_Mouse->Unacquire();	
		if(Device_Joystick1) Device_Joystick1->Unacquire();	
		if(Device_Joystick2) Device_Joystick2->Unacquire();	
		if(Device_Joystick3) Device_Joystick3->Unacquire();	
		if(Device_Joystick4) Device_Joystick4->Unacquire();	
	}
}



//-----------------------------------------------------------------------------
// Keyb3_Init()
// Laittaa DirectInputin p‰‰lle, sek‰ alustaa kaikki ohjaindevicet
//-----------------------------------------------------------------------------
int Keyb3_Init(HWND hw,DWORD caps)
{
	// Otetaan talteen Hwindow
	MGHwnd=hw;

	// Asetukset
	UseKeyboard=FALSE;
	UseMouse=FALSE;
	UseJoystick1=FALSE;
	UseJoystick2=FALSE;
	UseJoystick3=FALSE;
	UseJoystick4=FALSE;
	if (caps&KEYB3_CAPS_KEYBOARD) UseKeyboard=TRUE;
	if (caps&KEYB3_CAPS_MOUSE) UseMouse=TRUE;
	if (caps&KEYB3_CAPS_JOYSTICK) UseJoystick1=TRUE;
	if (caps&KEYB3_CAPS_JOYSTICK2) UseJoystick2=TRUE;
	if (caps&KEYB3_CAPS_JOYSTICK3) UseJoystick3=TRUE;
	if (caps&KEYB3_CAPS_JOYSTICK4) UseJoystick4=TRUE;

	if(caps&KEYB3_CAPS_USE_RAWINPUT)
	{
		UseRawInput = TRUE;
	} else
	{
		UseRawInput = FALSE;
	}

	// DInput p‰‰lle
    if( FAILED( CreateDInput(MGHwnd) ) ) return FALSE;


	 if(UseRawInput == TRUE)
	 {
		bool mflag = true, kflag = true;
		if(!UseMouse) mflag = false;
		if(!UseKeyboard) kflag = false;

		if(!RawInputDeviceHandler::init( mflag, kflag ))
			UseRawInput = FALSE;
		else
		{
			for(int l = 0; l < (int)RawInputDeviceHandler::numMice; l++)
				ZeroMemory( &hiiri[l], sizeof(hiiri1));
			if(UseKeyboard)
			{
				for(unsigned int j = 0; j < MAX_KEYBOARDS; j++)
					for(int i = 0; i < MAX_KEYS; i++)
					{
						multkeyb[j][i] = false;
						multkeyb_old[j][i] = false;
					}
			}
		}
	 }

	 if(UseRawInput == FALSE) {
		 if(UseMouse)
			 if( FAILED ( CreateMouseDInput(MGHwnd) ) )
			 {
				 UseMouse = FALSE;
			 } else {
				 //hiiri = new hiiri1[1];
			 }
			 if (UseKeyboard)
				 if( FAILED( CreateKeyboard(MGHwnd) ) )
					 UseKeyboard=FALSE;
	 }


	 // tee n‰ppis

	// tee joystick1
    if (UseJoystick1) if (JoyNum>0) 
	if(FAILED(CreateJoystick(MGHwnd,&GUID_Joystick1,&Device_Joystick1))) UseJoystick1=FALSE;

	// tee joystick2
    if (UseJoystick2) if (JoyNum>1) 
	if(FAILED(CreateJoystick(MGHwnd,&GUID_Joystick2,&Device_Joystick2))) UseJoystick2=FALSE;

	// tee joystick3
    if (UseJoystick3) if (JoyNum>2) 
	if(FAILED(CreateJoystick(MGHwnd,&GUID_Joystick3,&Device_Joystick3))) UseJoystick3=FALSE;

	// tee joystick4
    if (UseJoystick4) if (JoyNum>3) 
	if(FAILED(CreateJoystick(MGHwnd,&GUID_Joystick4,&Device_Joystick4))) UseJoystick4=FALSE;

	 // p‰‰lle syncci
	Keyb3_SetActive(TRUE);

	return TRUE;
}



//-----------------------------------------------------------------------------
// Keyb3_Free()
// Poistaa DirectInputin p‰‰lt‰
//-----------------------------------------------------------------------------
void Keyb3_Free()
{
	// pois syncci
	Keyb3_SetActive(FALSE);

	// poistetaan ohjaindevicet
    DestroyInputDevices();

	// poistetaan DInput-objekti
    DestroyDInput();

	 // RawInput
	 RawInputDeviceHandler::free();

	 //delete [] hiiri;

}



//-----------------------------------------------------------------------------
// Keyb3_WaitKeyPress()
// Odottaa napinpainallusta, ja palauttaa napin koodin
//-----------------------------------------------------------------------------
int Keyb3_WaitKeypress( bool returnIndividualMouse )
{
	int OK=-1;

	Keyb3_UpdateDevices();
	Keyb3_UpdateDevices();

	do
	{
		Keyb3_UpdateDevices();
		if(!RawInputDeviceHandler::keyboardInitialized)
		{
			for (int i=0;i<MAX_KBS;i++)
				if (nappis.keysdown[i]>nappis.exkd[i]) OK=i;
		} else
		{
			RawInputDeviceHandler dh;
			for( int j = 0; j < dh.getNumOfKeyboards (); j++)
				for (int i = 0; i < MAX_KEYS; i++)
					if (RawInputDeviceHandler::getKeyboardInfo(j)->keyDown[i])
						OK=KEYCODE_GEN_KEYBID(j, i);
		}

	} while ( OK < 0 );

	// palautetaan painetun napin koodi
	return OK;
}



//-----------------------------------------------------------------------------
// Keyb3_IsKeyDown()
// Onko tietty nappi painettuna
//-----------------------------------------------------------------------------
int Keyb3_IsKeyDown(int keynum)
{
	if(!RawInputDeviceHandler::keyboardInitialized || (keynum < BASIC_KEYCODE_AMOUNT) )
	{
		if (nappis.keysdown[keynum]) return 1;
	} else
	{
		//if (RawInputDeviceHandler::getKeyboardInfo (KEYCODE_KEYBID(keynum))->keyDown[KEYCODE_KEYID(keynum)])
		if(multkeyb[KEYCODE_KEYBID(keynum)][KEYCODE_KEYID(keynum)])
			return 1;
	}
	return 0;
}



//-----------------------------------------------------------------------------
// Keyb3_IsKeyPressed()
// Onko tietty nappi painettu nyt alas
//-----------------------------------------------------------------------------
int Keyb3_IsKeyPressed(int keynum)
{
	if(!RawInputDeviceHandler::keyboardInitialized || (keynum < BASIC_KEYCODE_AMOUNT) )
	{
		if (nappis.exkd[keynum]) return 0;
		if (nappis.keysdown[keynum]) return 1;
	} else
	{
		if (multkeyb_old[KEYCODE_KEYBID(keynum)][KEYCODE_KEYID(keynum)])
			return 0;
		if (multkeyb[KEYCODE_KEYBID(keynum)][KEYCODE_KEYID(keynum)])
			return 1;
	}
	return 0;
}



//-----------------------------------------------------------------------------
// Keyb3_IsKeyReleased()
// Onko tietty nappi "nostettu" nyt ylˆs
//-----------------------------------------------------------------------------
int Keyb3_IsKeyReleased(int keynum)
{
	if(!RawInputDeviceHandler::keyboardInitialized || (keynum < BASIC_KEYCODE_AMOUNT) )
	{
		if (nappis.keysdown[keynum]) return 0;
		if (nappis.exkd[keynum]) return 1;
	} else
	{
		if (multkeyb[KEYCODE_KEYBID(keynum)][KEYCODE_KEYID(keynum)])
			return 0;
		if (multkeyb_old[KEYCODE_KEYBID(keynum)][KEYCODE_KEYID(keynum)])
			return 1;
	}
	return 0;
}



//-----------------------------------------------------------------------------
// Keyb3_GetKeyPress()
// Onko tietty nappi painettu nyt alas (poistetaan merkkaus)
//-----------------------------------------------------------------------------
int Keyb3_GetKeyPress(int keynum)
{
	assert(!UseRawInput); // RawInput -implementation not yet done. (Doesn't seem to called anywhere anyway)
	int ok=0;
	if (nappis.exkd[keynum]) return 0;
	if (nappis.keysdown[keynum]) ok=1;
	if (nappis.getpress[keynum]) ok=0;
	nappis.getpress[keynum]=1;
	return ok;
}



//-----------------------------------------------------------------------------
// Keyb3_GetKeyRelease()
// Onko tietty nappi painettu nyt alas (poistetaan merkkaus)
//-----------------------------------------------------------------------------
int Keyb3_GetKeyRelease(int keynum)
{
	assert(!UseRawInput); // RawInput -implementation not yet done. (Doesn't seem to called anywhere anyway)
	int ok=0;
	if (nappis.keysdown[keynum]) return 0;
	if (nappis.exkd[keynum]) ok=1;
	if (nappis.getrelease[keynum]) ok=0;
	nappis.getrelease[keynum]=1;
	return ok;
}



//-----------------------------------------------------------------------------
// Keyb3_SetMouseBorders()
// Asettaa hiiren rajat
//-----------------------------------------------------------------------------
void Keyb3_SetMouseBorders(int max_x, int max_y, int mouseID)
{
	// May have some compatibility issues with RawInput and need tweaking.
	if(UseRawInput)
	{
		if(mouseID != MOUSEHANDLER_ALL_MOUSES_ID)
		{
			hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].max_x=max_x;
			hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].max_y=max_y;
		} else
		{
			for(int l = 0; l < (int)RawInputDeviceHandler::numMice; l++)
			{
				hiiri[l].max_x=max_x;
				hiiri[l].max_y=max_y;
			}
		}
	} else
	{
		hiiri[0].max_x=max_x;
		hiiri[0].max_y=max_y;
	}

	window_rect.right = max_x;
	window_rect.bottom = max_y;
	ClipCursor(&window_rect);
	supposed_to_clip_cursor = true;
}



//-----------------------------------------------------------------------------
// Keyb3_SetMouseBorders()
// poistaa hiiren rajat
//-----------------------------------------------------------------------------
void Keyb3_ReleaseMouseBorders()
{
	// May have some compatibility issues with RawInput and need tweaking.
	ClipCursor(0);
	supposed_to_clip_cursor = false;
}



//-----------------------------------------------------------------------------
// Keyb3_ReadMouse()
// Lukee hiiren arvot
//-----------------------------------------------------------------------------
void Keyb3_ReadMouse(int *x,int *y,int *dx,int *dy, int mouseID)
{
	if(UseRawInput == FALSE)
	{
		if (x!=NULL) (*x)=current_pos.x;
		if (y!=NULL) (*y)=current_pos.y;
		if (dx!=NULL) (*dx)=hiiri[0].dx;
		if (dy!=NULL) (*dy)=hiiri[0].dy;
	} else {
		// RawInputDeviceHandler::MouseInfo *m_info = RawInputDeviceHandler::getMouseInfo ( mouseID );
		int mID = RawInputDeviceHandler::getMouseArrayID(mouseID);
		if(x!=NULL) (*x) = hiiri[mID].x;
		if(y!=NULL) (*y) = hiiri[mID].y;
		if(dx!=NULL) (*dx) = hiiri[mID].dx;
		if(dy!=NULL) (*dy) = hiiri[mID].dy;
	}
}



//-----------------------------------------------------------------------------
// Keyb3_SetMousePos()
// Asettaa hiiren koordinaatit
//-----------------------------------------------------------------------------
void Keyb3_SetMousePos(int x,int y, int mouseID)
{
	// May have some compatibility issues with RawInput and need tweaking.
	if(UseRawInput)
	{
		if(mouseID != MOUSEHANDLER_ALL_MOUSES_ID)
		{
			hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].x=x;
			hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].y=y;
		} else
		{
			for(int l = 0; l < (int)RawInputDeviceHandler::numMice; l++)
			{
				hiiri[l].x=x;
				hiiri[l].y=y;
			}
		}
		hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].dx=0;
		hiiri[RawInputDeviceHandler::getMouseArrayID(mouseID)].dy=0;
	} else
	{
		hiiri[0].x=x;
		hiiri[0].y=y;
		hiiri[0].dx = 0;
		hiiri[0].dy = 0;
	}

	current_pos.x = x;
	current_pos.y = y;

	mouse_x = x;
	mouse_y = y;

	old_mouse_x = x;
	old_mouse_y = y;

	if(window_active)
		SetCursorPos(x, y);

	if(UseRawInput)
	{
		RawInputDeviceHandler::MouseInfo *m_info = RawInputDeviceHandler::getMouseInfo (mouseID);
		m_info->X = x;
		m_info->Y = y;
		m_info->oldX = x;
		m_info->oldY = y;
		m_info->dX = 0;
		m_info->dY = 0;
	}

}



//-----------------------------------------------------------------------------
// Keyb3_ReadJoystick()
// Lukee joystikin arvot
//-----------------------------------------------------------------------------
void Keyb3_ReadJoystick(int joynum,int *x,int *y,int *rx,int *ry,int *throttle,int *rudder)
{
	if (joynum>=0)
	if (joynum<MAX_JOYSTICKS)
	{
		if (x!=NULL) (*x)=joy[joynum].x;
		if (y!=NULL) (*y)=joy[joynum].y;
		if (rx!=NULL) (*rx)=joy[joynum].rot_x;
		if (ry!=NULL) (*ry)=joy[joynum].rot_y;
		if (throttle!=NULL) (*throttle)=joy[joynum].throttle;
		if (rudder!=NULL) (*rudder)=joy[joynum].rudder;
	}
}

int Keyb3_GetNumberOfMouseDevices()
{
	if(UseRawInput && RawInputDeviceHandler::initialized)
		return RawInputDeviceHandler::numMice;
	if(!UseRawInput)	// DirectInput has only 1 mouse.
		return 1;
	return 0;
}

int Keyb3_DefaultMouseID()
{
	if(UseRawInput)
		return RawInputDeviceHandler::getMouseArrayID ( MOUSEHANDLER_ALL_MOUSES_ID );
	else
		return 0;
}

int Keyb3_GetMouseIDFromKeycode( int keycode )
{
	if( keycode < KEYCODE_MOUSE1_UP || keycode > KEYCODE_MOUSE4_BUTTON8)
		return -1;

	return (((keycode - KEYCODE_MOUSE0_UP) / 10) % 10) >> 1;
}

#endif
