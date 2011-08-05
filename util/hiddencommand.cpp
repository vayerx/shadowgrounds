// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "hiddencommand.h"

#include <assert.h>

#include <stdlib.h>
#include <memory.h>

#include "../system/Logger.h"

#include "Debug_MemoryManager.h"

#ifdef WIN32
#include <tchar.h>
#include <windows.h>

bool hiddencommand(const char *command, bool wait_process )
{
	if (command == NULL)
	{
		Logger::getInstance()->error("hiddencommand - Null parameter given.");
		assert(!"hiddencommand - Null parameter given.");
		return false;
	}

 	STARTUPINFO sis;
	PROCESS_INFORMATION pis;

	memset( &sis, 0, sizeof(sis) );
    sis.cb = sizeof(sis);
    memset(  &pis, 0, sizeof(pis) );

	sis.wShowWindow = SW_HIDE;
	sis.dwFlags = STARTF_USESHOWWINDOW;

	if (strlen(command) > 512)
	{
		Logger::getInstance()->error("hiddencommand - Command string too long.");
		assert(!"hiddencommand - Command string too long.");
		return false;
	}
	static char foobuf[512+1];
	strcpy(foobuf, command);

	BOOL success = CreateProcess(NULL, foobuf, NULL, NULL, false,
		CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS, NULL, NULL, &sis, &pis);

	if (success )
	{
	  // this won't work, why?
	  //int termstat;
	  //_cwait( &termstat, pidValue, _WAIT_CHILD );
	if( wait_process )
	{
		DWORD excode = STILL_ACTIVE;
		int failcount = 0;
		while (excode == STILL_ACTIVE)
		{
			GetExitCodeProcess(pis.hProcess, &excode);
			Sleep(100);
			failcount++;
			if (failcount > 100)
			{
				Logger::getInstance()->warning("hiddencommand - Command did not terminate within time limit");
				Logger::getInstance()->debug(command);
				break;
			}
		}
	}
		return true;

  } else {
		Logger::getInstance()->error("hiddencommand - Failed to run given command.");
		Logger::getInstance()->debug(command);

		return false;
  }
}


#else

#include "igios.h"

bool hiddencommand(const char *command, bool wait_process ) {
	igios_unimplemented();
	return false;
}
#endif
