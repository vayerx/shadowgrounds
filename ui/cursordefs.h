
#ifndef CURSORDEFS_H
#define CURSORDEFS_H

//
// Definitions for cursor states (images)
// 

#include "../ogui/Ogui.h"

#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_CLAW_PROTO)

	#define DH_CURSOR_INVISIBLE 0
	#define DH_CURSOR_ARROW 1
	#define DH_CURSOR_AIM 2
	#define DH_CURSOR_REPAIR 3
	#define DH_CURSOR_BUY 4
	#define DH_CURSOR_SELL 5
	#define DH_CURSOR_MOVE_TO 6
	#define DH_CURSOR_INVALID_TARGET 7
	#define DH_CURSOR_FORWARD 8
	#define DH_CURSOR_BACKWARD 9
	#define DH_CURSOR_LEFT 10
	#define DH_CURSOR_RIGHT 11
	#define DH_CURSOR_ROTATE_LEFT 12
	#define DH_CURSOR_ROTATE_RIGHT 13
	#define DH_CURSOR_ORBIT_LEFT 14
	#define DH_CURSOR_ORBIT_RIGHT 15
	#define DH_CURSOR_UP_LEFT 16
	#define DH_CURSOR_UP_RIGHT 17
	#define DH_CURSOR_DOWN_LEFT 18
	#define DH_CURSOR_DOWN_RIGHT 19
	#define DH_CURSOR_AIM_HEAVY 20
	#define DH_CURSOR_AIM_ALL 21
	#define DH_CURSOR_SNEAK_TO 22
	#define DH_CURSOR_SPRINT_TO 23
	#define DH_CURSOR_AIM_SPREAD1 24
	#define DH_CURSOR_AIM_SPREAD2 25
	#define DH_CURSOR_AIM_SPREAD3 26
	#define DH_CURSOR_AIM_SPREAD4 27
	#define DH_CURSOR_AIM_SPREAD5 28
	#define DH_CURSOR_AIM_RELOADING 29
	#define DH_CURSOR_AIM_PLAYER2 30
	#define DH_CURSOR_AIM_PLAYER3 31
	#define DH_CURSOR_AIM_PLAYER4 32

#elif defined(PROJECT_SURVIVOR)

	#define DH_CURSOR_INVISIBLE 0
	#define DH_CURSOR_ARROW 1
	#define DH_CURSOR_AIM_SPREAD1 2
	#define DH_CURSOR_AIM_SPREAD2 3
	#define DH_CURSOR_AIM_SPREAD3 4
	#define DH_CURSOR_AIM_SPREAD4 5
	#define DH_CURSOR_AIM_SPREAD5 6
	#define DH_CURSOR_AIM_RELOADING 7
	#define DH_CURSOR_AIM_PLAYER2 8
	#define DH_CURSOR_AIM_PLAYER3 9
	#define DH_CURSOR_AIM_PLAYER4 10

#else

	#define DH_CURSOR_INVISIBLE 0
	#define DH_CURSOR_ARROW 1

	#define DH_CURSOR_MOVE_TO 2
	#define DH_CURSOR_SNEAK_TO 3
	#define DH_CURSOR_SPRINT_TO 4

	#define _DH_CURSOR_RESERVED_5 5
	#define _DH_CURSOR_RESERVED_6 6
	#define _DH_CURSOR_RESERVED_7 7

	#define DH_CURSOR_PLAYER1_AIM 8
	#define DH_CURSOR_PLAYER2_AIM 9
	#define DH_CURSOR_PLAYER3_AIM 10
	#define DH_CURSOR_PLAYER4_AIM 11

	#define DH_CURSOR_PLAYER1_RELOADING 12
	#define DH_CURSOR_PLAYER2_RELOADING 13
	#define DH_CURSOR_PLAYER3_RELOADING 14
	#define DH_CURSOR_PLAYER4_RELOADING 15

	#define _DH_CURSOR_RESERVED_16 16
	#define _DH_CURSOR_RESERVED_17 17
	#define _DH_CURSOR_RESERVED_18 18
	#define _DH_CURSOR_RESERVED_19 19
	#define _DH_CURSOR_RESERVED_20 20
	#define _DH_CURSOR_RESERVED_21 21
	#define _DH_CURSOR_RESERVED_22 22
	#define _DH_CURSOR_RESERVED_23 23
	#define _DH_CURSOR_RESERVED_24 24
	#define _DH_CURSOR_RESERVED_25 25
	#define _DH_CURSOR_RESERVED_26 26
	#define _DH_CURSOR_RESERVED_27 27
	#define _DH_CURSOR_RESERVED_28 28
	#define _DH_CURSOR_RESERVED_29 29
	#define _DH_CURSOR_RESERVED_30 30
	#define _DH_CURSOR_RESERVED_31 31

#endif

#define DH_CURSOR_AMOUNT 32

// notice: a static limit for cursor states is defined in Ogui.cpp
// if amount of these go over that (32 at the moment), it must be increased.
// so no more cursor states fit unless the limit is changed!

extern void loadDHCursors(Ogui *ogui, int controller);
extern void unloadDHCursors(Ogui *ogui, int controller);

#endif

