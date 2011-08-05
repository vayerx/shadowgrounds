
#include "precompiled.h"

#include "gamedefs.h"
#include "BuildingList.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  BuildingList::BuildingList()
  {
    allBuildings = new LinkedList();
  }

  // NOTE, does not delete the buildings inside this but just the list of them
  BuildingList::~BuildingList()
  {
    while (!allBuildings->isEmpty())
    {
      allBuildings->popLast();
    }
    delete allBuildings;
  }

  SaveData *BuildingList::getSaveData() const
  {
    // TODO
    return NULL;
  }

  const char *BuildingList::getStatusInfo() const
  {
    return "BuildingList";
  }

  int BuildingList::getAllBuildingAmount()
  {
    // TODO, optimize, not thread safe! (caller may not be using iterator)
    int count = 0;
    allBuildings->resetIterate();
    while (allBuildings->iterateAvailable())
    {
      allBuildings->iterateNext();
      count++;
    }
    return count;
  }

  LinkedList *BuildingList::getAllBuildings()
  {
    return allBuildings;
  }

  void BuildingList::addBuilding(Building *building)
  {
    allBuildings->append(building);
  }

  // does not delete the building, just removes it from the list
  void BuildingList::removeBuilding(Building *building)
  {
    allBuildings->remove(building);
  }

}

