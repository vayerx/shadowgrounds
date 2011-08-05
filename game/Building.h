
#ifndef BUILDING_H
#define BUILDING_H

#include <DatatypeDef.h>

#include "GameObject.h"
#include "../ui/VisualObject.h"


namespace ui
{
  class VisualObject;
}

namespace game
{
	class GamePhysics;
	class StaticPhysicsObject;

  extern const int buildingDataId;

  /**
   * A game building.
   *
   * @version 1.1, 23.1.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see BuildingList
   *
   */

  class Building : public GameObject, public ui::IVisualObjectData
  {
  public:

    //Building(char *filename, int mapX, int mapY);
    Building(const char *filename);

    ~Building();

    /** 
     * TODO!
     * To implement GameObject "interface" class.
     * @return SaveData, data to be saved. TODO, currently NULL.
     */
    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    virtual void *getVisualObjectDataId() const;

    char *getModelFilename();

    //VC2I getMapPosition();

    void setPosition(VC3 &position);

    void addPhysics(GamePhysics *gamePhysics);
    void deletePhysics(GamePhysics *gamePhysics);

    VC3 getPosition();

    ui::VisualObject *getVisualObject();
    void setVisualObject(ui::VisualObject *visualObject);

  private:
    char *modelFilename;

    VC3 position;
    //VC2I mapPosition;

    ui::VisualObject *visualObject;

		StaticPhysicsObject *physicsObject;
  };

}

#endif
