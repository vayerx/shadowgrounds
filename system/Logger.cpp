// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include "Logger.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../util/Debug_MemoryManager.h"

Logger *Logger::instance = NULL;


// not thread safe, need synchronization in case of multiple threads
Logger *Logger::getInstance()
{
	if (instance == NULL)
	{
		instance = new Logger(LOGGER_DEFAULT_OUTPUT_FILE);
	}
	return instance;
}


// not thread safe, need synchronization in case of multiple threads
void Logger::createInstanceForLogfile(const char *logfile)
{
	cleanInstance();
	if (instance == NULL)
	{
		instance = new Logger(logfile);
	}
}


void Logger::cleanInstance()
{
	if (instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
}


Logger::Logger(const char *logfile)
{
	//logLevel = LOGGER_LEVEL_DEBUG;
	//listenerLogLevel = LOGGER_LEVEL_DEBUG;
	logLevel = LOGGER_LEVEL_NONE;
	listenerLogLevel = LOGGER_LEVEL_NONE;
	listener = NULL;
	file = NULL;
	filename = new char[strlen(logfile) + 1];
	strcpy(filename, logfile);

	createLock();
}

Logger::~Logger()
{
	destroyLock();

	if (file != NULL)
	{
		fclose(file);
		file = NULL;
	}
	if (filename != NULL)
	{
		delete [] filename;
		filename = NULL;
	}
}

void Logger::writeToLog(const char *msg, bool linefeed)
{
	if (file == NULL)
	{
		file = fopen(filename, "wb");
		if (file != NULL)
		{
			time_t t;
			time(&t);
			struct tm *lt = localtime(&t);
			int hour = lt->tm_hour;
			int min = lt->tm_min;
			int year = lt->tm_year + 1900;
			int month = lt->tm_mon + 1;
			int day = lt->tm_mday;
			char minstr[3];
			minstr[0] = '0' + (min / 10);
			minstr[1] = '0' + (min % 10);
			minstr[2] = '\0';

			fprintf(file, "%d.%d.%d [%d:%s]%s", day, month, year, hour, minstr, LOGGER_LINEFEED);
		}
	}
	if (file != NULL)
	{
		if (linefeed)
			fprintf(file, LOGGER_LINEFEEDED_S, msg);
		else
			fprintf(file, "%s", msg);

		fflush(file);
	}
}

void Logger::setLogLevel(int level)
{
	logLevel = level;
}

void Logger::setListenerLogLevel(int level)
{
	listenerLogLevel = level;
}

int Logger::getListenerLogLevel()
{
	return listenerLogLevel;
}

void Logger::setListener(ILoggerListener *listener)
{
	this->listener = listener;

	for(unsigned int i = 0; i < cachedMessages.size(); ++i)
	{
		const std::pair<int, std::string> &data = cachedMessages[i];

		if(logLevel >= data.first)
			listener->logMessage(data.second.c_str(), data.first);
	}

	cachedMessages.clear();
}

void Logger::syncListener()
{
	lock();

	for(unsigned int i = 0; i < messagesToListener.size(); ++i)
	{
		const std::pair<int, std::string> &data = messagesToListener[i];
		listener->logMessage(data.second.c_str(), data.first);
	}

	messagesToListener.clear();

	unlock();
}

void Logger::debug(const char *msg)
{
	if (logLevel >= LOGGER_LEVEL_DEBUG)
	{
		lock();

		if (msg == NULL) msg = "(null)";
		writeToLog("DEBUG: ", false);
		writeToLog(msg, true);
		if (listener != NULL)
		{
			if (listenerLogLevel >= LOGGER_LEVEL_DEBUG)
			{
				messagesToListener.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_DEBUG, msg));
				assert(messagesToListener.size() != 20000);
			}
		}
		else if(msg)
		{
			if (listener == NULL)
			{
				cachedMessages.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_DEBUG, msg));
			}
		}

		unlock();
	}
}

void Logger::info(const char *msg)
{
	if (logLevel >= LOGGER_LEVEL_INFO)
	{
		lock();

		if (msg == NULL) msg = "(null)";
		writeToLog("INFO: ", false);
		writeToLog(msg, true);
		if (listener != NULL)
		{
			if (listenerLogLevel >= LOGGER_LEVEL_INFO)
			{
				messagesToListener.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_INFO, msg));
				assert(messagesToListener.size() != 20000);
			}
		}
		else if(msg)
		{
			if (listener == NULL)
			{
				cachedMessages.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_INFO, msg));
			}
		}

		unlock();
	}
}

void Logger::warning(const char *msg)
{
	if (logLevel >= LOGGER_LEVEL_WARNING)
	{
		lock();

		if (msg == NULL) msg = "(null)";
		writeToLog("WARNING: ", false);
		writeToLog(msg, true);
		if (listener != NULL)
		{
			if (listenerLogLevel >= LOGGER_LEVEL_WARNING)
			{
				messagesToListener.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_WARNING, msg));
				assert(messagesToListener.size() != 20000);
			}
		}
		else if(msg)
		{
			if (listener == NULL)
			{
				cachedMessages.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_WARNING, msg));
			}
		}

		unlock();
	}
}

void Logger::error(const char *msg)
{
	if (logLevel >= LOGGER_LEVEL_ERROR)
	{
		lock();

		if (msg == NULL) msg = "(null)";
		writeToLog("ERROR: ", false);
		writeToLog(msg, true);
		if (listener != NULL)
		{
			if (listenerLogLevel >= LOGGER_LEVEL_ERROR)
			{
				messagesToListener.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_ERROR, msg));
				assert(messagesToListener.size() != 20000);
			}
		}
		else if(msg)
		{
			if (listener == NULL)
			{
				cachedMessages.push_back(std::make_pair<int, std::string>(LOGGER_LEVEL_ERROR, msg));
			}
		}

		unlock();
	}
}

