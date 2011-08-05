#include "precompiled.h"
#include "userdata.h"

#ifndef WIN32
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "igios.h"

// User data directory handling

void igios_createDirectoryInternal(const std::string &path)
{
#ifndef WIN32
  static struct stat st;
  if (stat(path.c_str(),&st) == 0) {
    if (S_ISDIR(st.st_mode) && st.st_mode&S_IWUSR && st.st_mode&S_IRUSR && st.st_mode&S_IXUSR)
      return;
  } else if (mkdir(path.c_str(),0777) == 0) return;
  igiosWarning("Unable to access or create directory: %s.\n",path.c_str());
  exit(-1);
#endif
}

// Returns the path to the user data directory.
// The directory will be created if it doesnt exist yet.
const std::string &igios_initializeUserDataInternal()
{
	static bool initialized = false;
	static std::string result = "";
	if (!initialized) {
		std::string publisherDir = "";

#ifndef WIN32
		if (char *home = getenv("HOME")) {
#if defined(PROJECT_SHADOWGROUNDS)
#ifdef __APPLE__
			publisherDir = std::string(home) + "/Library";
			result = publisherDir + "/Shadowgrounds/";
#else
			publisherDir = std::string(home) + "/.frozenbyte";
			result = publisherDir + "/shadowgrounds/";
#endif
#elif defined(PROJECT_SURVIVOR)
#ifdef __APPLE__
			publisherDir = std::string(home) + "/Library";
			result = publisherDir + "/Survivor/";
#else
			publisherDir = std::string(home) + "/.frozenbyte";
			result = publisherDir + "/survivor/";
#endif
#else // #if defined(PROJECT_SHADOWGROUNDS)
#error "No project defined at compile time."
			igiosWarning("No project defined at compile time.\n");
			igios_backtrace();
			exit(-1);
#endif // #if defined(PROJECT_SHADOWGROUNDS)

		} else {
			igiosWarning("No HOME environment variable set. User files will be placed into the current directory.\n");
			result = "";
		}
		if (publisherDir != "")
			igios_createDirectoryInternal(publisherDir);
		igios_createDirectoryInternal(result);
		igios_createDirectoryInternal(result+"Profiles");
		igios_createDirectoryInternal(result+"Config");
		igios_createDirectoryInternal(result+"Screenshots");
#endif // #ifndef WIN32
		initialized = true;
  }
  return result;
}

std::string igios_getUserDataPrefix()
{
  return igios_initializeUserDataInternal();
}

std::string igios_mapUserDataPrefix(const std::string &path)
{
  if (path != "")
    return igios_initializeUserDataInternal() + igios_unmapUserDataPrefix(path);
  else
    return "";
}

std::string igios_unmapUserDataPrefix(const std::string &path)
{
  static const std::string prefix = igios_initializeUserDataInternal();
  if (path.substr(0,prefix.length()) == prefix)
    return path.substr(prefix.length());
  else
    return path;
}

void igios_initializeUserData()
{
  igios_initializeUserDataInternal();
}
