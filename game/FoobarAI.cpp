
#include "FoobarAI.h"
#include "../system/Logger.h"
#include "../container/LinkedList.h"
#include "../game/UnitActor.h"
#include "../game/unittypes.h"
#include "../game/Part.h"
#include "../game/Unit.h"
#include "../game/Game.h"
#include "../game/GameCollisionInfo.h"

using namespace game;

/*
#define FOOAI_WAYPOINTS 4
float fooAIX[FOOAI_WAYPOINTS] = { 0, -200, 300, 100 };
float fooAIY[FOOAI_WAYPOINTS] = { 0, 200, 200, -200 };
*/

::FoobarAI::FoobarAI(game::Game *game, int player)
{
  this->game = game;
  this->player = player;
}

void ::FoobarAI::doStuff()
{
  /*
  // get list of units owned by this player
  LinkedList *ulist = game->units->getOwnedUnits(player);
  LinkedListIterator iter = LinkedListIterator(ulist);
  while (iter.iterateAvailable())
  {
	  Unit *u = (Unit *)iter.iterateNext();

    if (u->hasTarget())
    {
      if (u->getTargetUnit() != NULL
        && u->getTargetUnit()->isDestroyed())
      {
        u->clearTarget();
      }
    }

    if (u->atFinalDestination())
    {
      if (u->hasTarget() && u->getTargetUnit() != NULL)
      {
        VC3 dirvect = u->getTargetUnit()->getPosition() - u->getPosition();
        float goDist = dirvect.GetLength() - 400;
        if (goDist > 0)
        {
          dirvect.Normalize();
          dirvect *= goDist;
          dirvect += u->getPosition();
          // ...now dirvect contains the new position we want to go to.

          UnitActor *ua = getUnitActorForUnit(u);
          ua->setPathTo(u, dirvect);
        }
      } else {
        int randWayPoint = (game->gameRandom->nextInt() % FOOAI_WAYPOINTS);
        float dx = fooAIX[randWayPoint];
        float dy = fooAIY[randWayPoint];
        UnitActor *ua = getUnitActorForUnit(u);
        //ua->setPathTo(u, VC3(dx, 0, dy));
      }
    }

    // if we don't have a target already, try to target some
    if (!u->hasTarget())
    {
	    // compare this unit's location to all other units...
	    // doing this is not very efficient, it's O(n^2), but who cares.
	    //LinkedList *allUnitsList = game->units->getAllUnits();
	    LinkedList *allUnitsList = game->units->getOwnedUnits(game->singlePlayerNumber);
	    LinkedListIterator allIter = LinkedListIterator(allUnitsList);
	    while (allIter.iterateAvailable())
	    {
	      Unit *other = (Unit *)allIter.iterateNext();

	      // skip the the current unit and dead units
	      if (other != u && !other->isDestroyed())
	      { 
		      //if (other->getOwner() == player)
		      if (other->getOwner() != 0)
		      {
            // other is a friendly unit
		      } else {
            // other is an enemy unit

            // unit positions and their distance...
            VC3 otherPos = other->getPosition() + VC3(0,1,0);
            VC3 ownPos = u->getPosition() + VC3(0,1,0);
            VC3 distVector = otherPos - ownPos;
            float dist = distVector.GetLength();

            // is other unit closer than 700 meters
            if (dist < 700)
            {
              // raytrace to it
              GameCollisionInfo cinfo;
              VC3 normDirection = distVector.GetNormalized();
              u->getVisualObject()->setCollidable(false);
              game->rayTrace(ownPos, normDirection, 1000, cinfo, false, false);
              u->getVisualObject()->setCollidable(true);
              // did ray hit something
              if (cinfo.hit)
              {
                // did ray hit a unit
                if (cinfo.unit != NULL)
                {
                  // was it a friendly or an enemy unit
                  //if (cinfo.unit->getOwner() != player)
                  if (cinfo.unit->getOwner() == 0)
                  {
                    // start shooting the sucker!
                    u->setTarget(cinfo.unit);
                    // and go towards him
                    VC3 dir = u->getTargetUnit()->getPosition() - u->getPosition();
                    float goDist = dir.GetLength() - 400;
                    if (goDist > 0)
                    {
                      dir.Normalize();
                      dir *= goDist;
                      dir += u->getPosition();
                      UnitActor *ua = getUnitActorForUnit(u);
                      //ua->setPathTo(u, dir);
                    }
                  } else {
                    // a friendly unit was blocking us.
                  }
                }
              }
            }
		      }
	      }
	    }
    } 
  }
  */
  // ;)
}
