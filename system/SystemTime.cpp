
#include "precompiled.h"

#include <time.h>
#include <stdio.h>

#include "SystemTime.h"


char systemtime_dateconvbuf[128];


std::string SystemTime::getCompactDataAndTime()
{
	systemtime_dateconvbuf[0] = '\0';

  time_t ltime;
	time(&ltime);
  struct tm *tims = localtime(&ltime);

	sprintf(systemtime_dateconvbuf, "%.4i%.2i%.2i%.2i%.2i", tims->tm_year+1900, tims->tm_mon+1, tims->tm_mday, tims->tm_hour, tims->tm_min);

	return std::string(systemtime_dateconvbuf);
}

std::string SystemTime::getSortableDateAndTime()
{
	systemtime_dateconvbuf[0] = '\0';

  time_t ltime;
	time(&ltime);
  struct tm *tims = localtime(&ltime);

	sprintf(systemtime_dateconvbuf, "%.4i/%.2i/%.2i - %.2i:%.2i", tims->tm_year+1900, tims->tm_mon+1, tims->tm_mday, tims->tm_hour, tims->tm_min);

	return std::string(systemtime_dateconvbuf);
}

std::string SystemTime::getSortableDate()
{
	systemtime_dateconvbuf[0] = '\0';

  time_t ltime;
	time(&ltime);
  struct tm *tims = localtime(&ltime);

	sprintf(systemtime_dateconvbuf, "%.4i/%.2i/%.2i", tims->tm_year+1900, tims->tm_mon+1, tims->tm_mday);

	return std::string(systemtime_dateconvbuf);
}

std::string SystemTime::getTime()
{
	systemtime_dateconvbuf[0] = '\0';

  time_t ltime;
	time(&ltime);
  struct tm *tims = localtime(&ltime);

	sprintf(systemtime_dateconvbuf, "%.2i:%.2i", tims->tm_hour, tims->tm_min);

	return std::string(systemtime_dateconvbuf);
}

std::string SystemTime::getTimeWithSeconds()
{
	systemtime_dateconvbuf[0] = '\0';

  time_t ltime;
	time(&ltime);
  struct tm *tims = localtime(&ltime);

	sprintf(systemtime_dateconvbuf, "%.2i:%.2i:%.2i", tims->tm_hour, tims->tm_min, tims->tm_sec);

	return std::string(systemtime_dateconvbuf);
}


unsigned int SystemTime::getSystemTimeTimestamp()
{
  time_t ltime;
	time(&ltime);
	return (int)ltime;
}


