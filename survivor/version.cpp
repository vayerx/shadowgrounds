
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
  LOG_INFO("---------------------------------");
  LOG_INFO(PROGRAM_NAME_STRING);
  LOG_INFO("Version " VERSION_STRING);
#ifdef FB_TESTBUILD
  LOG_INFO("Test build " APPLICATION_NAME_STRING);
#elif NDEBUG
  LOG_INFO("Build " APPLICATION_NAME_STRING);
#else
  LOG_INFO("DEBUG Build " APPLICATION_NAME_STRING);
#endif
  LOG_INFO("---------------------------------");
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
