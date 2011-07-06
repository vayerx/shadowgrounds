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
#include <windows.h>
#include <process.h>
#include "c2_thread.h"


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
Thread::Thread() : thread_handle(0), running(false), finished(true)
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

