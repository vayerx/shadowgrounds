
// This includes just temporary dummy functions and symbols until we get the actual Forcewear API :)

#pragma once

#define GLIB_OK 1

int SetUpJacket( void );
void TearDownJacket( void );
int SetEffect( int nEffect );
int SetEffect2( int nLen, int nID ) ;
int GetErrorCode (void);
const char *GetErrorText (void );

#define E_MACHINEGUN_FRONT 0
#define E_MACHINEGUN_BACK 0
#define E_BIG_BLAST_FRONT 0
#define E_BIG_BLAST_BACK 0
#define E_SMALL_BLAST_FRONT 0
#define E_SMALL_BLAST_BACK  0
#define E_PISTOL_FRONT 0
#define E_PISTOL_BACK 0
#define E_PUNCH_FRONT 0
#define E_PUNCH_BACK 0
#define E_STAB_FRONT 0
#define E_STAB_BACK 0
#define E_SHOTGUN_FRONT 0 
#define E_SHOTGUN_BACK 0
#define E_RIFLE_FRONT 0
#define E_RIFLE_BACK 0
#define E_LEFT_SIDE_HIT 0 
#define E_RIGHT_SIDE_HIT 0

