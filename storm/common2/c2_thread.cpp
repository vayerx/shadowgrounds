/*

  Common library v2
  (C) Sebastian Aaltonen 2001

  Thread routines

  Classes:
  
	Thread			- Thread class (derive your own threads from this)

*/


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "c2_thread.h"


#ifdef IGIOS_THREADS

//------------------------------------------------------------------
// Thread function
//------------------------------------------------------------------
int ThreadFunction(void *pt)
{
	// Typecast
	Thread *tob=(Thread*)pt;

	// Run thread
	tob->ThreadCode();

	// Thread is finished
	tob->finished=true;

	return 0;
}


//------------------------------------------------------------------
// Constructor & Destructor
//------------------------------------------------------------------
Thread::Thread() : thread_handle(NULL), finished(true), running(false)
{
}


Thread::~Thread()
{
	// Stop thread
	running=false;

	// Wait thread to close
    SDL_WaitThread(thread_handle, NULL);
}


//------------------------------------------------------------------
// Start executing
//------------------------------------------------------------------
void Thread::ExecuteThread()
{
	running=true;
	finished=false;
	thread_handle=SDL_CreateThread(ThreadFunction, this);
}


//------------------------------------------------------------------
// Is thead finished (ie thread no longer exists)
//------------------------------------------------------------------
bool Thread::IsThreadFinished()
{
	return finished;
}

#else

//------------------------------------------------------------------
// Thread function
//------------------------------------------------------------------
void ThreadFunction(void *pt)
{
	// Typecast
	Thread *tob=(Thread*)pt;

	// Run thread
	tob->ThreadCode();

	// Thread is finished
	tob->finished=true;

	// End the thread
	_endthread();
}


//------------------------------------------------------------------
// Constructor & Destructor
//------------------------------------------------------------------
Thread::Thread() : thread_handle(0), finished(true), running(false)
{
}


Thread::~Thread()
{
	// Stop thread
	running=false;

	// Wait thread to close
	while (!finished) {Sleep(1);}
}


//------------------------------------------------------------------
// Start executing
//------------------------------------------------------------------
void Thread::ExecuteThread()
{
	running=true;
	finished=false;
	thread_handle=_beginthread(ThreadFunction,0,(void*)this);
}


//------------------------------------------------------------------
// Is thead finished (ie thread no longer exists)
//------------------------------------------------------------------
bool Thread::IsThreadFinished()
{
	return finished;
}
#endif
