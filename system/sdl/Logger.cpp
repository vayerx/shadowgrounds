#include <assert.h>
#include <SDL.h>

#include "system/Logger.h"

// NOTE: this should possibly be declared volatile? or maybe the crit.section handles that by itself.

SDL_mutex *logger_critical_section;
bool logger_critical_section_inited = false;


void Logger::createLock() {
	if (!logger_critical_section_inited)
	{
		logger_critical_section_inited = true;
		logger_critical_section = SDL_CreateMutex();
	} else {
		assert(!"Logger - logger_critical_section_inited already set.");
	}
}


void Logger::destroyLock() {
	if (logger_critical_section_inited)
	{
		logger_critical_section_inited = false;
		SDL_DestroyMutex(logger_critical_section);
	} else {
		assert(!"Logger::~Logger - logger_critical_section_inited was not set.");
	}
}


void Logger::lock() {
	SDL_LockMutex(logger_critical_section);
}


void Logger::unlock() {
	SDL_UnlockMutex(logger_critical_section);
}


