
#include "precompiled.h"
#include <stdlib.h>
#include <stdio.h>

#include "../system/Logger.h"
#include "Preprocessor.h"
#include "hiddencommand.h"

namespace util
{
	bool Preprocessor::preprocess(const char *preprocessorCheck, const char *preprocessorCommand,
		const char *inputFilename, const char *outputFilename)
	{
		bool error = false;

		const char *preproscheck = preprocessorCheck;
		const char *prepros = preprocessorCommand;
		const char *old_filename = inputFilename;
		const char *filename = outputFilename;

		bool checkPassed = false;
		if (preproscheck != NULL && preproscheck[0] != '\0')
		{
			FILE *chk = fopen(preproscheck, "rb");
			if (chk != NULL)
			{
				checkPassed = true;
				fclose(chk);
			} else {
				Logger::getInstance()->warning("Preprocessor::preprocess - Preprocessor check failed, preprocessor not will not be executed.");
			}
		} else {
			Logger::getInstance()->warning("Preprocessor::preprocess - Preprocessing without checking not allowed (check option not specified).");
		}
		if (checkPassed)
		{
			if (prepros != NULL && prepros[0] != '\0')
			{
				char *execcmd = new char[strlen(prepros) + strlen(filename) + strlen(old_filename) + 32];
				strcpy(execcmd, prepros);
				strcat(execcmd, " dhps.tmp ");
				strcat(execcmd, old_filename);
				Logger::getInstance()->debug("About to execute preprocessor...");
				Logger::getInstance()->debug(execcmd);
				//system(execcmd);
				hiddencommand(execcmd);
				remove(filename);
				rename("dhps.tmp", filename);

				// see if an error file exists and contains something
				FILE *errf = fopen("serr.tmp", "rb");
				if (errf != NULL)
				{
					fseek(errf, 0, SEEK_END);
					int errsize = ftell(errf);
					fseek(errf, 0, SEEK_SET);
					if (errsize > 0)
					{
						char *errbuf = new char[errsize + 1];
						fread(errbuf, errsize, sizeof(char), errf);
						errbuf[errsize] = '\0';
						Logger::getInstance()->error("Preprocess error, output follows...");
						Logger::getInstance()->error(errbuf);
						delete [] errbuf;
						error = true;
					}
					fclose(errf);
					remove("serr.tmp");
				}
			} else {
				error = true;
			}
		} else {
			error = true;
		}

		return !error;
	}
}
