

// Keyb3 Control System (v3.5)
// (C) Sebastian Aaltonen 2000


#ifndef _KEYB3_H_
#define _KEYB3_H_

#include "RawInputMouseHandler.h"
#include "../../ui/GameController.h"

#ifdef JOY_BUTTON1
#undef JOY_BUTTON1
#endif

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// Inits/frees Keyb3 Control System
int Keyb3_Init(int CAPS);		// Returns: TRUE=ok, FALSE=error
void Keyb3_Free();

// Activates/Deactivates Keyb3 Control System.
// Deactivate Keyb3 when your window gets minimized.
// Activate Keyb3 when your window gets active again.
void Keyb3_SetActive(bool on);

// Updates all devices
// Use this function frequently (it's best to be placed in your programs main loop)
void Keyb3_UpdateDevices();

// Updates all devices. Updating devices uses a lot of processor time, so this
// function updates devices only if "int time" amount of time is elapsed.
void Keyb3_UpdateDevices_Optimized(int time);	// time in 1/1000 secs (1000=1 second)

// Waits for a keypress, and returns keycode. If returnIndividualMouse = true, returns keycode from a single mouse (e.g. KEYCODE_MOUSE1_BUTTON1),
// otherwise returns keycode which applies to every mouse (e.g. KEYCODE_MOUSE_BUTTON1)
int Keyb3_WaitKeypress( bool returnIndividualMouse = true );

// Checks if key is down (returns TRUE if down, FALSE is not)
int Keyb3_IsKeyDown(unsigned int keycode);	

// Gets keypress(/release), and clears the keypress(/release) mark. (returns TRUE if pressed, FALSE is not)
int Keyb3_GetKeyPress(int keynum);
int Keyb3_GetKeyRelease(int keynum);

// Checks is key state is changed from last update (returns TRUE if changed, FALSE is not)
int Keyb3_IsKeyPressed(int keycode);
int Keyb3_IsKeyReleased(int keycode);

// Mouse setup
void Keyb3_SetMouseBorders(int screen_x,int screen_y, int mouseID = MOUSEHANDLER_ALL_MOUSES_ID);
void Keyb3_SetMousePos(int x,int y, int mouseID = MOUSEHANDLER_ALL_MOUSES_ID);

void Keyb3_ReleaseMouseBorders();
int Keyb3_DefaultMouseID();			// Mouse ID that is controlled by every mouse for RawInput. 0 for DirectInput.
int Keyb3_GetMouseIDFromKeycode( int keycode ); // returns -1 if keycode doesn't belong to a mouse.

// Direct mouse read (give a NULL-pointer if you don't want some information) 
void Keyb3_ReadMouse(int *x,int *y,int *dx,int *dy, int mouseID = MOUSEHANDLER_ALL_MOUSES_ID);
int Keyb3_GetNumberOfMouseDevices();	// NOTICE: counts the "every mouse" -handle as a device as well, which is the last mouseID. see MouseHandler.h for more info.

// Direct joystick read. (Axis: [-1000,1000], 0=center)
// (joynum: 0=first joystick, 1=second joystick)
// (give a NULL-pointer if you don't want some information) 
void Keyb3_ReadJoystick(int joynum,int *x,int *y,int *rx,int *ry,int *throttle,int *rudder);

// Add GameController callback
void Keyb3_AddController(ui::GameController *_gc);


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define BASIC_KEYCODE_AMOUNT 534			// Keycode amount excluding additional keyboards

#define ADDITIONAL_KEYBOARD_KEYS_AMOUNT (MAX_KEYS * MAX_KEYBOARDS)

// Returns keycode for keyboard #id..
#define KEYCODE_GEN_KEYBID(id, keycode) ( BASIC_KEYCODE_AMOUNT + ((id) * MAX_KEYS) + (keycode) )

// Return keyboard id from keycode
#define KEYCODE_KEYBID(keycode) ( ( ( ( keycode ) - BASIC_KEYCODE_AMOUNT ) / MAX_KEYS ) )

// Return keycode id from keycode
#define KEYCODE_KEYID(keycode) ( ( ( ( keycode ) - BASIC_KEYCODE_AMOUNT ) % MAX_KEYS ) )


// Keyboard
// - Here is NOT all available keycodes... It's recommended that you use Keyb3_WaitKeypress to
//   ask user for key configuration (instead of using these codes).
#define KEYCODE_A				30
#define KEYCODE_B				48
#define KEYCODE_C				46
#define KEYCODE_D				32
#define KEYCODE_E				18
#define KEYCODE_F				33
#define KEYCODE_G				34
#define KEYCODE_H				35
#define KEYCODE_I				23
#define KEYCODE_J				36
#define KEYCODE_K				37
#define KEYCODE_L				38
#define KEYCODE_M				50
#define KEYCODE_N				49
#define KEYCODE_O				24
#define KEYCODE_P				25
#define KEYCODE_Q				16
#define KEYCODE_R				19
#define KEYCODE_S				31
#define KEYCODE_T				20
#define KEYCODE_U				22
#define KEYCODE_V				47
#define KEYCODE_W				17
#define KEYCODE_X				45
#define KEYCODE_Y				21
#define KEYCODE_Z				44

#define KEYCODE_0				11
#define KEYCODE_1				2
#define KEYCODE_2				3
#define KEYCODE_3				4
#define KEYCODE_4				5
#define KEYCODE_5				6
#define KEYCODE_6				7
#define KEYCODE_7				8
#define KEYCODE_8				9
#define KEYCODE_9				10

#define KEYCODE_F1				59
#define KEYCODE_F2				60
#define KEYCODE_F3				61
#define KEYCODE_F4				62
#define KEYCODE_F5				63
#define KEYCODE_F6				64
#define KEYCODE_F7				65
#define KEYCODE_F8				66
#define KEYCODE_F9				67
#define KEYCODE_F10				68
#define KEYCODE_F11				87
#define KEYCODE_F12				88

#define KEYCODE_UP_ARROW		200
#define KEYCODE_DOWN_ARROW		208
#define KEYCODE_LEFT_ARROW		203
#define KEYCODE_RIGHT_ARROW		205

#define KEYCODE_KEYPAD_1		79
#define KEYCODE_KEYPAD_2		80
#define KEYCODE_KEYPAD_3		81
#define KEYCODE_KEYPAD_4		75
#define KEYCODE_KEYPAD_5		76
#define KEYCODE_KEYPAD_6		77
#define KEYCODE_KEYPAD_7		71
#define KEYCODE_KEYPAD_8		72
#define KEYCODE_KEYPAD_9		73
#define KEYCODE_KEYPAD_0		82
#define KEYCODE_KEYPAD_DOT		83

#define KEYCODE_KEYPAD_DIVIDE		181
#define KEYCODE_KEYPAD_MULTIPLY		55
#define KEYCODE_KEYPAD_MINUS		74
#define KEYCODE_KEYPAD_PLUS			78
#define KEYCODE_KEYPAD_ENTER		156

#define KEYCODE_ESC				1
#define KEYCODE_ENTER			28
#define KEYCODE_SPACE			57
#define KEYCODE_TILDE			41
#define KEYCODE_TAB				15
#define KEYCODE_BACKSPACE		14

#define KEYCODE_CAPSLOCK		58
#define KEYCODE_NUMLOCK			69
#define KEYCODE_SCROLLLOCK		70

#define KEYCODE_SHIFT_LEFT		42
#define KEYCODE_SHIFT_RIGHT		54
#define KEYCODE_CONTROL_LEFT	29
#define KEYCODE_CONTROL_RIGHT	157
#define KEYCODE_ALT				56
#define KEYCODE_ALT_GR			184

#define KEYCODE_WINLOGO_LEFT	219
#define KEYCODE_WINLOGO_RIGHT	220
#define KEYCODE_WINDOWS_MENU	221

#define KEYCODE_PRINTSCREEN		183

#define KEYCODE_PAGE_UP		201
#define KEYCODE_PAGE_DOWN		209
#define KEYCODE_INSERT		210
#define KEYCODE_DELETE		211


// Mouse
// With rawinput, every mouses affect these.
#define KEYCODE_MOUSE_UP			256
#define KEYCODE_MOUSE_DOWN			257
#define KEYCODE_MOUSE_LEFT			258
#define KEYCODE_MOUSE_RIGHT		259
#define KEYCODE_MOUSE_WHEEL_UP	261
#define KEYCODE_MOUSE_WHEEL_DOWN	260

#define KEYCODE_MOUSE_BUTTON1		262
#define KEYCODE_MOUSE_BUTTON2		263
#define KEYCODE_MOUSE_BUTTON3		264
#define KEYCODE_MOUSE_BUTTON4		265
#define KEYCODE_MOUSE_BUTTON5		266
#define KEYCODE_MOUSE_BUTTON6		267
#define KEYCODE_MOUSE_BUTTON7		268
#define KEYCODE_MOUSE_BUTTON8		269

// Usage: For example: KEYCODE_MOUSE(2, KEYCODE_MOUSE_BUTTON1) returns keycode for button1 from mouse #2
#define KEYCODE_MOUSE(id, mousekeycode) ( (mousekeycode) + KEYCODE_MOUSE0_UP - KEYCODE_MOUSE_UP + 20 * (id) )

// Joystick
#define KEYCODE_JOY_UP			270
#define KEYCODE_JOY_DOWN		271
#define KEYCODE_JOY_LEFT		272
#define KEYCODE_JOY_RIGHT		273

#define KEYCODE_JOY_BUTTON1		274
#define KEYCODE_JOY_BUTTON2		275
#define KEYCODE_JOY_BUTTON3		276
#define KEYCODE_JOY_BUTTON4		277
#define KEYCODE_JOY_BUTTON5		278
#define KEYCODE_JOY_BUTTON6		279
#define KEYCODE_JOY_BUTTON7		280
#define KEYCODE_JOY_BUTTON8		281
#define KEYCODE_JOY_BUTTON9		282
#define KEYCODE_JOY_BUTTON10	283
#define KEYCODE_JOY_BUTTON11	284
#define KEYCODE_JOY_BUTTON12	285
#define KEYCODE_JOY_BUTTON13	286
#define KEYCODE_JOY_BUTTON14	287
#define KEYCODE_JOY_BUTTON15	288
#define KEYCODE_JOY_BUTTON16	289


// Joystick 2
#define KEYCODE_JOY2_UP			290
#define KEYCODE_JOY2_DOWN		291
#define KEYCODE_JOY2_LEFT		292
#define KEYCODE_JOY2_RIGHT		293

#define KEYCODE_JOY2_BUTTON1	294
#define KEYCODE_JOY2_BUTTON2	295
#define KEYCODE_JOY2_BUTTON3	296
#define KEYCODE_JOY2_BUTTON4	297
#define KEYCODE_JOY2_BUTTON5	298
#define KEYCODE_JOY2_BUTTON6	299
#define KEYCODE_JOY2_BUTTON7	300
#define KEYCODE_JOY2_BUTTON8	301
#define KEYCODE_JOY2_BUTTON9	302
#define KEYCODE_JOY2_BUTTON10	303
#define KEYCODE_JOY2_BUTTON11	304
#define KEYCODE_JOY2_BUTTON12	305
#define KEYCODE_JOY2_BUTTON13	306
#define KEYCODE_JOY2_BUTTON14	307
#define KEYCODE_JOY2_BUTTON15	308
#define KEYCODE_JOY2_BUTTON16	309

// Extras! -jpk
#define KEYCODE_JOY_ROT_UP			310
#define KEYCODE_JOY_ROT_DOWN		311
#define KEYCODE_JOY_ROT_LEFT		312
#define KEYCODE_JOY_ROT_RIGHT		313
#define KEYCODE_JOY_THROTTLE_UP			314
#define KEYCODE_JOY_THROTTLE_DOWN		315
#define KEYCODE_JOY_RUDDER_LEFT		316
#define KEYCODE_JOY_RUDDER_RIGHT		317

#define KEYCODE_JOY2_ROT_UP			318
#define KEYCODE_JOY2_ROT_DOWN		319
#define KEYCODE_JOY2_ROT_LEFT		320
#define KEYCODE_JOY2_ROT_RIGHT		321
#define KEYCODE_JOY2_THROTTLE_UP			322
#define KEYCODE_JOY2_THROTTLE_DOWN		323
#define KEYCODE_JOY2_RUDDER_LEFT		324
#define KEYCODE_JOY2_RUDDER_RIGHT		325

#define KEYCODE_JOY3_ROT_UP			326
#define KEYCODE_JOY3_ROT_DOWN		327
#define KEYCODE_JOY3_ROT_LEFT		328
#define KEYCODE_JOY3_ROT_RIGHT		329
#define KEYCODE_JOY3_THROTTLE_UP			330
#define KEYCODE_JOY3_THROTTLE_DOWN		331
#define KEYCODE_JOY3_RUDDER_LEFT		332
#define KEYCODE_JOY3_RUDDER_RIGHT		333

#define KEYCODE_JOY4_ROT_UP			334
#define KEYCODE_JOY4_ROT_DOWN		335
#define KEYCODE_JOY4_ROT_LEFT		336
#define KEYCODE_JOY4_ROT_RIGHT		337
#define KEYCODE_JOY4_THROTTLE_UP			338
#define KEYCODE_JOY4_THROTTLE_DOWN		339
#define KEYCODE_JOY4_RUDDER_LEFT		340
#define KEYCODE_JOY4_RUDDER_RIGHT		341

// Joystick 3
#define KEYCODE_JOY3_UP			342
#define KEYCODE_JOY3_DOWN		343
#define KEYCODE_JOY3_LEFT		344
#define KEYCODE_JOY3_RIGHT		345

#define KEYCODE_JOY3_BUTTON1	346
#define KEYCODE_JOY3_BUTTON2	347
#define KEYCODE_JOY3_BUTTON3	348
#define KEYCODE_JOY3_BUTTON4	349
#define KEYCODE_JOY3_BUTTON5	350
#define KEYCODE_JOY3_BUTTON6	351
#define KEYCODE_JOY3_BUTTON7	352
#define KEYCODE_JOY3_BUTTON8	353
#define KEYCODE_JOY3_BUTTON9	354
#define KEYCODE_JOY3_BUTTON10	355
#define KEYCODE_JOY3_BUTTON11	356
#define KEYCODE_JOY3_BUTTON12	357
#define KEYCODE_JOY3_BUTTON13	358
#define KEYCODE_JOY3_BUTTON14	359
#define KEYCODE_JOY3_BUTTON15	360
#define KEYCODE_JOY3_BUTTON16	361

// Joystick 4
#define KEYCODE_JOY4_UP			362
#define KEYCODE_JOY4_DOWN		363
#define KEYCODE_JOY4_LEFT		364
#define KEYCODE_JOY4_RIGHT		365

#define KEYCODE_JOY4_BUTTON1	366
#define KEYCODE_JOY4_BUTTON2	367
#define KEYCODE_JOY4_BUTTON3	368
#define KEYCODE_JOY4_BUTTON4	369
#define KEYCODE_JOY4_BUTTON5	370
#define KEYCODE_JOY4_BUTTON6	371
#define KEYCODE_JOY4_BUTTON7	372
#define KEYCODE_JOY4_BUTTON8	373
#define KEYCODE_JOY4_BUTTON9	374
#define KEYCODE_JOY4_BUTTON10	375
#define KEYCODE_JOY4_BUTTON11	376
#define KEYCODE_JOY4_BUTTON12	377
#define KEYCODE_JOY4_BUTTON13	378
#define KEYCODE_JOY4_BUTTON14	379
#define KEYCODE_JOY4_BUTTON15	380
#define KEYCODE_JOY4_BUTTON16	381

// With rawinput, only a single mouse affect these. DirectInput doesn't give any response to these.
#define KEYCODE_MOUSE0_UP				400
#define KEYCODE_MOUSE0_DOWN			401
#define KEYCODE_MOUSE0_LEFT			402
#define KEYCODE_MOUSE0_RIGHT			403
#define KEYCODE_MOUSE0_WHEEL_UP		405
#define KEYCODE_MOUSE0_WHEEL_DOWN	404
#define KEYCODE_MOUSE0_BUTTON8		406
#define KEYCODE_MOUSE0_BUTTON7		407
#define KEYCODE_MOUSE0_BUTTON6		408
#define KEYCODE_MOUSE0_BUTTON5		409
#define KEYCODE_MOUSE0_BUTTON4		410
#define KEYCODE_MOUSE0_BUTTON3		411
#define KEYCODE_MOUSE0_BUTTON2		412
#define KEYCODE_MOUSE0_BUTTON1		413

#define KEYCODE_MOUSE1_UP				420
#define KEYCODE_MOUSE1_DOWN			421
#define KEYCODE_MOUSE1_LEFT			422
#define KEYCODE_MOUSE1_RIGHT			423
#define KEYCODE_MOUSE1_WHEEL_UP		425
#define KEYCODE_MOUSE1_WHEEL_DOWN	424
#define KEYCODE_MOUSE1_BUTTON8		426
#define KEYCODE_MOUSE1_BUTTON7		427
#define KEYCODE_MOUSE1_BUTTON6		428
#define KEYCODE_MOUSE1_BUTTON5		429
#define KEYCODE_MOUSE1_BUTTON4		430
#define KEYCODE_MOUSE1_BUTTON3		431
#define KEYCODE_MOUSE1_BUTTON2		432
#define KEYCODE_MOUSE1_BUTTON1		433

#define KEYCODE_MOUSE2_UP				440
#define KEYCODE_MOUSE2_DOWN			441
#define KEYCODE_MOUSE2_LEFT			442
#define KEYCODE_MOUSE2_RIGHT			443
#define KEYCODE_MOUSE2_WHEEL_UP		445
#define KEYCODE_MOUSE2_WHEEL_DOWN	444
#define KEYCODE_MOUSE2_BUTTON8		446
#define KEYCODE_MOUSE2_BUTTON7		447
#define KEYCODE_MOUSE2_BUTTON6		448
#define KEYCODE_MOUSE2_BUTTON5		449
#define KEYCODE_MOUSE2_BUTTON4		450
#define KEYCODE_MOUSE2_BUTTON3		451
#define KEYCODE_MOUSE2_BUTTON2		452
#define KEYCODE_MOUSE2_BUTTON1		453

#define KEYCODE_MOUSE3_UP				460
#define KEYCODE_MOUSE3_DOWN			461
#define KEYCODE_MOUSE3_LEFT			462
#define KEYCODE_MOUSE3_RIGHT			463
#define KEYCODE_MOUSE3_WHEEL_UP		465
#define KEYCODE_MOUSE3_WHEEL_DOWN	464
#define KEYCODE_MOUSE3_BUTTON8		466
#define KEYCODE_MOUSE3_BUTTON7		467
#define KEYCODE_MOUSE3_BUTTON6		468
#define KEYCODE_MOUSE3_BUTTON5		469
#define KEYCODE_MOUSE3_BUTTON4		470
#define KEYCODE_MOUSE3_BUTTON3		471
#define KEYCODE_MOUSE3_BUTTON2		472
#define KEYCODE_MOUSE3_BUTTON1		473

#define KEYCODE_MOUSE4_UP				480
#define KEYCODE_MOUSE4_DOWN			481
#define KEYCODE_MOUSE4_LEFT			482
#define KEYCODE_MOUSE4_RIGHT			483
#define KEYCODE_MOUSE4_WHEEL_UP		485
#define KEYCODE_MOUSE4_WHEEL_DOWN	484
#define KEYCODE_MOUSE4_BUTTON8		486
#define KEYCODE_MOUSE4_BUTTON7		487
#define KEYCODE_MOUSE4_BUTTON6		488
#define KEYCODE_MOUSE4_BUTTON5		489
#define KEYCODE_MOUSE4_BUTTON4		490
#define KEYCODE_MOUSE4_BUTTON3		491
#define KEYCODE_MOUSE4_BUTTON2		492
#define KEYCODE_MOUSE4_BUTTON1		493

#define KEYCODE_MOUSE5_UP				500
#define KEYCODE_MOUSE5_DOWN			501
#define KEYCODE_MOUSE5_LEFT			502
#define KEYCODE_MOUSE5_RIGHT			503
#define KEYCODE_MOUSE5_WHEEL_UP		505
#define KEYCODE_MOUSE5_WHEEL_DOWN	504
#define KEYCODE_MOUSE5_BUTTON8		506
#define KEYCODE_MOUSE5_BUTTON7		507
#define KEYCODE_MOUSE5_BUTTON6		508
#define KEYCODE_MOUSE5_BUTTON5		509
#define KEYCODE_MOUSE5_BUTTON4		510
#define KEYCODE_MOUSE5_BUTTON3		511
#define KEYCODE_MOUSE5_BUTTON2		512
#define KEYCODE_MOUSE5_BUTTON1		513

#define KEYCODE_MOUSE6_UP				520
#define KEYCODE_MOUSE6_DOWN			521
#define KEYCODE_MOUSE6_LEFT			522
#define KEYCODE_MOUSE6_RIGHT			523
#define KEYCODE_MOUSE6_WHEEL_UP		525
#define KEYCODE_MOUSE6_WHEEL_DOWN	524
#define KEYCODE_MOUSE6_BUTTON8		526
#define KEYCODE_MOUSE6_BUTTON7		527
#define KEYCODE_MOUSE6_BUTTON6		528
#define KEYCODE_MOUSE6_BUTTON5		529
#define KEYCODE_MOUSE6_BUTTON4		530
#define KEYCODE_MOUSE6_BUTTON3		531
#define KEYCODE_MOUSE6_BUTTON2		532
#define KEYCODE_MOUSE6_BUTTON1		533


// CAPS used when Keyb3 is initialized.
// Defines which controlling devices is used.
// Keyb3 uses exclusive access, so other
// programs cannot use devices used by Keyb3.
// Normally you should use KEYB3_CAPS_ALLDEVICES,
// because that allows user to use any device he/she likes.
#define KEYB3_CAPS_KEYBOARD		0x00000001
#define KEYB3_CAPS_MOUSE			0x00000002
#define KEYB3_CAPS_JOYSTICK		0x00000004
#define KEYB3_CAPS_JOYSTICK2		0x00000008
#define KEYB3_CAPS_JOYSTICK3		16 //0x00000010
#define KEYB3_CAPS_JOYSTICK4		32 //0x00000020
#define KEYB3_CAPS_ALLDEVICES	KEYB3_CAPS_KEYBOARD | KEYB3_CAPS_MOUSE | KEYB3_CAPS_JOYSTICK | KEYB3_CAPS_JOYSTICK2 | KEYB3_CAPS_JOYSTICK3 | KEYB3_CAPS_JOYSTICK4

#define KEYB3_CAPS_USE_RAWINPUT 64	 // Default is DirectInput
#define KEYB3_CAPS_USE_DIRECTINPUT 0 // To keep the code compatible with old one.



#endif //_KEYB3_H_



