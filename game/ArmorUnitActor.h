
#ifndef ARMORUNITACTOR_H
#define ARMORUNITACTOR_H

#include "UnitActor.h"


namespace game
{
  class Game;
  class Unit;


  class ArmorUnitActor : public UnitActor
  {
  public:
    ArmorUnitActor(Game *game);
    
    //virtual ~ArmorUnitActor() { };

    virtual void act(Unit *unit);

    virtual bool setPathTo(Unit *unit, const VC3 &destination);

    virtual frozenbyte::ai::Path *solvePath(Unit *unit, const VC3 &startPosition, VC3 &endPosition, int maxDepth = 100);

    virtual void stopUnit(Unit *unit);

    virtual void addUnitObstacle(Unit *unit);

    virtual void removeUnitObstacle(Unit *unit);

    virtual void moveUnitObstacle(Unit *unit, int x, int y);

    virtual void actSideGravity(Unit *unit);
  };

}

#endif

