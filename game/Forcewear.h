
// Simple wrapper to forcewear SDK.

/*
Forcewear effects:
 * machinegun
 * bigblast
 * smallblast
 * pistol
 * punch
 * stab
 * shotgun
 * rifle
*/

#pragma once

#include "tngaming.h"

#define FW_BACK 0
#define FW_FRONT 1

#define FW_MACHINEGUN 1
#define FW_BIG_BLAST 2
#define FW_SMALL_BLAST 3
#define FW_PISTOL 4
#define FW_PUNCH 5
#define FW_STAB 6
#define FW_SHOTGUN 7
#define FW_RIFLE 8

class Forcewear
{
	static bool enabled;

public:

	static inline bool enable() { if(!enabled) { enabled = true; return (::SetUpJacket() == GLIB_OK ? true : false ); } return false; };
	static inline bool disable() { if(enabled) { enabled = false; ::TearDownJacket(); return true; } return false; } ;

	static inline int SetEffect( int nEffect ) { if(enabled) return ::SetEffect( nEffect); return 0; };
	static inline int SetEffect2( int nLen, int nID ) { if(enabled) return ::SetEffect2( nLen, nID); return 0; };

	// Takes ID got from stringToInt as an argument, side is either FW_BACK or FW_FRONT.
	static int SendEffect( unsigned int ef, unsigned int side );

	// For scripting purproses: converts a string to a corresponding int, which is again converted to Forcewear API ID.
	static unsigned int stringToType(const char *string);

};
