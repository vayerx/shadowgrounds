
#ifndef LOGGER_H
#define LOGGER_H

#include "../util/Debug_MemoryManager.h"

#include <stdio.h>
#include <vector>
#include <string>

#define LOGGER_LEVEL_NONE 0
#define LOGGER_LEVEL_ERROR 1
#define LOGGER_LEVEL_WARNING 2
#define LOGGER_LEVEL_INFO 3
#define LOGGER_LEVEL_DEBUG 4

#define LOGGER_LINEFEED "\r\n"
#define LOGGER_LINEFEEDED_S "%s\r\n"

#ifdef LEGACY_FILES
#define LOGGER_DEFAULT_OUTPUT_FILE "log.txt"
#else
#define LOGGER_DEFAULT_OUTPUT_FILE "logs/log.txt"
#endif


#define LOG_ERROR(msg) Logger::getInstance()->error(msg)
#define LOG_ERROR_W_DEBUG(msg, debugmsg) Logger::getInstance()->error(msg); Logger::getInstance()->debug(debugmsg)
#define LOG_ERROR_W_DEBUG2(msg, debugmsg, debugmsg2) Logger::getInstance()->error(msg); Logger::getInstance()->debug(debugmsg); Logger::getInstance()->debug(debugmsg2)
#define LOG_WARNING(msg) Logger::getInstance()->warning(msg)
#define LOG_WARNING_W_DEBUG(msg, debugmsg) Logger::getInstance()->warning(msg); Logger::getInstance()->debug(debugmsg)
#define LOG_WARNING_W_DEBUG2(msg, debugmsg, debugmsg2) Logger::getInstance()->warning(msg); Logger::getInstance()->debug(debugmsg); Logger::getInstance()->debug(debugmsg2)
#define LOG_INFO(msg) Logger::getInstance()->info(msg)
#define LOG_DEBUG(msg) Logger::getInstance()->debug(msg)


// an interface for a class that logger will forward it's messages to
class ILoggerListener
{
public:
	virtual ~ILoggerListener() {}
  virtual void logMessage(const char *msg, int level) = 0;
};


class Logger
{
public:
  static Logger *getInstance();

  static void cleanInstance();

	static void createInstanceForLogfile(const char *logfile);

  Logger(const char *logfile);
  ~Logger();

  // sets the level above which messages are ignored
  void setLogLevel(int level);

  // sets the level above which messages are sent to listener
  // general log level affects this also, setting this above that level
  // will have no effect. (messages that are ignored won't be sent to
  // listener either even if the listener log level would be greater).
  void setListenerLogLevel(int level);

  int getListenerLogLevel();

  // different methods for different level of log messages
  void debug(const char *msg);
  void info(const char *msg);
  void warning(const char *msg);
  void error(const char *msg);

  // sets a listener object 
  // (if you want to forward the logged messages to some other object too)
  // the message forwarded may or may not contain level info before the 
  // actual message. (probably will change over time)
  void setListener(ILoggerListener *listener);

	// call to actually pass the messages to the listener
	// (required due to safe multithreading)
  void syncListener();

private:
  static Logger *instance;

  int logLevel;
  int listenerLogLevel;
  ILoggerListener *listener;
  char *filename;
  FILE *file;

  std::vector<std::pair<int, std::string> > cachedMessages;
  std::vector<std::pair<int, std::string> > messagesToListener;

  void writeToLog(const char *msg, bool linefeed = false);

  void createLock();
  void destroyLock();
  void lock();
  void unlock();

};

#endif
