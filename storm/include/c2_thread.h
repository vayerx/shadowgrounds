/*

  Common library v2
  (C) Sebastian Aaltonen 2001

  Thread routines

  Classes:
  
	Thread			- Thread class (derive your own threads from this)

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#if defined (WIN32) && !defined(__WINE__)
#include <windows.h>
#include <process.h>

#else
#include <SDL.h>
#define IGIOS_THREADS

#endif

#include "c2_common.h"



//------------------------------------------------------------------
// Thread
//------------------------------------------------------------------
class Thread
{
	// Thread handle
#ifdef IGIOS_THREADS
	SDL_Thread *thread_handle;
#else
	unsigned long thread_handle;
#endif

	// Is the thread ended?
	bool finished;		

protected:

	// Running flag
	// If running==false the thread should exit as soon as possible
	bool running;		

	// Thread code
	// Put your own code here.
	virtual void ThreadCode()=0;

public:

	// Constructor & Destructor
	Thread();
	virtual ~Thread();

	// Start executing
	void ExecuteThread();

	// Is thead finished (ie thread no longer exists)
	bool IsThreadFinished();

#ifdef IGIOS_THREADS
	friend int ThreadFunction(void *pt);
#else
	friend void ThreadFunction(void *pt);
#endif
};

