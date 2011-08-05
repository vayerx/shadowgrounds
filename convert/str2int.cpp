#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef __GNUC__
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include <string.h>
#include "str2int.h"
#include "../util/Debug_MemoryManager.h"


char convert_strbuf[16];

// note: 32 bit int presumed...

int _last_str2int_errno = 0;

char *int2str(int value)
{
  int tmp;
  int expv;
  int strpos = 0;

  if (value < 0) 
  {
    convert_strbuf[0] = '-';
    strpos++;
    value = -value;
  }

  for (expv = 1000000000; expv > 1; expv /= 10)
  {
    if ((value / expv) != 0) break;
  }

  for (; expv > 0; expv /= 10)
  {
    tmp = (value / expv);
    value -= tmp * expv;
    convert_strbuf[strpos] = (char)('0' + tmp);
    strpos++;
  }

  convert_strbuf[strpos] = '\0';

  return convert_strbuf;
}

// does not check for possible overflow...

int str2int(const char *string)
{
  int i;
  int len = strlen(string);
  int value = 0;
  int exp = 1;

  if (len > 11) 
	{
    _last_str2int_errno = 1;
		return 0;
	}

  for (i = len - 1; i >= 0; i--)
  {
    if (string[i] >= '0' && string[i] <= '9')
    {
      value += (string[i] - '0') * exp;
      exp *= 10;
    } else {
      if (i == 0)
      {
        if (string[0] == '-')
        {
					_last_str2int_errno = 0;
          return -value;
        }
      }
      _last_str2int_errno = 1;
      return 0;
    }
  }
  _last_str2int_errno = 0;
  return value;
}


int str2int_errno()
{
  return _last_str2int_errno;
}

const char *time2str(int secs)
{
  if (secs < 0) 
  {
		return "00:00:00";
  }
	int max_secs = 99 + 99*60 + 99*60*60;
	if(secs > max_secs)
	{
		secs = max_secs;
	}

	// fill
	int hours = secs / 3600;
	int mins = (secs - hours * 3600) / 60;
	int sex = (secs - hours * 3600 - mins * 60);
	int timeVal[3] = {hours, mins, sex};

	int tmp;
	int strpos = 0;

	for(int i = 0; i < 3; i++)
	{
		int value = timeVal[i];
		if(value > 100) value = 99;

		int expv = 10;
		if ((value / expv) == 0)
		{
			expv = 1;
			// zero padding
			convert_strbuf[strpos] = '0';
			strpos++;
		}

		for (; expv > 0; expv /= 10)
		{
			tmp = (value / expv);
			value -= tmp * expv;
			convert_strbuf[strpos] = (char)('0' + tmp);
			strpos++;
		}
		if(i < 2)
		{
			convert_strbuf[strpos] = ':';
			strpos++;
		}
	}

  convert_strbuf[strpos] = '\0';


  return convert_strbuf;
}
