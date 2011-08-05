#include "precompiled.h"

#include <windows.h>
#include <assert.h>

#include "../../system/Logger.h"

// NOTE: this should possibly be declared volatile? or maybe the crit.section handles that by itself.

CRITICAL_SECTION logger_critical_section;
bool logger_critical_section_inited = false;


void Logger::createLock() {
	if (!logger_critical_section_inited)
	{
		logger_critical_section_inited = true;
		InitializeCriticalSection(&logger_critical_section);
	} else {
		assert(!"Logger - logger_critical_section_inited already set.");
	}
}


void Logger::destroyLock() {
	if (logger_critical_section_inited)
	{
		logger_critical_section_inited = false;
		DeleteCriticalSection(&logger_critical_section);
	} else {
		assert(!"Logger::~Logger - logger_critical_section_inited was not set.");
	}
}


void Logger::lock() {
	EnterCriticalSection(&logger_critical_section);
}


void Logger::unlock() {
	LeaveCriticalSection(&logger_critical_section);
}


