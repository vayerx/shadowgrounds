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
#include "c2_common.h"



//------------------------------------------------------------------
// Thread
//------------------------------------------------------------------
class Thread
{
	// Thread handle
	unsigned long thread_handle;

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

	friend void ThreadFunction(void *pt);
};

