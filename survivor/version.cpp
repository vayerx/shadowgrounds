
#include "precompiled.h"

#include "../system/Logger.h"
#include "version.h"

#define PROGRAM_NAME_STRING "Survivor"
#define VERSION_STRING "1.0"
//#define APPLICATION_NAME_STRING "Survivor " ## VERSION_STRING ## " (" ## __DATE__ ") "
#define APPLICATION_NAME_STRING "Survivor"
#define APPLICATION_CLASSNAME_STRING "Survivor"

// --------

void log_version()
{
  Logger::getInstance()->info("---------------------------------");
  Logger::getInstance()->info(PROGRAM_NAME_STRING);
  Logger::getInstance()->info("Version " VERSION_STRING);
#ifdef FB_TESTBUILD
  Logger::getInstance()->info("Test build " APPLICATION_NAME_STRING);
#elif NDEBUG
  Logger::getInstance()->info("Build " APPLICATION_NAME_STRING);
#else
  Logger::getInstance()->info("DEBUG Build " APPLICATION_NAME_STRING);
#endif
  Logger::getInstance()->info("---------------------------------");
}

const char *get_application_name_string()
{
	return APPLICATION_NAME_STRING;
}

const char *get_application_classname_string()
{
	return APPLICATION_CLASSNAME_STRING;
}

const char *get_version_string()
{
	return VERSION_STRING;
}
