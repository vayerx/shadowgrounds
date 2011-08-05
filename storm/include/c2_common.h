// Copyright 2002-2004 Frozenbyte Ltd.


#pragma once
#include <math.h>


//------------------------------------------------------------------
// Defines for DLL export/import
//------------------------------------------------------------------
/*#ifdef COMMON2_EXPORTS	// Do not define this inside your program!
	#define C2_EXP_DLLAPI __declspec(dllexport)
	#define C2_IMP_DLLAPI __declspec(dllimport)
#else
	#define C2_EXP_DLLAPI __declspec(dllimport)
	#define C2_IMP_DLLAPI __declspec(dllexport)
#endif*/



//------------------------------------------------------------------
// Help defines
//------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

#ifndef C2_DO_NOT_DEFINE_MACROS
#define PI				(float)3.1415926535897932384626433832795
#define DEG2RAD(x)	((x*(PI/180.0f)))
#define RAD2DEG(x)	((x*(180.0f/PI)))

#ifndef HUGE
#define HUGE			1.0e+38f
#endif

// Blows math.h
//  -- psd

#define EPSILON		1.0e-5f
#define RND()			(((float)rand())/(float)RAND_MAX)			// Random number [0,1]
#define SRND()			(((float)rand())/(float)(RAND_MAX>>1)-1.0f)	// Random number [-1,1]

#define SAFE_DELETE(p)			{if(p) {delete(p);(p)=NULL;}}
#define SAFE_DELETE_ARRAY(p)	{if(p) {delete[] (p);(p)=NULL;}}
#endif

#ifndef C2_DO_NOT_DEFINE_DATATYPES
typedef unsigned char	BYTE;
typedef unsigned short	WORD;

// turol: FIXME: ugly hack
#ifdef __GNUC__
 #ifdef __WINE__
  typedef unsigned int	DWORD;
 #else
  typedef long unsigned int DWORD;
 #endif // __WINE__
#else
typedef unsigned long	DWORD;
#endif

#endif


