
#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include <string>

class SystemTime
{
public:

	// returns date+time in format "yyyymmddhhmm"
	static std::string getCompactDataAndTime();

	// returns date+time in format "yyyy/mm/dd - hh:mm"
	static std::string getSortableDateAndTime();

	// returns date in format "yyyy/mm/dd"
	static std::string getSortableDate();

	// returns time in format "hh:mm"
	static std::string getTime();

	// returns time in format "hh:mm:ss"
	static std::string getTimeWithSeconds();

	// return time in "standard" timestamp format
	static unsigned int getSystemTimeTimestamp();

	// return time in custom format
	//static std::string getFormattedDateAndTime(const char *formatString);


};

#endif

