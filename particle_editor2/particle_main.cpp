// Copyright 2002-2004 Frozenbyte Ltd.

#include <windows.h>
#include "application.h"
#include <string>
#include <direct.h>

bool forceLowDetail = false;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int)
{
	char snapshotPath[1024] = { 0 };
	GetModuleFileName(GetModuleHandle(0), snapshotPath, 1023);
	int lastSlash = 0;
	for (int snc = strlen(snapshotPath); snc >= 0; snc--)
	{
		if (snapshotPath[snc] == '\\' || snapshotPath[snc] == '/')
		{
			lastSlash = snc;
			break;
		}
	}

	snapshotPath[lastSlash] = '\0';

	if (snapshotPath[0] != '\0')
	{
		//MessageBox(0, snapshotPath, "Changing to path", MB_OK);

		// when probably running from visual ide, don't change dir...
		if (strstr(snapshotPath, "\\Release") == NULL
			&& strstr(snapshotPath, "\\Debug") == NULL
			&& strstr(snapshotPath, "\\release") == NULL
			&& strstr(snapshotPath, "\\debug") == NULL)
		{
			if (_chdir(snapshotPath) != 0)
			{
				MessageBox(0, "Failed to change to correct directory", "Initialization error", MB_OK);
			}
		}
	}

	std::string startupFilename = std::string(cmdLine);

	if (startupFilename[0] == '\"')
	{
		startupFilename = startupFilename.substr(1, startupFilename.length() - 2);
	}

	frozenbyte::particle::Application application;
	application.run(startupFilename);

	return 0;
}
