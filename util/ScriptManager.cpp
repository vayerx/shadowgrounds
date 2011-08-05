// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "ScriptManager.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <assert.h>
#include "SimpleParser.h"
#include "../convert/str2int.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "../system/FileTimestampChecker.h"
#include "Preprocessor.h"
#include "../filesystem/input_stream_wrapper.h"

#include <memory>
#include <string>

// TEMP: for profiling info..
#include "../system/Timer.h"

// WARNING: bad dependency from util -> game
// (same goes to buildingmap)
#include "../game/SimpleOptions.h"
#include "../game/options/options_precalc.h"
#include "../util/Debug_MemoryManager.h"

#define SCRIPT_MAX_IMPORT_DEPTH 12
using namespace frozenbyte;

namespace util
{
	char scrman_currentfile[256 + 1] = { 0 };
	int scrman_currentline = 0;
	bool scrman_force_preprocess = false;

	extern bool script_doublequotes_warning;

	char scrman_importfilestack[SCRIPT_MAX_IMPORT_DEPTH][256 + 1]
    = { "s0", "s1" ,"s2" ,"s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11" };

  int scrman_importdepth = 0;

	ScriptManager *ScriptManager::instance = NULL;


	ScriptManager::ScriptManager()
	{
		scriptNameHash = new ScriptHashType();

		this->fileHash = new ScriptFileHashType();

		processor = NULL;

		keywordsAmount = 0;
		keywords = NULL;
		keywordDatatypes = NULL;

		allScripts = new LinkedList();

		clearInternalPreprocessorMacros();
#ifdef LEGACY_FILES
		loadInternalPreprocessorMacros("Data/Scripts/internal_macros.dhh");
#else
		loadInternalPreprocessorMacros("data/script/internal_macros.dhh");
#endif
	}

	
	ScriptManager::~ScriptManager()
	{
		/*
		ScriptHashType::iterator iter;
		for (iter = scriptNameHash->begin(); iter != scriptNameHash->end(); )
		{
			Script *s = (*iter).second; 			
			delete s;
			//scriptNameHash->erase(iter);
			++iter;
		}
		*/
		while (!allScripts->isEmpty())
		{
			delete (Script *)allScripts->popLast();
		}
		delete allScripts;

		scriptNameHash->clear();
		delete scriptNameHash;
	
		{		
			for(ScriptFileHashType::iterator it = fileHash->begin(); it != fileHash->end(); ++it)
			{
				delete[] it->second.first;
			}
		}
		
		fileHash->clear();
		delete fileHash;

		for (int j = 0; j < keywordsAmount; j++)
		{
			delete [] keywords[j];
		}
		delete [] keywords;
		delete [] keywordDatatypes;

	}


	ScriptManager *ScriptManager::getInstance()
	{
		// (not 100% thread safe)
		if (instance == NULL)
		{
			instance = new ScriptManager();
		}
		return instance;
	}

	void ScriptManager::cleanInstance()
	{
		Script::resetNonPermanentGlobalVariables();

		if (instance != NULL)
		{
			delete instance;
			instance = NULL;
		}
	}

	void ScriptManager::clearInternalPreprocessorMacros()
	{
		internalMacros.clear();
	}

	void ScriptManager::loadInternalPreprocessorMacros(const char *filename)
	{
		Timer::update();
		int mlStartTime = Timer::getTime();

		//filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		//if (f != NULL)

		SimpleParser sp;
		bool loadOk = sp.loadFile(filename);
		
		if (loadOk)
		{
			while (sp.next())
			{
				const char *l = sp.getLine();
// TODO: should be '#'
// (but cannot use that, 'cos simpleparser will remove those lines)
				if (l[0] == '%')
				{
// TODO: should be "#define"
// (but cannot use that, 'cos simpleparser will remove those lines)
					if (strncmp(l, "%define ", 8) == 0)
					{
						// TODO: properly parse these lines, for now just a hack!
						if (l[8] == ' ')
						{
							sp.error("ScriptManager::loadInternalPreprocessorMacros - Assuming single whitespace seperators, but encountered multiple whitespaces.");
						} else {
							int slen = strlen(l);
							int seppos = slen;
							for (int i = 8; i < slen; i++)
							{
								if (l[i] == ' ')
								{
									seppos = i;
									break;
								}
							}
							if (seppos >= slen)
							{
								sp.error("ScriptManager::loadInternalPreprocessorMacros - Invalid macro definition, parameter expected (macros expanding to empty string not allowed).");
							} else {
								if (seppos < 10)
								{
									sp.error("ScriptManager::loadInternalPreprocessorMacros - Macro names less than 2 chars not accepted.");								
								} else {
									char *tmp = new char[slen + 1];
									strcpy(tmp, l);
									tmp[seppos] = '\0';
									char *name = &tmp[8];
									char *expansion = &tmp[seppos + 1];

									std::pair<std::string, std::string> macropair;
									macropair.first = std::string(name);
									macropair.second = std::string(expansion); 
									internalMacros.push_back(macropair);

									delete [] tmp;
								}
							}
						}
					} else {
// TODO: should be "#define"
// (but cannot use that, 'cos simpleparser will remove those lines)
//						sp.error("ScriptManager::loadInternalPreprocessorMacros - Invalid non-#define line encountered.");
						sp.error("ScriptManager::loadInternalPreprocessorMacros - Invalid non-define line encountered.");
					}					
				} else {
					sp.error("ScriptManager::loadInternalPreprocessorMacros - Invalid non-preprocessor line encountered.");
				}
			}

			/*
			int flen = fb_fsize(f);

			if (flen > 0)
			{
				char *buf = new char[flen + 1];
				int got = fb_fread(buf, flen, 1, f);
				if (got == 1)
				{
					// TODO:

				} else {
					Logger::getInstance()->warning("ScriptManager::loadInternalPreprocessorMacros - Error reading internal preprocessor macros file.");
					Logger::getInstance()->debug(filename);
				}
			} else {
				Logger::getInstance()->warning("ScriptManager::loadInternalPreprocessorMacros - Empty internal preprocessor macros file.");
				Logger::getInstance()->debug(filename);
			}
			
			fb_fclose(f);
			*/

		} else {
			Logger::getInstance()->warning("ScriptManager::loadInternalPreprocessorMacros - Failed to open internal preprocessor macros file.");
			Logger::getInstance()->debug(filename);
		}

		Timer::update();
		int mlEndTime = Timer::getTime();

		Logger::getInstance()->debug("internal preprocessor macros load time");
		Logger::getInstance()->debug(int2str(mlEndTime - mlStartTime));
		Logger::getInstance()->debug("total number of macros loaded so far:");
		Logger::getInstance()->debug(int2str(internalMacros.size()));
	}


	void ScriptManager::error(const char *err, int linenum, bool isError)
	{
		char *buf = new char[strlen(err) + 1 + 60 + strlen(scrman_currentfile)];
		strcpy(buf, err);
		strcat(buf, " (file ");
		strcat(buf, scrman_currentfile);
		strcat(buf, ", line ");
		strcat(buf, int2str(linenum));
		strcat(buf, ")");
		if (isError)
			Logger::getInstance()->error(buf);
		else
			Logger::getInstance()->warning(buf);
		delete [] buf;
	}


	void ScriptManager::scriptCompileError(const char *err, bool isError)
	{
		error(err, scrman_currentline, isError);
	}

	void ScriptManager::setForcePreprocessForNextLoad(bool forcepp)
	{
		scrman_force_preprocess = forcepp;
	}

	void ScriptManager::loadScripts(const char *filename_, const char *relativeToFilenamePath, bool replace)
	{
		std::string filenameStr = std::string(filename_);
		for (int tolo = 0; tolo < (int)filenameStr.length(); tolo++)
		{
			filenameStr[tolo] = tolower(filenameStr[tolo]);
		}
		if (relativeToFilenamePath != NULL)
		{
#ifdef PROJECT_SHADOWGROUNDS
			// nop
#else
			std::string pathStr = std::string(relativeToFilenamePath);
			if (filenameStr.find("data/", 0) != 0
				&& filenameStr.find("Data/", 0) != 0)
			{
				std::string::size_type lastSlash = pathStr.rfind("/", pathStr.length());
				if (lastSlash != std::string::npos)
				{
					pathStr = pathStr.substr(0, lastSlash+1);
				}
				filenameStr = pathStr + filenameStr;
			}
#endif
		}

		const char *filename = filenameStr.c_str();

		assert(filename != NULL);
		if (filename == NULL)
		{
			Logger::getInstance()->error("ScriptManager::loadScripts - Null filename parameter given.");
			return;
		}

		Timer::update();
		int loadStartTime = Timer::getTime();

		Logger::getInstance()->debug("About to load script files:");
		Logger::getInstance()->debug(filename);

		char *filename_preprocessed = new char[strlen(filename) + 10];
		strcpy(filename_preprocessed, filename);
		if (strlen(filename) > 4 
			&& strcmp(&filename[strlen(filename) - 4], ".dhs") == 0)
		{
			filename_preprocessed[strlen(filename) - 4] = '\0';
		}
		strcat(filename_preprocessed, ".dhps");
		const char *old_filename = filename;
		filename = filename_preprocessed;
#ifdef SCRIPT_PREPROCESS
		bool upToDate = FileTimestampChecker::isFileNewerOrSameThanFile(filename, old_filename);
		bool ppWarning = false;

		if (!game::SimpleOptions::getBool(DH_OPT_B_AUTO_SCRIPT_PREPROCESS))
		{
			if (!upToDate)
			{
				ppWarning = true;
			}
			upToDate = true;
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_FORCE_SCRIPT_PREPROCESS))
		{
			Logger::getInstance()->warning("ScriptManager::loadScripts - Preprocessed script is recreated (force preprocess on).");
			Logger::getInstance()->debug(old_filename);
			upToDate = false;
		}

		if (scrman_force_preprocess)
		{
			upToDate = false;
			scrman_force_preprocess = false;
		}


		bool noPPFile = false;

		if (!upToDate)
		{
			Logger::getInstance()->debug("Checking for a nopp-flagged script...");
			FILE *noppf = fopen(old_filename, "rb");
			if (noppf != NULL)
			{
				fseek(noppf, 0, SEEK_END);
				int flen = ftell(noppf);
				fseek(noppf, 0, SEEK_SET);
				if (flen > 12)
				{
					char noppbuf[12+1] = { 0 };
					fread(noppbuf, 11, 1, noppf);

					if (strncmp(noppbuf, "#!dhs -nopp", 11) == 0)
					{
						Logger::getInstance()->debug("About to copy a nopp-flagged script...");
						// no preprocess script, just copy the original .dhs
						
						fseek(noppf, 0, SEEK_SET);
												
						char *noppbuf2 = new char[flen + 1];
						noppbuf2[0] = '\0';
						int got = fread(noppbuf2, flen, 1, noppf);
						if (got == 1)
						{
							FILE *noppfout = fopen(filename, "wb");
							if (noppfout != NULL)
							{
								fwrite("// NO PP!!!", 11, 1, noppfout);
								int gotwr = fwrite(&noppbuf2[11], flen-11, 1, noppfout);
								if (gotwr != 1)
								{
									Logger::getInstance()->warning("ScriptManager::loadScripts - Failed to write nopp-flagged preprocessed script file.");
								}
								fclose(noppfout);
							} else {
								Logger::getInstance()->warning("ScriptManager::loadScripts - Failed to open nopp-flagged preprocessed script file for writing.");
							}
						} else {
							Logger::getInstance()->warning("ScriptManager::loadScripts - Failed to read a file with preprocess off flag.");
						}
						noppbuf2[flen] = '\0';
						delete [] noppbuf2;

						noPPFile = true;
					}
				}
				fclose(noppf);
			}
		}

		if (!noPPFile)
		{
			if (ppWarning)
			{
				Logger::getInstance()->warning("ScriptManager::loadScripts - Preprocessed script is not up to date, but it will not be recreated (auto preprocess off).");
				Logger::getInstance()->debug(old_filename);
			}
		}

		if (!upToDate)
		{
			if (!noPPFile)
			{
				// normal external program preprocessor usage...
				const char *preproscheck = game::SimpleOptions::getString(DH_OPT_S_SCRIPT_PREPROCESSOR_CHECK);
				const char *prepros = game::SimpleOptions::getString(DH_OPT_S_SCRIPT_PREPROCESSOR);
				util::Preprocessor::preprocess(preproscheck, prepros, old_filename, filename);
				// end of normal external preprocess
			}

		}
#endif
		if (strlen(filename) < 256)
			strcpy(scrman_currentfile, filename);

		Timer::update();
		int ppEndTime = Timer::getTime();

		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");

		if (f == NULL)
		{
#ifdef SCRIPT_PREPROCESS
			if (!noPPFile)
				error("Could not open script file or preprocess failed.", 0, true);
			else
#endif
				error("Could not open script file.", 0, true);

			delete [] filename_preprocessed;
			return;
		}

		//fseek(f, 0, SEEK_END);
		//int flen = ftell(f);
		//fseek(f, 0, SEEK_SET);
		int flen = filesystem::fb_fsize(f);

		char *buf = new char[flen + 1];

		int datalen = filesystem::fb_fread(buf, sizeof(char), flen, f);
		buf[datalen] = '\0';

		filesystem::fb_fclose(f);

#ifdef _DEBUG
#ifdef FROZENBYTE_DEBUG_MEMORY
		char dbuf[256 + 20];
		if (filename != NULL
			&& strlen(filename) < 256)
		{
			strcpy(dbuf, "load script ");
			strcat(dbuf, filename);
			frozenbyte::debug::debugSetAllocationInfo(dbuf);
		}
#endif
#endif

		loadMemoryScripts(filename, buf, datalen, replace);

		Timer::update();
		int loadEndTime = Timer::getTime();

		Logger::getInstance()->debug("Script files load done for:");
		Logger::getInstance()->debug(filename);

		if (ppEndTime - loadStartTime >= 100
			|| loadEndTime - ppEndTime >= 100)
		{
			Logger::getInstance()->debug("Script files took significant time to load, time stats follow...");
			Logger::getInstance()->debug("preprocess time");
			Logger::getInstance()->debug(int2str(ppEndTime - loadStartTime));
			Logger::getInstance()->debug("compile time");
			Logger::getInstance()->debug(int2str(loadEndTime - ppEndTime));
		}

		delete [] buf;
		delete [] filename_preprocessed;

		// TEMP: print script command/data arrays allocation info
		/*
		int totalAllocSize = 0;
		LinkedListIterator allocIter(allScripts);
		while (allocIter.iterateAvailable())
		{
			Script *s = (Script *)allocIter.iterateNext();
			totalAllocSize += s->allocedSize;
		}
		Logger::getInstance()->debug("Current total script array allocations size (in commands, not bytes):");
		Logger::getInstance()->debug(int2str(totalAllocSize));
		*/

	}

	bool ScriptManager::doInternalPreprocess(const char *filename, char **bufOut, int *datalenOut, const char *buf_unchanged, int datalen_unchanged)
	{
		assert(datalenOut != NULL);
		assert(bufOut != NULL);
		assert(buf_unchanged != NULL);

		Timer::update();
		// int ippStartTime = Timer::getTime();

		int expandedAmount = 0;

		int alloced = datalen_unchanged + 1 + 32;
		int datalen = datalen_unchanged;

		char *buf = new char[alloced];
		int bufp = 0;

		// FIXME: will not work correctly if file starts with a macro line?
		// (macros found only after first line break/semi colon seperator)

		for (int i = 0; i < datalen_unchanged; i++)
		{
			// TODO: handle comment lines!!!

			buf[bufp] = buf_unchanged[i];
			bufp++;

			if (buf_unchanged[i] == '\n'
				|| buf_unchanged[i] == '\r'
				|| buf_unchanged[i] == ';')
			{
				for (int j = i+1; j < datalen_unchanged; j++)
				{
					if (buf_unchanged[j] != ' ' && buf_unchanged[j] != '\t')
					{
						// NOTE: this works for script commands only, as only the beginning of each line is compared to
						// the internal macros after skipping whitespaces.
						// (if params need to support internal macros, then that is a totally diffent case)
						for (int mac = 0; mac < (int)internalMacros.size(); mac++)
						{
							int namelen = internalMacros[mac].first.length();
							int expandlen = internalMacros[mac].second.length();
							if (strncmp(&buf_unchanged[j], internalMacros[mac].first.c_str(), namelen) == 0
								&& !(buf_unchanged[j + namelen] >= 'A' && buf_unchanged[j + namelen] <= 'Z')
								&& !(buf_unchanged[j + namelen] >= 'a' && buf_unchanged[j + namelen] <= 'z')
								&& !(buf_unchanged[j + namelen] >= '0' && buf_unchanged[j + namelen] <= '9')
								&& !(buf_unchanged[j + namelen] == '_'))
							{
								// FIXME: this check may be incorrect...
								// possibly off by 1 or something?

								// need to reallocate a bigger output buffer?
								if (bufp + expandlen + (datalen_unchanged - j) + 1 >= alloced)
								{
									if (alloced*2 < bufp + expandlen + (datalen_unchanged - j) + 1)
									{
										alloced = bufp + expandlen + (datalen_unchanged - j) + 1;
										assert(alloced >= datalen_unchanged + 1 + 32);
									}
									alloced *= 2;
									char *oldbuf = buf;
									buf = new char[alloced];
									for (int k = 0; k < alloced/2; k++)
									{
										buf[k] = oldbuf[k];
									}
									for (int k = alloced/2; k < alloced; k++)
									{
										buf[k] = '\0';
									}
									delete [] oldbuf;
								}

								strcpy(&buf[bufp], internalMacros[mac].second.c_str());

								i += namelen;
								bufp += expandlen;

								expandedAmount++;
								break;
							}
						}

						break;
					} else {
						buf[bufp] = buf_unchanged[j];
						i++;
						bufp++;
					}
				}
			}
		}

		datalen = bufp;

		buf[datalen] = '\0';

		*datalenOut = datalen;
		*bufOut = buf;

		Timer::update();
		// int ippEndTime = Timer::getTime();

		/*
		Logger::getInstance()->debug("script internal preprocess time");
		Logger::getInstance()->debug(int2str(ippEndTime - ippStartTime));
		Logger::getInstance()->debug("number of macros expanded:");
		Logger::getInstance()->debug(int2str(expandedAmount));
		*/

		return true;
	}


	void ScriptManager::loadMemoryScripts(const char *filename, char *buf_unchanged, int datalen_unchanged, bool replace)
	{
		assert(buf_unchanged[datalen_unchanged] == '\0');

		if (strlen(filename) < 256)
			strcpy(scrman_currentfile, filename);
    else
      scrman_currentfile[0] = '\0';

		int hc;
		SCRIPT_HASHCODE_CALC(filename, &hc);

		/*
		Logger::getInstance()->debug("ScriptManager::loadScripts - Calculated hashcode for script filename:");
		Logger::getInstance()->debug(filename);
		Logger::getInstance()->debug(int2str(hc));
		*/

		ScriptFileHashType::iterator iter = fileHash->find(hc);
		if (iter != fileHash->end())
		{
			char *loaded = (*iter).second.first; 
			if (loaded != NULL) 
			{
				if (replace)
				{
					Logger::getInstance()->debug("ScriptManager::loadScripts - Reloading script file...");
					Logger::getInstance()->debug(filename);
					//psdhax
					delete[] iter->second.first;
					fileHash->erase(iter);
				} else {
					Logger::getInstance()->debug("ScriptManager::loadScripts - Script file already loaded, skipping...");
					Logger::getInstance()->debug(filename);

					for (int i = 0; i < scrman_importdepth; i++)
					{
						if (scrman_importfilestack[i] != NULL 
							&& strcmp(scrman_importfilestack[i], filename) == 0)
						{
							Logger::getInstance()->warning("ScriptManager::loadScripts - Cyclic script import chain.");
							assert(scrman_importdepth > 0);
							Logger::getInstance()->debug(scrman_importfilestack[scrman_importdepth - 1]);
							Logger::getInstance()->debug(filename);
						}
					}

					return;
				}
			}
		}


		char *buf = NULL;
		int datalen = 0;

		bool ippSuccess = doInternalPreprocess(filename, &buf, &datalen, buf_unchanged, datalen_unchanged);
		if (!ippSuccess)
		{
			Logger::getInstance()->error("ScriptManager::loadScripts - Internal script preprocessor failed to preprocess script.");
			Logger::getInstance()->debug(filename);
			return;
		}

		// NEW: create a non-const buffer copy to insert to hash...
		// FIXME: this leaks memory.
		// (should perhaps delete the filename copy when the fileHash is cleared)
		char *filename_copy = new char[strlen(filename) + 1];
		strcpy(filename_copy, filename);

		unsigned long timestamp = FileTimestampChecker::getFileTimestamp(filename_copy);
		fileHash->insert(std::pair<int, std::pair<char *, unsigned long> > (hc, std::pair<char *, unsigned long>(filename_copy, timestamp)));

		Logger::getInstance()->debug("ScriptManager::loadScripts - Parsing data...");
		Logger::getInstance()->debug(filename);

		Script *currentScript = NULL;

		int i;
		bool atCommentLine = false;
		bool insideQuotes = false;
		int lineNumber = 1;
		int lastpos = 0;

    int ifDepth = 0;
    int autoNestedIfs = 0;
    int manualNestedIfs = 0;
    int ifNestMask = 1;
    int isAlreadyManuallyNested = 0;
    bool loopReasonForManualNest = false;
    bool autoNestingNotAllowed = false;
    bool nestedInsideLoop = false;
    bool didABreakLoop = false;

		int commandCountIf = 0;
		int commandCountEndif = 0;
		int commandCountThen = 0;
		int commandCountSub = 0;
		int commandCountEndSub = 0;
		int commandCountLoop = 0;
		int commandCountEndLoop = 0;
		int commandCountSelect = 0;
		int commandCountEndSelect = 0;
		bool inSub = false;
		int selectDepth = 0;

		for (i = 0; i < datalen; i++)
		{
			// WARNING: semicolon now treated like a newline!
			// should check that it really works and won't cause problems!

			if (buf[i] == '\r' || buf[i] == '\n' 
				|| (buf[i] == ';' && !insideQuotes && !atCommentLine))
			{
				if (buf[i] == '\n')
				{
					lineNumber++;
				}

				bool endCommentLine = false;

				// TODO: linenumbering goes wrong if carriage return missing 
				// so if non-windows text format, it's real line number + 1 
				if (buf[i] == '\r' || buf[i] == '\n') 
				{
					endCommentLine = true;
				}


				buf[i] = '\0';

				// remove trailing spaces
				for (int j = i - 1; j >= lastpos; j--)
				{
					if (buf[j] == ' ') 
						buf[j] = '\0';
					else 
						break;
				}

				if (strncmp(&buf[lastpos], "//", 2) == 0
					|| buf[lastpos] == '#')
				{
					atCommentLine = true;

					// is this a preprocessor line maybe?
					if (buf[lastpos] == '#' && buf[lastpos + 1] == ' '
						&& buf[lastpos + 2] >= '0' && buf[lastpos + 2] <= '9')
					{
						// then start line numbering from here..
						lineNumber = 0;
					}
				}

				// process if not empty and not start with comments
				if (buf[lastpos] != '\0' && !atCommentLine)
				{
					int tokenSep = -1;
					for (int k = lastpos; k < i; k++)
					{
						if (buf[k] == ' ')
						{
							buf[k] = '\0';
							tokenSep = k;
							break;
						}
					}
					char *cmd = &buf[lastpos];
					char *data = NULL;
					if (tokenSep != -1) 
					{
						data = &buf[tokenSep + 1];
						while (data[0] == ' ' || data[0] == '\t')
						{
							data = &data[1];
						}
						for (int k = strlen(data); k >= 0; k--)
						{
							if (data[k] != ' ' && data[k] != '\t')
							{
								break;
							}
							data[k] = '\0';
						}
					}

					bool lineok = false;

					if (strcmp(cmd, "import") == 0)
					{
            if (scrman_importdepth < SCRIPT_MAX_IMPORT_DEPTH)
            { 
              if (scrman_importdepth >= 0)
              {
                if (scrman_currentfile != NULL)
                {
                  strcpy(scrman_importfilestack[scrman_importdepth], scrman_currentfile);
                } else {
                  scrman_importfilestack[scrman_importdepth][0] = '\0';
                }
              }
            } else {
							Logger::getInstance()->error("ScriptManager::loadMemoryScripts - Script import depth over maximum limit.");
              assert(!"ScriptManager::loadMemoryScripts - Script import depth over maximum limit.");
            }
            scrman_importdepth++;

						if (data[0] == '\"')
						{
							char *stringedData = new char[strlen(&data[1]) + 1];
							strcpy(stringedData, &data[1]);
							for (int stfix = strlen(stringedData) - 1; stfix >= 0; stfix--)
							{
								if (stringedData[stfix] == '\"')
								{
									stringedData[stfix] = '\0';
									break;
								}
							}
							this->loadScripts(stringedData, filename, false);
							delete [] stringedData;
						} else {
							this->loadScripts(data, filename, false);
						}

            //Logger::getInstance()->error(scrman_currentfile);

            scrman_importdepth--;
            if (scrman_importdepth < SCRIPT_MAX_IMPORT_DEPTH)
            { 
              if (scrman_importdepth >= 0)
              {
                strcpy(scrman_currentfile, scrman_importfilestack[scrman_importdepth]);
              }
            } else {
              assert(!"ScriptManager::loadMemoryScripts - Script import depth over maximum limit.");
            }
            assert(scrman_importdepth >= 0);

						lineok = true;
					}

					if (strcmp(cmd, "script") == 0)
					{ 					 
						if (currentScript != NULL)
						{
							error("Expected endScript, script block not properly ended before next script block.", lineNumber, true);
						}

						Script *tmp = getScript(data);
						if (tmp != NULL)
						{
							if (replace)
							{
								int rhc;
								SCRIPT_HASHCODE_CALC(data, &rhc);

								ScriptHashType::iterator iter = scriptNameHash->find(rhc);
								if (iter != scriptNameHash->end())
								{
									allScripts->remove(tmp);
									// FIXME: will crash if the script is being run!!!
									// (no script processes with references to that
									// script are allowed!)
									// TODO: ...
									delete tmp;
									//psdhax
									scriptNameHash->erase(iter);
								}
							} else {
								if (tmp->getName() != NULL
									&& data != NULL
									&& strcmp(data, tmp->getName()) != 0)
								{
									error("Script name hashcode calculation error, same hashcode for different script names.", lineNumber, true);
									Logger::getInstance()->debug(data);
									Logger::getInstance()->debug(tmp->getName());
								} else {
									error("Redefinition of script.", lineNumber, true);
								}
							}
						}

						currentScript = new Script();
						currentScript->name = new char[strlen(data) + 1];
						strcpy(currentScript->name, data);

						currentScript->processorKeywordsAmount = keywordsAmount;
						currentScript->processorKeywords = keywords;
						currentScript->processorDatatypes = keywordDatatypes;

						currentScript->processor = processor;

						int hc;
						SCRIPT_HASHCODE_CALC(currentScript->name, &hc);

						scriptNameHash->insert(std::pair<int, Script *> (hc, currentScript));
						allScripts->append(currentScript);

						lineok = true;
					}

					if (strcmp(cmd, "endScript") == 0)
					{ 					 
						if (currentScript == NULL)
						{
							error("Unexpected endScript, no script block started before end.", lineNumber, true);
						} else {
							currentScript->optimizeJumps();
						}
						currentScript = NULL;
						lineok = true;

						if (commandCountSub != commandCountEndSub)
						{
							error("Script parse error, invalid sub block encountered.", lineNumber, true);
						}
						commandCountSub = 0;
						commandCountEndSub = 0;
					}

					if (!lineok)
					{
						if (currentScript != NULL)
						{
              if (data != NULL && data[0] == '"'
                && (data[strlen(data) - 1] == ' '
                || data[strlen(data) - 1] == '\t'))
              {
                error("Whitespaces after likely string data, probably unintended?", lineNumber, false);
              }

							if (data != NULL && data[0] == '"' 
								&& data[strlen(data) - 1] == '"')
							{
								char *datastrip = new char[strlen(data) + 1];
								strcpy(datastrip, &data[1]);
								datastrip[strlen(datastrip) - 1] = '\0';
								scrman_currentline = lineNumber;
								lineok = currentScript->addCommand(cmd, datastrip);
								delete [] datastrip;
							} else {
                if (strcmp(cmd, "return") == 0)
                {
                  if (ifDepth >= 16)
                  {
                    error("Nesting depth too great for nested return (if depth greater than 16).", lineNumber, true);
									} else {
										if (ifDepth >= 2)
										{
											char depthbuf[16];
											strcpy(depthbuf, int2str((ifDepth << (32-4))));
											bool retconvok = currentScript->addCommand("_returnMultiple", depthbuf); 
											if (!retconvok)
											{
												error("Internal error while trying to convert nested return (bug).", lineNumber, true);
												assert(!"nested return conversion bugged.");
											}
										}
									}
                }
                if (strcmp(cmd, "loop") == 0)
                {
									commandCountLoop++;
                  autoNestingNotAllowed = true;
                  if ((isAlreadyManuallyNested & (ifNestMask * 2)) != 0)
                  {
                    loopReasonForManualNest = true;
                  }
                  // not nested inside the nested loop ;)
                  // should stack these though, but who cares...
                  nestedInsideLoop = false;
                }
                if (strcmp(cmd, "endLoop") == 0)
                {
									commandCountEndLoop++;
                  autoNestingNotAllowed = false;
                  nestedInsideLoop = false;
                  didABreakLoop = false;
                }
                if (strcmp(cmd, "if") == 0)
                {
									commandCountIf++;
                  ifDepth++;
                  ifNestMask *= 2;
                  // just some limit to make sure this thing does not 
                  // totally screw something up.
                  if (ifDepth >= 8)
                  {
                    error("If nesting depth over maximum allowed limit.", lineNumber, true);
                    assert(!"If nesting depth over maximum allowed limit.");
                  }
                  if (ifDepth >= 2)
                  {
                    if ((isAlreadyManuallyNested & ifNestMask) == 0)
                    {
                      //if (autoNestingNotAllowed)
                      //{
                      //  error("Cannot automatically nest if block because of loop (need to nest the loop manually).", lineNumber, true);
                      //  assert(!"Cannot automatically nest if block because of loop.");
                      //} else {
												scrman_currentline = lineNumber;
								        bool autonestok = currentScript->addCommand("_externCallPush", NULL); 
                        if (!autonestok)
                        {                      
                          error("Internal error while trying to autonest if blocks, at _externCallPush (bug).", lineNumber, true);
                          assert(!"if autonesting bugged at _externCallPush.");
                        }
                        autoNestedIfs++;
                      //}
                    }
                  }
                }
								scrman_currentline = lineNumber;
								script_doublequotes_warning = true;
								lineok = currentScript->addCommand(cmd, data);
								script_doublequotes_warning = false;
                if (strcmp(cmd, "endif") == 0)
                {
									commandCountEndif++;
                  if (ifDepth >= 2)
                  {
                    if ((isAlreadyManuallyNested & ifNestMask) == 0)
                    {
                      if (didABreakLoop)
                      {
                        error("Bad automatically nested if block because of breakLoop (need to nest manually).", lineNumber, true);
                        assert(!"Bad automatically nested if block because of breakLoop.");
                      }
                      didABreakLoop = false;

											scrman_currentline = lineNumber;
								      bool autonestok = currentScript->addCommand("_externCallPop", NULL); 
                      if (!autonestok)
                      {                      
                        error("Internal error while trying to autonest if blocks, at _externCallPop (bug).", lineNumber, true);
                        assert(!"if autonesting bugged at _externCallPop.");
                      }
                    }
									} else {
										didABreakLoop = false;
                  }
                  ifDepth--;
                  //isAlreadyManuallyNested &= (0xffff ^ ifNestMask);
                  ifNestMask /= 2;
                  if (ifDepth < 0)
                  {
                    error("Internal error while trying to autonest if blocks, if depth below zero.", lineNumber, true);
                    assert(!"Internal error while trying to autonest if blocks, if depth below zero.");
                  }
                }
                if (strcmp(cmd, "breakLoop") == 0)
                {
                  didABreakLoop = true;
                  if (nestedInsideLoop)
                  {                    
                    error("Possibly an erronous use of manual _externCallPush/Pop nesting inside loop.", lineNumber, false);
                    assert(!"Possibly an erronous use of manual _externCallPush/Pop nesting inside loop.");
                  }
                }
                if (strcmp(cmd, "_externCallPush") == 0)
                {
                  manualNestedIfs++;
                  isAlreadyManuallyNested |= (ifNestMask * 2);
                  if (autoNestingNotAllowed)
                  {
                    nestedInsideLoop = true;
                  }
                }
                if (strcmp(cmd, "_externCallPop") == 0)
                {
                  isAlreadyManuallyNested &= (0xffff ^ (ifNestMask * 2));
                  nestedInsideLoop = false;
                }
                if (strcmp(cmd, "then") == 0)
                {
									commandCountThen++;
									if (ifDepth == 0)
									{
                    error("then encountered outside if block (if expected before then).", lineNumber, true);
									}
								}
                if (strcmp(cmd, "local") == 0)
                {
									if (selectDepth != 0
										|| commandCountLoop != 0
										|| commandCountIf != 0)
									{
                    error("Local variables must be declared in the beginning of the sub.", lineNumber, true);
									}
									inSub = true;
								}
                if (strcmp(cmd, "sub") == 0)
                {
									commandCountSub++;
									if (inSub)
									{
                    error("sub encountered while inside sub (endSub expected after sub).", lineNumber, true);
									}
									inSub = true;
								}
                if (strcmp(cmd, "endSub") == 0)
                {
									if (!inSub)
									{
                    error("endSub encountered without sub (sub expected before endSub).", lineNumber, true);
									}
									commandCountEndSub++;
									inSub = false;

									if (commandCountThen != commandCountIf || commandCountEndif != commandCountIf)
									{
										if (commandCountEndif < commandCountIf)
											error("Script parse error, invalid if block encountered (missing endif after if).", lineNumber, true);
										else if (commandCountEndif > commandCountIf)
											error("Script parse error, invalid if block encountered (missing if before endif).", lineNumber, true);
										else
											error("Script parse error, invalid if block encountered. (missing then inside if block)", lineNumber, true);
									}
									commandCountIf = 0;
									commandCountThen = 0;
									commandCountEndif = 0;

									if (commandCountLoop != commandCountEndLoop)
									{
										error("Script parse error, invalid loop block encountered.", lineNumber, true);
									}
									commandCountLoop = 0;
									commandCountEndLoop = 0;

									if (commandCountSelect != commandCountEndSelect || selectDepth != 0)
									{
										error("Script parse error, invalid select block encountered.", lineNumber, true);
									}
									commandCountSelect = 0;
									commandCountEndSelect = 0;
									selectDepth = 0;
								}
                if (strcmp(cmd, "select") == 0)
                {
									commandCountSelect++;
									if (selectDepth > 0)
									{
										// nested selects can exist? (with _externCallPush/Pop?)
                    //error("select encountered while inside select (endSelect expected after select).", lineNumber, true);
									}
									selectDepth++;
								}
                if (strcmp(cmd, "endSelect") == 0)
                {
									if (selectDepth == 0)
									{
                    error("endSelect encountered without select (select expected before endSelect).", lineNumber, true);
									}
									commandCountEndSelect++;
									selectDepth--;
								}
							}
						} else {
							error("Command outside script block.", lineNumber, true); 
							lineok = true;
						}
					}

					if (!lineok)
					{
						error("Unknown command.", lineNumber, false);
						error(cmd, lineNumber, false);
						//assert(!"Unknown script command.");
					}
				}

				// skip leading spaces or tabs for next entry
				while (buf[i + 1] == ' ' || buf[i + 1] == '\t') 
				{ 
					i++; 
				}
				lastpos = i + 1;

				if (endCommentLine)
				{
					atCommentLine = false;
					if (insideQuotes)
					{
						//Logger::getInstance()->warning("ScriptManager::loadScripts - Line missing ending quote.");
						error("ScriptManager::loadScripts - Line missing ending quote.", lineNumber, false);
					}
					insideQuotes = false;
				}


				if (lastpos < datalen && 
					(strncmp(&buf[lastpos], "//", 2) == 0 || buf[lastpos] == '#'))
				{
					atCommentLine = true;

					// is this a preprocessor line maybe?
					if (buf[lastpos] == '#' && buf[lastpos + 1] == ' '
						&& buf[lastpos + 2] >= '0' && buf[lastpos + 2] <= '9')
					{
						// then start line numbering from here..
						lineNumber = 0;
					}
				}

			} else {
				if (buf[i] == '"')
				{
					insideQuotes = !insideQuotes;
				}
			}
		}

		if (currentScript != NULL)
		{
			error("Expected endScript, script block not properly ended before end of file.", lineNumber, true);
		}

    if (ifDepth != 0)
    {
			error("Error processing if clauses, if depth not zero at end of file.", lineNumber, true); 
      assert(!"ScriptManager::loadMemoryScripts - if depth not zero at end of file.");
    }

		/*
    if (autoNestedIfs > 0)
    {
      Logger::getInstance()->debug("Autonested if blocks, amount follows.");
      Logger::getInstance()->debug(int2str(autoNestedIfs));
    }
		*/

    if (manualNestedIfs > 0 && !loopReasonForManualNest)
    {
      // NOTICE: this is for temporary use only (maybe..?)
			error("Manually nested blocks (_externCallPush/Pop) found, they should be removed.", lineNumber, false); 
      Logger::getInstance()->debug("Manually nested blocks, amount follows.");
      Logger::getInstance()->debug(int2str(manualNestedIfs));
    }

		delete [] buf;

		return;
	}

	Script *ScriptManager::getScript(const char *scriptName)
	{
		assert(scriptName != NULL);
		
		int hc;
		SCRIPT_HASHCODE_CALC(scriptName, &hc);

		ScriptHashType::iterator iter = scriptNameHash->find(hc);
		if (iter != scriptNameHash->end())
		{
			Script *loaded = (*iter).second; 
			return loaded;
		}
		return NULL;
	}

	void ScriptManager::setKeywords(int amount, const char **keywords, int *datatypes)
	{
		// delete old arrays
		if (this->keywords != NULL)
		{
			for (int i = 0; i < keywordsAmount; i++)
			{
				assert(this->keywords[i] != NULL);
				delete [] this->keywords[i];
			}
			delete [] this->keywords;
		}
		if (keywordDatatypes != NULL)
		{
			delete [] keywordDatatypes;
		}

		// set new ones
		keywordsAmount = amount;
		this->keywords = new char *[keywordsAmount];
		this->keywordDatatypes = new int[keywordsAmount];
		for (int j = 0; j < keywordsAmount; j++)
		{
			assert(keywords[j] != NULL);
			this->keywords[j] = new char[strlen(keywords[j]) + 1];
			strcpy(this->keywords[j], keywords[j]);
			keywordDatatypes[j] = datatypes[j];
		}
	}
		
	void ScriptManager::setProcessor(IScriptProcessor *processor)
	{
		this->processor = processor;
	}

	int ScriptManager::getScriptAmount()
	{
		int ret = 0;
		if (allScripts != NULL)
		{
			LinkedListIterator iter(allScripts);
			while (iter.iterateAvailable())
			{
				iter.iterateNext();
				ret++;
			}
		}
		return ret;
	}

	int ScriptManager::reloadChangedScripts()
	{
		int ret = 0;

		std::vector<std::pair<std::string, unsigned long> > scriptFiles;

		for(ScriptFileHashType::iterator it = fileHash->begin(); it != fileHash->end(); ++it)
		{
			scriptFiles.push_back(std::pair<std::string, unsigned long>(std::string(it->second.first), it->second.second));
		}

		for (int i = 0; i < (int)scriptFiles.size(); i++)
		{
			const char *filename = scriptFiles[i].first.c_str();

			std::string dhsFilename = filename;

			if (dhsFilename.length() > 5
				&& dhsFilename.substr(dhsFilename.length() - 5, 5) == ".dhps")
			{
				if (dhsFilename.length() > 9
					&& dhsFilename.substr(dhsFilename.length() - 9, 9) == ".2da.dhps")
				{
					dhsFilename = dhsFilename.substr(0, dhsFilename.length() - 5);
				} else {
					dhsFilename = dhsFilename.substr(0, dhsFilename.length() - 5) + ".dhs";
				}

				//Logger::getInstance()->debug(dhsFilename.c_str());
				//Logger::getInstance()->debug(int2str(scriptFiles[i].second));
				//Logger::getInstance()->debug(int2str(FileTimestampChecker::getFileTimestamp(filename)));

				if ((unsigned long)FileTimestampChecker::getFileTimestamp(dhsFilename.c_str()) > scriptFiles[i].second)
				{
					loadScripts(dhsFilename.c_str(), NULL, true);
					ret++;
				}
			}
		}

		return ret;
	}

	std::string ScriptManager::getStatusInfo()
	{
		std::string ret = "ScriptManager status info:\r\n";
		ret += "List of scripts loaded:\r\n";
		int commandSum = 0;
		int scriptSum = 0;
		if (allScripts != NULL)
		{
			LinkedListIterator iter(allScripts);
			while (iter.iterateAvailable())
			{
				Script *s = (Script *)iter.iterateNext();
				ret += s->getName();
				ret += "\r\n";
				commandSum += s->commandAmount;
				scriptSum++;
			}
		}
		ret += "List of script files loaded:\r\n";
		for(ScriptFileHashType::iterator it = fileHash->begin(); it != fileHash->end(); ++it)
		{
			ret += it->second.first;
			ret += "\r\n";
		}
		ret += "Scripts amount:";
		ret += int2str(getScriptAmount());
		ret += "\r\n";
		int avgCommandCount = 0;
		if (scriptSum > 0)
			avgCommandCount = commandSum / scriptSum;
		ret += "Average command count:";
		ret += int2str(avgCommandCount);
		ret += "\r\n";

		return ret;
	}

}


