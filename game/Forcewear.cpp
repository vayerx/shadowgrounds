
#include "precompiled.h"
#include "Forcewear.h"

bool Forcewear::enabled = false;

unsigned int Forcewear::stringToType (const char *string)
{
	if(strcmp(string, "machinegun") == 0)
		return FW_MACHINEGUN;
	if(strcmp(string, "bigblast") == 0)
		return FW_BIG_BLAST;
	if(strcmp(string, "smallblast") == 0)
		return FW_SMALL_BLAST;
	if(strcmp(string, "pistol") == 0)
		return FW_PISTOL;
	if(strcmp(string, "punch") == 0)
		return FW_PUNCH;
	if(strcmp(string, "stab") == 0)
		return FW_STAB;
	if(strcmp(string, "shotgun") == 0)
		return FW_SHOTGUN;
	if(strcmp(string, "rifle") == 0)
		return FW_RIFLE;
	return 0;
}

// A messy macro. Simply used to check if Forcewear predefined effects are requested. Saves huge amount of writing work.
#define FW_CHECK(a)\
	if( ef == FW_ ## a && side == FW_FRONT ) \
		return SetEffect ( E_ ## a ## _FRONT );\
	if( ef == FW_ ## a && side == FW_BACK ) \
		return SetEffect ( E_ ## a ## _BACK );

int Forcewear::SendEffect(unsigned int ef, unsigned int side)
{
	//if( ef == FW_MACHINEGUN && side == FW_FRONT)
	//	return this->SetEffect (E_MACHINEGUN_FRONT);
	FW_CHECK(MACHINEGUN);
	FW_CHECK(BIG_BLAST);
	FW_CHECK(SMALL_BLAST);
	FW_CHECK(PISTOL);
	FW_CHECK(PUNCH);
	FW_CHECK(STAB);
	FW_CHECK(SHOTGUN);
	FW_CHECK(RIFLE);

	return 0;
}

// TEMPORARY ROUTINES. Delete when actual API is available
int SetUpJacket( void ) { return  GLIB_OK; };
void TearDownJacket( void ) {  };
int SetEffect( int nEffect ) { return GLIB_OK; };
int SetEffect2( int nLen, int nID ) { return GLIB_OK; };
int GetErrorCode (void) { return GLIB_OK; };
const char *GetErrorText (void ) { return "..."; };

