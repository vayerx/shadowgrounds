#include "precompiled.h"
#include <SDL.h>

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#  pragma warning(disable:4103)
#  pragma warning(disable:4786)
#endif

#if defined(_WIN32)
#  include <windows.h>
#endif  // _WIN32 && __GNUC__

#include "Timer.h"

// TEMP!!
#include <string.h>
#include "Logger.h"

#include "../util/Debug_MemoryManager.h"

#define TIMER_FACTOR_MULTIPLIER       256
#define TIMER_FACTOR_MULTIPLIER_SHIFT 8

int Timer::currentTime = 0;
int Timer::currentUnfactoredTime = 0;
int Timer::factorTimeAdd = 0;
int Timer::timeHaxSub = 0;
int Timer::timeFactor = TIMER_FACTOR_MULTIPLIER;

void Timer::init()
{
    SDL_Init(SDL_INIT_TIMER);
    update();
}

void Timer::uninit()
{
    SDL_QuitSubSystem(SDL_INIT_TIMER);
}

int Timer::getTime()
{
    return currentTime;
}

int Timer::getCurrentTime()
{
    // TODO: apple

#ifdef _WIN32
    return timeGetTime();

#else
    // assume linux
    return SDL_GetTicks();

#endif

}

int Timer::getUnfactoredTime()
{
    return currentUnfactoredTime;
}

void Timer::setTimeFactor(float factor)
{
    float oldFactor = (float)timeFactor / TIMER_FACTOR_MULTIPLIER;
    int oldFactorTimeAdd = factorTimeAdd;

    // must use this rounded new factor, can't use the given accurate
    // factor as that is not really used (would cause a cumulative error)
    float newFactor = float( int(factor * TIMER_FACTOR_MULTIPLIER) ) / TIMER_FACTOR_MULTIPLIER;

    Sint64 curActualTime = SDL_GetTicks();
    int newFactorTimeAdd =
        oldFactorTimeAdd + (int)( (float)curActualTime * (oldFactor - newFactor) );

    //int foo = curActualTime;
    //foo = ((foo * timeFactor) >> TIMER_FACTOR_MULTIPLIER_SHIFT);
    //foo += factorTimeAdd;

    factorTimeAdd = newFactorTimeAdd;
    timeFactor = (int)(factor * TIMER_FACTOR_MULTIPLIER);

    //int bar = curActualTime;
    //bar = ((bar * timeFactor) >> TIMER_FACTOR_MULTIPLIER_SHIFT);
    //bar += factorTimeAdd;

    //char buf[256];
    //sprintf(buf, "%d -> %d, %d, %d", foo, bar, oldFactorTimeAdd, newFactorTimeAdd);
    //Logger::getInstance()->error(buf);

}

float Timer::getTimeFactor()
{
    return timeFactor / (float)TIMER_FACTOR_MULTIPLIER;
}

void Timer::update()
{
    // NOTICE: presuming NT 10ms ticks!!! (not 95/98 55ms ticks)
    // May well need a more accurate timer!
    //currentTime = GetTickCount();
    // fixed like this...
    currentUnfactoredTime = SDL_GetTicks();
    currentTime = currentUnfactoredTime;
    currentTime = (int)( ( (Sint64)currentTime * (Sint64)timeFactor ) >> TIMER_FACTOR_MULTIPLIER_SHIFT );
    currentTime += factorTimeAdd;
    currentTime -= timeHaxSub;
}

void Timer::addTimeSub(int time)
{
    timeHaxSub += time;
}
