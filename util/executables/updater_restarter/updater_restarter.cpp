
#include "windows.h"
#include <stdio.h>

int main()
{
	Sleep(1000);

	FILE *f = fopen("Updater_new.exe", "rb");
	if (f != NULL)
	{
		fclose(f);
		system("del Updater_old.exe");
		{
			int failcount = 0;
			while (true)
			{
				int renfail = rename("Updater.exe", "Updater_old.exe");
				if (!renfail) break;
				failcount++;
				if (failcount > 15)
				{
					break;
				}
				Sleep(500);
			}
		}
		{
			int failcount = 0;
			while (true)
			{
				int renfail = rename("Updater_new.exe", "Updater.exe");
				if (!renfail) break;
				failcount++;
				if (failcount > 15)
				{
					break;
				}
				Sleep(500);
			}
		}
	}

	system("start Updater.exe");
	return 0;
}

