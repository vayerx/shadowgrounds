#include <windows.h>
#include "application.h"
#include "../system/logger.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR param, int)
{	
	Logger::createInstanceForLogfile( "launcher.log" );

	frozenbyte::launcher::Application application;
	application.run();

	return 0;
}