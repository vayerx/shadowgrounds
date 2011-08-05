// Copyright 2002-2004 Frozenbyte Ltd.

#include <windows.h>
#include <string>
#include "application.h"

#include "../system/Logger.h"

bool forceLowDetail = false;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int)
{
#ifdef LEGACY_FILES
	Logger::createInstanceForLogfile("viewer.log");
#else
	Logger::createInstanceForLogfile("logs/viewer.log");
#endif

	std::string fileName = cmdLine;
	//MessageBox(0, cmdLine, "Command line", MB_OK);

#ifdef NDEBUG
	if(strlen(cmdLine) > 2)
	{
		char buffer[1024] = { 0 };
		GetModuleFileName(GetModuleHandle(0), buffer, 1023);
		std::string dir = buffer;
		int end = dir.find_last_of('\\');
		dir = dir.substr(0, end);

		//MessageBox(0, dir.c_str(), "Changin to", MB_OK);
		SetCurrentDirectory(dir.c_str());

		{
			int start = 1;
			int end = fileName.find_last_of('\"');

			fileName = fileName.substr(start, end - start);
		}
	}
#endif

	frozenbyte::viewer::Application application;
	application.run(fileName);

	return 0;
}
