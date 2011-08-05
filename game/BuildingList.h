
#ifndef BUILDINGLIST_H
#define BUILDINGLIST_H

#include "../container/LinkedList.h"
#include "GameObject.h"
#include "Building.h"


namespace game
{
  /**
   * A class holding game buildings.
   * 
   * @version 1.0, 4.7.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see Building
   * @see Game
   */

  class BuildingList : public GameObject
  {
  public:
    BuildingList();
    ~BuildingList();

    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    int getAllBuildingAmount();

    LinkedList *getAllBuildings();

    void addBuilding(Building *building);
    void removeBuilding(Building *building);

  private:
    LinkedList *allBuildings;
  };

}

#endif

