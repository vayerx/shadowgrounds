#include "precompiled.h"

#include <windows.h>
#include "../Timer.h"

void Timer::init()
{
	timeBeginPeriod(1);
	update();
}


void Timer::uninit()
{
	timeEndPeriod(1);
}



  // return the current time right now
int Timer::getCurrentTime() {
	return timeGetTime();
};
