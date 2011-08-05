#include <SDL.h>

#include "../system/Timer.h"

void Timer::init()
{
	SDL_Init(SDL_INIT_TIMER);
	update();
}


void Timer::uninit()
{
	SDL_QuitSubSystem(SDL_INIT_TIMER);
}


  // return the current time right now
int Timer::getCurrentTime() {
	return SDL_GetTicks();
};
