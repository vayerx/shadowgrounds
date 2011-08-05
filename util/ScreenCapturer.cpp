
#include "precompiled.h"

#include "ScreenCapturer.h"

#include <Storm3D_UI.h>
#include <time.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <direct.h>
#endif

#include "../system/Logger.h"

#define SCREENSHOT_DIR "Screenshots"
#define SCREENSHOT_FILE_STR "Screenshots/shot_%d_%02d_%02d__%d_%s_%s_%c.bmp"

namespace util
{
	static char filename[300];

  void ScreenCapturer::captureScreen(IStorm3D *storm3d)
  {
    Logger::getInstance()->debug("Taking a screencapture...");

    time_t t;
    time(&t);
    struct tm *lt = localtime(&t);
    int hour = lt->tm_hour;
    int min = lt->tm_min;
    int sec = lt->tm_sec;
    int year = lt->tm_year + 1900;
    int month = lt->tm_mon + 1;
    int day = lt->tm_mday;
    char minstr[3];
    minstr[0] = '0' + (min / 10);
    minstr[1] = '0' + (min % 10);
    minstr[2] = '\0';
    char secstr[3];
    secstr[0] = '0' + (sec / 10);
    secstr[1] = '0' + (sec % 10);
    secstr[2] = '\0';

#ifdef _MSC_VER
    _mkdir(SCREENSHOT_DIR);     
#endif

    FILE *f = NULL;
    char alphabet = 'A';
    while (true)
    {
      sprintf(filename, SCREENSHOT_FILE_STR, year, month, day, hour, minstr, secstr, alphabet);

      f = fopen(filename, "rb");

      if (f != NULL)
      {
        fclose(f);
        alphabet++;
        if (alphabet >= 'Z') 
        {
          Logger::getInstance()->warning("Overwriting last screenshot.");
          break;
        }
      } else {
        break;
      }
    }

    storm3d->TakeScreenshot(filename);

    Logger::getInstance()->debug(filename);
  }

	void ScreenCapturer::captureScreenWithLastName(IStorm3D *storm3d, const char *extra_extension)
	{
		// find dot
		unsigned int i = 0;
		while(true)
		{
			if(filename[i] == 0) return;
			if(filename[i] == '.') break;
			i++;
		}
		// backup the extension
		char extension[256];
		strcpy(extension, filename + i);
		// terminate on dot
		filename[i] = 0;
		// add extra extension
		strcat(filename, extra_extension);
		// add old extension
		strcat(filename, extension);

		// take shot
    storm3d->TakeScreenshot(filename);
    Logger::getInstance()->debug(filename);
	}

}
