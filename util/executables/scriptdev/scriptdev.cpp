// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "../../../game/scripting/gamescripting_amount.h"
#include "../../ScriptManager.h"
#include "../../Script.h"
#include "../../../game/GameOptionManager.h"
#include "../../../system/Logger.h"
#include "../../SimpleParser.h"
#include "../../../convert/str2int.h"
#include <assert.h>
#include <algorithm>
#include <direct.h>
#include "../../Debug_MemoryManager.h"

#include "../../../filesystem/file_package_manager.h"
#include "../../../filesystem/standard_package.h"
#include "../../../filesystem/zip_package.h"

// just some id number to be used at the end of datatype array.
#define GS_DATATYPE_ARR_END 1234

// TODO: move to some configuration file?
#define PROJECT_SURVIVOR 1

// ---------

#include "../../../game/scripting/scriptcommands.h"

// ---------

const char *gs_keywords[GS_CMD_AMOUNT + 1];
int gs_datatypes[GS_CMD_AMOUNT + 1];

struct gs_commands_listtype
{
	int id;
	char *name;
	int datatype;
};

#define GS_EXPAND_GAMESCRIPTING_LIST

#include "../../../game/scripting/scriptcommands.h"

#undef GS_EXPAND_GAMESCRIPTING_LIST

// ---------

/*
struct GSCommandSorter: 
public std::binary_function<gs_commands_listtype, gs_commands_listtype, bool>
{
	bool operator() (const gs_commands_listtype &a, const gs_commands_listtype &b) const
	{
		return a.id < b.id;
	}
};
*/

using namespace frozenbyte;

int main(int argc, char *argv[])
{
	char snapshotPath[1024] = { 0 };
	int lastSlash = 0;
	for (int snc = strlen(argv[0]); snc >= 0; snc--)
	{
		if (argv[0][snc] == '\\' || argv[0][snc] == '/')
		{
			lastSlash = snc;
			break;
		}
	}
	
	char *toolsdir = strstr(argv[0], "\\tools");
	if (toolsdir != NULL)
	{
		lastSlash = toolsdir - argv[0];
	}

	for (int sn = 0; sn < lastSlash; sn++)
	{
		snapshotPath[sn] = argv[0][sn];
	}
	snapshotPath[lastSlash] = '\0';

	if (snapshotPath[0] != '\0')
	{
//#ifndef _DEBUG
		// WARNING: this will changedir to scriptdev.exe's directory 
		// BUT, only on release build... not on debug build
		if (_chdir(snapshotPath) != 0)
		{
			printf("Failed to change to snapshot path \"%s\".\r\n", snapshotPath);
		}
//#endif
	}

	char dhmfile[1024];
	char dhsfile[1024];

	dhmfile[0] = '\0';
	dhsfile[0] = '\0';

	bool printToStdout = false;
	bool buildFirstMission = false;

	{
		using namespace frozenbyte::filesystem;
		boost::shared_ptr<IFilePackage> standardPackage(new StandardPackage());
		FilePackageManager &manager = FilePackageManager::getInstance();
		manager.addPackage(standardPackage, 999);

		boost::shared_ptr<IFilePackage> zipPackage1(new ZipPackage("../../data1.fbz"));
		boost::shared_ptr<IFilePackage> zipPackage2(new ZipPackage("../../data2.fbz"));
		boost::shared_ptr<IFilePackage> zipPackage3(new ZipPackage("../../data3.fbz"));
		boost::shared_ptr<IFilePackage> zipPackage4(new ZipPackage("../../data4.fbz"));
		boost::shared_ptr<IFilePackage> zipPackageMod1(new ZipPackage("data1.fbz"));
		boost::shared_ptr<IFilePackage> zipPackageMod2(new ZipPackage("data2.fbz"));
		boost::shared_ptr<IFilePackage> zipPackageMod3(new ZipPackage("data3.fbz"));
		boost::shared_ptr<IFilePackage> zipPackageMod4(new ZipPackage("data4.fbz"));

		manager.addPackage(zipPackage1, 1);
		manager.addPackage(zipPackage2, 2);
		manager.addPackage(zipPackage3, 3);
		manager.addPackage(zipPackage4, 4);
		manager.addPackage(zipPackageMod1, 5);
		manager.addPackage(zipPackageMod2, 6);
		manager.addPackage(zipPackageMod3, 7);
		manager.addPackage(zipPackageMod4, 8);
	}

	util::SimpleParser msp;
#ifdef LEGACY_FILES
	msp.loadFile("Data/Missions/missiondefs.txt");
#else
	msp.loadFile("data/mission/missiondefs.txt");
#endif
	while (msp.next())
	{
		char *key = msp.getKey();
		if (strcmp(key, "firstmission") == 0)
		{
			char *value = msp.getValue();
			if (strlen(value) < 256)
			{
				strcpy(dhmfile, value);
			}
		}
	}


	if (argc >= 2)
	{

		int fileArg = 1;

		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-stdout") == 0)
			{
				printToStdout = true;
				fileArg = i + 1;
			}
			if (strcmp(argv[i], "-firstmission") == 0)
			{
				buildFirstMission = true;
				fileArg = i + 1;
			}
		}

		if (fileArg < argc)
		{
			if (strlen(argv[fileArg]) < 256)
			{
				if (strcmp(&(argv[fileArg][strlen(argv[fileArg]) - 4]), ".dhm") == 0)
				{
#ifdef LEGACY_FILES
					char *datapath = strstr(argv[fileArg], "Data\\");
#else
					char *datapath = strstr(argv[fileArg], "data\\");
#endif

#ifdef PROJECT_SURVIVOR
					// survivor: survival mission support
					if(datapath == NULL)
					{
						datapath = strstr(argv[fileArg], "Survival\\");
					}
#endif

					if (datapath != NULL)
					{
						strcpy(dhmfile, datapath);
						for (int i = 0; i < (int)strlen(dhmfile); i++)
						{
							if (dhmfile[i] == '\\')
								dhmfile[i] = '/';
						}
					}
				}
				// ye olde code...
				/*
				if (strcmp(&(argv[fileArg][strlen(argv[fileArg]) - 4]), ".dhs") == 0)
				{
					char *datapath = strstr(argv[fileArg], "Data\\");
					if (datapath != NULL)
					{
						strcpy(dhsfile, datapath);
						for (int i = 0; i < (int)strlen(dhsfile); i++)
						{
							if (dhsfile[i] == '\\')
								dhsfile[i] = '/';
						}
					}
				}
				*/
				if (strcmp(&(argv[fileArg][strlen(argv[fileArg]) - 4]), ".dhs") == 0
					|| strcmp(&(argv[fileArg][strlen(argv[fileArg]) - 4]), ".2da") == 0)
				{
#ifdef LEGACY_FILES
					const char *mission_path = "Data\\Missions\\";
#else
					const char *mission_path = "data\\mission\\";
#endif
					char *mission_p_pos = strstr(argv[fileArg], (const char*) mission_path);

#ifdef PROJECT_SURVIVOR
					// survivor: survival mission support
					if(mission_p_pos == NULL)
					{
						mission_path = "Survival";
						mission_p_pos = strstr(argv[fileArg], "Survival");
					}
#endif

#ifdef LEGACY_FILES
					char common_mission_path[] = "Data\\Missions\\Common";
#else
					char common_mission_path[] = "data\\mission\\common";
#endif
					char *common_mission_p_pos = strstr(argv[fileArg], (const char*) mission_path);
					if(mission_p_pos != NULL && common_mission_p_pos == NULL)
					{
						char *mission_begin_pos = mission_p_pos + strlen((const char*) mission_path);
						char *mission_end_pos = strstr(mission_begin_pos, "\\");
						int mission_name_len = mission_end_pos - mission_begin_pos;
						int mission_name_start = mission_begin_pos - argv[fileArg];

						std::string mission_name = argv[fileArg];
						mission_name = "" + mission_name.substr(mission_name_start, mission_name_len);
						mission_name += ".dhm";
						//printf("Mission file: %s\n", mission_name.c_str());
						strcpy(dhmfile, mission_path);
						strcat(dhmfile, mission_name.substr(0,mission_name.length()-4).c_str());
						strcat(dhmfile, "\\");
						strcat(dhmfile, mission_name.c_str());
					}

#ifdef LEGACY_FILES
					char *datapath = strstr(argv[fileArg], "Data\\");
#else
					char *datapath = strstr(argv[fileArg], "data\\");
#endif

#ifdef PROJECT_SURVIVOR
					// survivor: survival mission support
					if(datapath == NULL)
					{
						datapath = strstr(argv[fileArg], "Survival");
					}
#endif
					if (datapath != NULL)
					{
						strcpy(dhsfile, datapath);
						for (int i = 0; i < (int)strlen(dhsfile); i++)
						{
							if (dhsfile[i] == '\\')
								dhsfile[i] = '/';
						}
					}
				}

			}
		}

	}


#ifdef LEGACY_FILES
	Logger::createInstanceForLogfile("scriptdev_err.txt");
#else
	Logger::createInstanceForLogfile("logs/scriptdev_err.txt");
#endif

	//std::sort(&gs_commands[0], &gs_commands[GS_CMD_AMOUNT + 1], GSCommandSorter());

#ifdef _DEBUG
	//if (gs_commands[GS_CMD_AMOUNT].id != GS_MAX_COMMANDS)
	//{
	//	assert(!"GameScripting - Command name array not ok (after sort). Fix it.");
	//}

	//{
	//	for (int i = 0; i < GS_CMD_AMOUNT; i++)
	//	{
	//		void *foo = gs_commands;
	//		if (gs_commands[i].id != i)
	//		{
	//			assert(!"GameScripting - Command name array not ok, (after sort). Fix it.");
	//		}
	//	}
	//}
#endif

	for (int i = 0; i < GS_CMD_AMOUNT; i++)
	{
		gs_keywords[i] = "__reserved_no_command";
		gs_datatypes[i] = SCRIPT_DATATYPE_NONE;
	}
	for (int i = 0; i < GS_CMD_AMOUNT; i++)
	{
		if (gs_commands[i].id == GS_MAX_COMMANDS)
		{
			break;
		}
		if (gs_commands[i].id < GS_CMD_AMOUNT)
		{
			if (strcmp(gs_keywords[gs_commands[i].id], "__reserved_no_command") == 0)
			{
				gs_keywords[gs_commands[i].id] = gs_commands[i].name;
				gs_datatypes[gs_commands[i].id] = gs_commands[i].datatype;
			} else {
				assert(!"scriptdev - Duplicate command id. Fix it.");
			}
		} else {
			assert(!"scriptdev - Command id over maximum amount. Fix it.");
		}
	}

#ifdef _DEBUG
	//if (gs_commands[GS_CMD_AMOUNT].name != NULL)
	//{
	//	if (strcmp(gs_commands[GS_CMD_AMOUNT].name, "***") != 0)
	//	{
	//		assert(!"GameScripting - Command name array not ok. Fix it.");
	//	}
	//} else {
	//	assert(!"GameScripting - Command name array not ok. Fix it.");
	//}
	//if (gs_commands[GS_CMD_AMOUNT].datatype != GS_DATATYPE_ARR_END)
	//{
	//	assert(!"GameScripting - Command datatype array not ok. Fix it.");
	//}
#endif

	game::GameOptionManager::getInstance()->load();

	//util::ScriptManager::getInstance()->setProcessor(this);
	util::ScriptManager::getInstance()->setKeywords(GS_CMD_AMOUNT, 
		gs_keywords, gs_datatypes);

	util::SimpleParser sp;
	sp.loadFile(dhmfile);
	while (sp.next())
	{
		char *l = sp.getLine();
		if (strncmp(l, "includeScript ", 14) == 0)
		{
			char *loads = &l[14];
			if (loads[0] == '"')
			{
				loads = &loads[1];
				// WARNING: modifies simpleparser's buffer!!!
				for (int i = 0; i < (int)strlen(loads); i++)
				{
					if (loads[i] == '"')
					{
						loads[i] = '\0';
						break;
					}
				}
			}
			util::ScriptManager::getInstance()->loadScripts(loads, dhmfile);
		}
	}

	if (!buildFirstMission)
	{
		Logger::cleanInstance();
#ifdef LEGACY_FILES
		Logger::createInstanceForLogfile("scriptdev_err.txt");
#else
		Logger::createInstanceForLogfile("logs/scriptdev_err.txt");
#endif
		Logger::getInstance()->setLogLevel(LOGGER_LEVEL_INFO);
	}

	if (dhsfile[0] != '\0')
	{
		//game::GameOptionManager::getInstance()->getOptionByName("force_script_preprocess")->setBooleanValue(true);

		Logger::getInstance()->info("Preprocess/compilation log for following file:");
		Logger::getInstance()->info(dhsfile);

		util::ScriptManager::getInstance()->setForcePreprocessForNextLoad(true);
		util::ScriptManager::getInstance()->loadScripts(dhsfile, dhsfile, true);
	} else {
		Logger::getInstance()->info("No file parameter given for preprocess/compilation");
	}

	if (printToStdout)
	{
		bool foundErrors = false;
#ifdef LEGACY_FILES
		FILE *f = fopen("scriptdev_err.txt", "rb");
#else
		FILE *f = fopen("logs/scriptdev_err.txt", "rb");
#endif
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			int flen = ftell(f);
			fseek(f, 0, SEEK_SET);

			char *buf = new char[flen + 1];

			fread(buf, flen, 1, f);
			buf[flen] = '\0';

			fclose(f);

			bool stillInError = false;

			for (int i = 0; i < flen; i++)
			{
				if (buf[i] == '\r')
				{
					buf[i] = ' ';
				}
				if (buf[i] == '\n' || buf[i] == '\0')
				{
					buf[i] = '\0';
					int skipErr = 0;
					if (strncmp(&buf[i + 1], "INFO: ", 6) == 0
						|| strncmp(&buf[i + 1], "DEBUG: ", 7) == 0)
					{
						stillInError = false;
					}
					if (strncmp(&buf[i + 1], "ERROR: ", 7) == 0
						|| strncmp(&buf[i + 1], "WARNING: ", 9) == 0
						|| stillInError)
					{
						if (strncmp(&buf[i + 1], "ERROR: ", 7) == 0) 
							skipErr = 7;
						if (strncmp(&buf[i + 1], "WARNING: ", 9) == 0) 
							skipErr = 9;
						stillInError = true;

						bool hadFileOrLine = false;
						foundErrors = true;
						for (int j = i+1; j < flen+1; j++)
						{
							if (strncmp(&buf[j], "(file ", 6) == 0)
							{
								//buf[j - 1] = '\0';
								buf[j] = '\0';
								for (int k = j; k < flen; k++)
								{
									if (buf[k] == ',' || buf[k] == ')')
									{
										buf[k] = '\0';
										char *filename = &buf[j + 6];
										char *dhpsext = strstr(filename, ".dhps");
										if (dhpsext != NULL)
										{
											dhpsext[3] = 's';
											dhpsext[4] = '\0';
											printf("%s:", filename);
											dhpsext[3] = 'p';
											dhpsext[4] = 's';
										} else {
											printf("%s:", filename);
										}
										j = k;
										hadFileOrLine = true;
										break;
									}
								}
							}
							if (strncmp(&buf[j], "line ", 5) == 0)
							{
								for (int k = j; k < flen; k++)
								{
									if (buf[k] == ',' || buf[k] == ')')
									{
										buf[k] = '\0';
										if (str2int(&buf[j + 5]) == 0)
										{
											printf("1: ", &buf[j + 5]);
										} else {
											printf("%s: ", &buf[j + 5]);
										}
										j = k;
										hadFileOrLine = true;
										break;
									}
								}
							}
							if (buf[j] == '\r')
							{
								buf[j] = ' ';
							}
							if (buf[j] == '\n' || j == flen)
							{
								buf[j] = '\0';
								//if (hadFileOrLine)
								//{
								//	printf("%s\n", &buf[i+1+skipErr]);
								//} else {
								//}
								printf("%s\n", &buf[i+1+skipErr]);
								if (i < j-1)
									i = j-1;
								break;
							}
						}
					}
				}
			}
		}

		if (foundErrors)
		{
			printf("*** Error(s) ***\n");
			return 1;
		}
	}

	return 0;
}

