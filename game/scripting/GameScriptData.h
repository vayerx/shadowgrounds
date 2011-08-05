
#ifndef GAMESCRIPTDATA_H
#define GAMESCRIPTDATA_H

// FIXME: not too happy about this include...
#include <string.h>

#include <DatatypeDef.h>
#include "../Unit.h"
#include "../unified_handle_type.h"

#include "../../util/Debug_MemoryManager.h"

namespace ui
{
	class Decoration;
}

namespace game
{
	class Water;
	class Part;

  class GameScriptData
  {
  public:
    GameScriptData() 
    {
      unit = NULL;
      originalUnit = NULL;
      position = VC3(0,0,0);
      secondaryPosition = VC3(0,0,0);
      alertUnit = NULL;
      spottedUnit = NULL;
      noisy = NULL;
      shooter = NULL;
      waitCounter = 0;
			waitRemainder = 0;
      waitDestination = false;
			waitCinematicScreen = false;
      player = 0;
      lastStoredPath = 0;
			storePathName = NULL;
			water = NULL;
			decoration = NULL;
			part = NULL;
			stringValue = NULL;
			floatValue = 0.0f;
			unifiedHandle = 0;
    }

		~GameScriptData()
		{
			setStorePathName(NULL);
			setStringValue(NULL);
		}

		inline void setStorePathName(const char *name)
		{
			if (storePathName != NULL)
			{
				delete [] storePathName;
				storePathName = NULL;
			}
			if (name != NULL)
			{
				storePathName = new char[strlen(name) + 1];
				strcpy(storePathName, name);
			}
		}

		inline void setStringValue(const char *sValue)
		{
			if (stringValue != NULL)
			{
				delete [] stringValue;
				stringValue = NULL;
			}
			if (sValue != NULL)
			{
				stringValue = new char[strlen(sValue) + 1];
				strcpy(stringValue, sValue);
			}
		}

    Unit *unit;
    Unit *originalUnit;
    VC3 position;
    VC3 secondaryPosition;
    Unit *alertUnit;
    Unit *spottedUnit;
    Unit *shooter;
    Unit *noisy;
		Part *part;
		Water *water;
		ui::Decoration *decoration;
    int waitCounter;
		int waitRemainder;
    bool waitDestination;
		bool waitCinematicScreen;
    int player;
    int lastStoredPath;
		char *storePathName;
		char *stringValue;
		float floatValue;
		UnifiedHandle unifiedHandle;
  };

}

#endif

