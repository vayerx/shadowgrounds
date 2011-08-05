/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Storm3D_Common defines

*/


#pragma once


//------------------------------------------------------------------
// Defines for DLL export/import
//------------------------------------------------------------------

#if defined _WIN32 && !defined __GNUC__

#ifdef COMBINE
	#define ST3D_EXP_DLLAPI
	#define ST3D_IMP_DLLAPI
#elif defined(STORM3DV2_EXPORTS)	// Do not define this inside your program!
	#define ST3D_EXP_DLLAPI __declspec(dllexport)
	#define ST3D_IMP_DLLAPI __declspec(dllimport)
#else
	#define ST3D_EXP_DLLAPI __declspec(dllimport)
	#define ST3D_IMP_DLLAPI __declspec(dllexport)
#endif

#else


#ifdef STORM3DV2_EXPORTS	// Do not define this inside your program!
	#define ST3D_EXP_DLLAPI __attribute__((visibility("default")))
	#define ST3D_IMP_DLLAPI 
#else
	#define ST3D_EXP_DLLAPI
	#define ST3D_IMP_DLLAPI
#endif

#endif


//------------------------------------------------------------------
// Iterator template stuff
//------------------------------------------------------------------
template <class A> class ST3D_EXP_DLLAPI Iterator
{
	
public:

	virtual void Next()=0;
	virtual bool IsEnd()=0;

	virtual A GetCurrent()=0;

	virtual ~Iterator() {}
};



template <class A> class ST3D_EXP_DLLAPI ICreate
{
	
public:
	virtual ~ICreate() {}
	virtual Iterator<A> *Begin()=0;
};


