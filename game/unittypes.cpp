
#include "precompiled.h"

#include <assert.h>

#include "unittypes.h"

#include "../system/Logger.h"
#include "../system/Timer.h"
#include "Unit.h"
#include "UnitType.h"
#include "UnitActor.h"
#include "ArmorUnitActor.h"
#include "ArmorUnitType.h"
#ifdef PROJECT_CLAW_PROTO
#include "ClawUnitActor.h"
#include "ClawUnitType.h"
#endif
#ifdef PROJECT_AOV
#include "SidewaysUnitActor.h"
#include "SidewaysUnitType.h"
#endif
#include "Game.h"


//#include "../editor/file_wrapper.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/ifile_list.h"
#include "../filesystem/file_package_manager.h"

// this now works as unit parser too
#include "PartTypeParser.h"

using namespace frozenbyte;

namespace game
{

  UnitActor *unitActorArray[MAX_UNIT_ACTORS] = { NULL };


  //UnitActor *getUnitActorForType(int unitType)

  UnitActor *getUnitActorForUnit(Unit *unit)
  {
    assert(unit != NULL);
    //int unitTypeId = unit->getUnitTypeId();

    //assert(unitType >= 0 && unitType < MAX_UNIT_TYPES);
    //assert(unitActorArray[unitType] != NULL);

    //return unitActorArray[unitType];

    //UnitType *ut = getUnitTypeById(unitTypeId);
    UnitType *ut = unit->getUnitType();
    if (ut == NULL)
    {
      Logger::getInstance()->error("getUnitActorForUnit - Could not solve unit type.");
      return NULL;
    }
    return ut->getActor();
  }


  void createUnitActors(Game *game)
  {
    for (int i = 0; i < MAX_UNIT_ACTORS; i++)
    {
      unitActorArray[i] = NULL;
    }
    unitActorArray[UNIT_ACTOR_ARMOR] = new ArmorUnitActor(game);
#ifdef PROJECT_CLAW_PROTO
    unitActorArray[UNIT_ACTOR_CLAW] = new ClawUnitActor(game);
#endif
#ifdef PROJECT_AOV
    unitActorArray[UNIT_ACTOR_SIDEWAYS] = new SidewaysUnitActor(game);
#endif
  }


  void deleteUnitActors()
  {
		PartTypeParser::clearLoadedList(".dhu");

    for (int i = 0; i < MAX_UNIT_ACTORS; i++)
    {
      if (unitActorArray[i] != NULL)
      {
        delete unitActorArray[i];
        unitActorArray[i] = NULL;
      }
    } 
  }


  void createUnitTypes()
  {
		// THE OLD SYSTEM, READ UNIT FILES FROM THE LIST
		// ...

		Timer::update();
		int startTime = Timer::getTime();

#ifdef LEGACY_FILES
    filesystem::FB_FILE *f = filesystem::fb_fopen("Data/Units/unitlist.txt", "rb");
#else
    filesystem::FB_FILE *f = filesystem::fb_fopen("data/unit/unitlist.txt", "rb");
#endif
    if (f == NULL)
    {
      Logger::getInstance()->error("createUnitTypes - Could not open unitlist file.");
      return;
    }
    //fseek(f, 0, SEEK_END);
    //int flen = ftell(f);
    //fseek(f, 0, SEEK_SET);
    int flen = filesystem::fb_fsize(f);

    char *buf = new char[flen + 1];
    int got = filesystem::fb_fread(buf, sizeof(char), flen, f);
    filesystem::fb_fclose(f);

    if (got != flen)
    {
      Logger::getInstance()->error("createUnitTypes - Error reading unitlist file.");
      return;
    }

    PartTypeParser ptp = PartTypeParser();

    int i;
    int lastpos = 0;
    for (i = 0; i < flen; i++)
    {
      if (buf[i] == '\n' || buf[i] == '\r')
      {
        buf[i] = '\0';
        if (buf[lastpos] != '\0')
        {
          if (strncmp(&buf[lastpos], "//", 2) != 0)
          {
            ptp.loadPartTypes(&buf[lastpos]);
          }
        }
        lastpos = i + 1;
      }
    }

    delete [] buf;

		// THE NEW SYSTEM, READ ALL UNIT FILES IN THE DIRECTORY
		// ...

		{
			/*
			frozenbyte::editor::FileWrapper fw(
				std::string("Data/Units"), std::string("*.dhu"));
			std::vector<std::string> allFiles = fw.getAllFiles();
			for (int i = 0; i < (int)allFiles.size(); i++)
			{
				const char *filename = allFiles[i].c_str();
				ptp.loadPartTypes(filename);
			}
			*/

#ifdef LEGACY_FILES
			boost::shared_ptr<filesystem::IFileList> fileList = filesystem::FilePackageManager::getInstance().findFiles("Data/Units", "*.dhu");
			std::vector<std::string> allFiles;
			filesystem::getAllFiles(*fileList, "Data/Units", allFiles, true);

			for (int i = 0; i < (int)allFiles.size(); i++)
			{
				const char *filename = allFiles[i].c_str();
				ptp.loadPartTypes(filename);
			}
#else
			// TODO: fix for subdirs too.
			boost::shared_ptr<filesystem::IFileList> fileList = filesystem::FilePackageManager::getInstance().findFiles("data/unit", "*.dhu");
			std::vector<std::string> allFiles;
			filesystem::getAllFiles(*fileList, "data/unit", allFiles, true);

			for (int i = 0; i < (int)allFiles.size(); i++)
			{
				const char *filename = allFiles[i].c_str();
				ptp.loadPartTypes(filename);
			}
#endif
		}

		Timer::update();
		int endTime = Timer::getTime();

		Logger::getInstance()->debug("createUnitTypes - Time used to parse units follows (msec):");
		Logger::getInstance()->debug(int2str(endTime - startTime));
	}


  void deleteUnitTypes()
  {
    SafeLinkedListIterator iter = SafeLinkedListIterator(&unitTypeIds);
    while (iter.iterateAvailable())
    {
      UnitType *ut = (UnitType *)iter.iterateNext();
      delete ut;
    }
  }

	UnitType *getNewUnitTypeForUnitTypeName(const char *baseclass)
	{
		if (baseclass == NULL)
		{
			Logger::getInstance()->error("getNewUnitTypeForUnitTypeName - Null unit type class name parameter given.");
			return NULL;
		}

		UnitType *parseUnit = NULL;

		if (strcmp(baseclass, "ArmorUnit") == 0) 
			parseUnit = new ArmorUnitType();
#ifdef PROJECT_CLAW_PROTO
		if (strcmp(baseclass, "ClawUnit") == 0) 
			parseUnit = new ClawUnitType();
#endif
#ifdef PROJECT_AOV
		if (strcmp(baseclass, "SidewaysUnit") == 0) 
			parseUnit = new SidewaysUnitType();
#endif

		if (parseUnit == NULL)
		{
			Logger::getInstance()->error("getNewUnitTypeForUnitTypeName - Unknown unit type class name.");
			Logger::getInstance()->debug(baseclass);
		}

		return parseUnit;
	}

}
