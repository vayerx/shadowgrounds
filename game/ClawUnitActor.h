
#ifndef CLAWUNITACTOR_H
#define CLAWUNITACTOR_H

#include "UnitActor.h"


namespace game
{
  class Game;
  class Unit;


  class ClawUnitActor : public UnitActor
  {
  public:
    ClawUnitActor(Game *game);
    
    //virtual ~ClawUnitActor() { };

		void actDirectClawControls(Unit *unit);

    virtual void act(Unit *unit);

    virtual bool setPathTo(Unit *unit, VC3 &destination);

    virtual frozenbyte::ai::Path *solvePath(Unit *unit, const VC3 &startPosition, VC3 &endPosition, int maxDepth = 100);

    virtual void stopUnit(Unit *unit);

    virtual void addUnitObstacle(Unit *unit);

    virtual void removeUnitObstacle(Unit *unit);

    virtual void moveUnitObstacle(Unit *unit, int x, int y);
  };

}

#endif

