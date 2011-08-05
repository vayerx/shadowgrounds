
#include "precompiled.h"

#include "UnitVisibilityChecker.h"

#include <c2_common.h>
#include <c2_vectors.h>

#include "../system/Logger.h"
#include "../container/LinkedList.h"
#include "../convert/str2int.h"
#include "Part.h"
#include "Unit.h"
#include "UnitList.h"
#include "UnitType.h"
#include "Game.h"
#include "GameRandom.h"
#include "SimpleOptions.h"
#include "options/options_cheats.h"
#include "options/options_game.h"
#include "GameUI.h"
#include "GameMap.h"
//#include "HiddenessSolver.h"
#include "GameScene.h"
#include "GameCollisionInfo.h"
#include "scaledefs.h"

#include "../util/Debug_MemoryManager.h"


// first person sees to this distance max.
#define FIRST_PERSON_VISION_RANGE 500


namespace game
{

static LinkedList *temp_player_and_allies_list = NULL;
static int temp_player_and_allies_list_amount = 0;
static LinkedList *temp_empty_list = NULL;


UnitVisibilityChecker::UnitVisibilityChecker(game::Game *game)
{
  this->game = game;
  unitIterator = new LinkedListIterator(game->units->getAllUnits());
  otherUnitIterator = NULL;
  totalUnits = 0;
  unitsInPass = 0;
  passCount = 0;
  currentUnit = NULL;
	updateDisabled = false;
}

UnitVisibilityChecker::~UnitVisibilityChecker()
{
  if (otherUnitIterator != NULL)
    delete otherUnitIterator;
  delete unitIterator;

	if (temp_player_and_allies_list != NULL)
	{
		delete temp_player_and_allies_list;
		temp_player_and_allies_list = NULL;
	}
	if (temp_empty_list != NULL)
	{
		delete temp_empty_list;
		temp_empty_list = NULL;
	}
}

void UnitVisibilityChecker::restart()
{
  LinkedList *allUnitsList = game->units->getAllUnits();

  // nobody sees nobody... except own units
  LinkedListIterator allIter = LinkedListIterator(allUnitsList);
  while (allIter.iterateAvailable())
	{
	  Unit *unit = (Unit *)allIter.iterateNext();
    for (int i = 0; i < ABS_MAX_PLAYERS; i++)
    {
      if (unit->getOwner() == i)
      {
        unit->visibility.setToBeSeenByPlayer(i, true);
        unit->visibility.setToBeInRadarByPlayer(i, true);
      } else {
        unit->visibility.setToBeSeenByPlayer(i, false);
        unit->visibility.setToBeInRadarByPlayer(i, false);
      }
    }
    unit->setToBeSeenUnit(NULL);

    unit->visibility.useToBeSeenByPlayer();
    unit->visibility.useToBeInRadarByPlayer();
    unit->useToBeSeenUnit();
  }

  delete unitIterator;
  unitIterator = new LinkedListIterator(game->units->getAllUnits());
  if (otherUnitIterator != NULL)
    delete otherUnitIterator;
  otherUnitIterator = new LinkedListIterator(game->units->getAllUnits());
  totalUnits = game->units->getAllUnitAmount();
  unitsInPass = totalUnits / VISIBILITY_CHECK_IN_PASSES;
  passCount = VISIBILITY_CHECK_IN_PASSES;
  currentUnit = NULL;

	temp_player_and_allies_list_amount = -1;
}

void UnitVisibilityChecker::runCheck()
{
	// NOTICE: amount of checks done per seconds depends
	// on rate of game ticks. 

	//int chkA = 0;

	for (int i = 0; i < totalUnits / 3; i++)
	{
		//chkA++;
		bool raytraced = this->runCheckImpl();
		if (raytraced) break;
	}
}

bool UnitVisibilityChecker::runCheckImpl()
{
	if (updateDisabled)
		return true; // update is disabled, so fake as if we just raytraced

	bool didRaytrace = false;

	// no units at all, or all inactive/destroyed may cause null unit... 
	// just skip the checks in that case
	if (currentUnit == NULL)
	{
		passCount = VISIBILITY_CHECK_IN_PASSES;
	}

  // go thru all needed passes to create visibility for one unit
  if (passCount < VISIBILITY_CHECK_IN_PASSES)
  {
    Unit *u = currentUnit;

    int player = u->getOwner();

    int unitCount = 0;
    while (otherUnitIterator->iterateAvailable())
	  {
	    Unit *other = (Unit *)otherUnitIterator->iterateNext();

      // skip the the current unit, friendly units 
			// and already seen dead units
      if (other->isSpottable()
				&& other != u && game->isHostile(player, other->getOwner())
        && other->isActive()
				&& (!other->isDestroyed()
				|| !other->visibility.isSeenByPlayer(player)))
	    { 
        VC3 otherPos = other->getPosition();
				if (other->getSpeed() == Unit::UNIT_SPEED_CRAWL)
					otherPos += VC3(0,1.0f,0);
				else
					otherPos += VC3(0,other->getUnitType()->getEyeHeight(),0);
        VC3 ownPos = u->getPosition();
				if (u->getSpeed() == Unit::UNIT_SPEED_CRAWL)
					ownPos += VC3(0,1.0f,0);
				else
					ownPos += VC3(0,u->getUnitType()->getEyeHeight(),0);
        VC3 distVector = otherPos - ownPos;

        float maxSeeDist = u->getUnitType()->getVisionRange() 
          * other->getUnitType()->getVisibilityRatio();

				// possible sight bonus (+30%)
				// (usually soldiers that have spotted player)
				// except against stealth
				if (u->hasSightBonus() && !other->isStealthing())
					maxSeeDist *= (1.0f + u->getUnitType()->getSightBonusAmount());

        // being on mountain top gives max 150% visibility and radar.
				// ...not anymore
        //float heightFactor = (1.0f + 0.5f * ownPos.y / game->gameMap->getScaledMaxHeight());
        //maxSeeDist *= heightFactor;

				// hiddeness factor... (drops the visibility to 50% at minimum)
//				float hidRes = (1.0f - HiddenessSolver::solveHiddenessFactorBetween(game->gameMap, ownPos, otherPos));
				float hidRes = 1.0f;
				float hiddenessFactor;

				// HACK: player 0 (human) gets full hiddeness, hostiles get a bit less
				if (player != game->singlePlayerNumber && game->isHostile(0, player))
				{
					hiddenessFactor = 0.4f + 0.6f * hidRes;
					// HACK: player 0 (human) gets stand still/sneak/stealth bonus...
					// however, not if you have a target (that means 
					// you are probably shooting). (should check for actual shots though)
					// TODO: stealthing!
					VC3 vel = other->getVelocity();
					if ((other->getSpeed() == Unit::UNIT_SPEED_SLOW
						|| (vel.x == 0 && vel.z == 0))
						&& !other->targeting.hasTarget())
					{
						// standing still at totally clear areas won't help so much
						// but if in bushes or something, it does help.
						if (hidRes > 0.6f)
							hiddenessFactor -= 0.1f;
						else
							hiddenessFactor -= 0.2f;
					}
				} else {
					hiddenessFactor = 0.6f + 0.4f * hidRes;
					// others get crawling bonus
					if (other->getSpeed() == Unit::UNIT_SPEED_CRAWL)
						hiddenessFactor -= 0.2f;				
				}	

				// stealthing effect. (see distance to one third)
				if (other->isStealthing())
				{
					hiddenessFactor *= other->getUnitType()->getStealthRatio();
				}

				maxSeeDist *= hiddenessFactor;

        GameCollisionInfo cinfo;
        float dist = distVector.GetLength();

        bool setSeen = false;
        bool setRadar = false;

        float radarRange = u->getUnitType()->getRadarRange()
          * other->getUnitType()->getRadarRatio();

        //radarRange *= heightFactor;

        if (dist < radarRange)
        {
          // was inside radar range...
          if (dist < radarRange / 10.0f)
          {
						// was very close
            setRadar = true;
          } else {
            VC3 otherVel = other->getVelocity();
            float bonus = 0;
            if (otherVel.x != 0 || otherVel.z != 0 || otherVel.y != 0)
            {
              bonus += 1.0f;
            } else {
							if (other->getMoveState() != Unit::UNIT_MOVE_STATE_IDLE)
								bonus -= 0.90f;
//								&& (game->gameRandom->nextInt() & 31) != 0)
						}
            //if (other->getMaxHeat() != 0)
            //{
            //  bonus += other->getHeat() / other->getMaxHeat();
            //}
						// actual multiplier is between 0.25f - 1.0f
            if (dist < (radarRange * (1.0f + bonus)) / 2.0f)
              setRadar = true;
          }
        }

        if ((dist < u->getUnitType()->getMinVisionRange() && other->isSpottable())
					|| (other->getUnitType()->isFlying() && dist < maxSeeDist))
        {
          // was inside minimum vision range, always see him
          setSeen = true;
					// NOTE: new behaviour, seen units no-longer
					// necessarily are in radar
          //setRadar = true;
        }

        if (!setSeen && dist < maxSeeDist)
        {
          // was inside vision range... check if line of sight and stuff...
          int fovAngle = u->getUnitType()->getVisionFOV() / 2;
          int fovRotate = 0;
          if (fovAngle < 180)
          { 
					  // sight bonus?
						if (u->hasSightBonus())
						{
							fovAngle += 30; // 30 degrees (x2 = 60)
							if (fovAngle > 180)
								fovAngle = 180;
						}

            // was not 360 vision fov, check it...
            VC2 destFloat = VC2(
              (float)(otherPos.x-ownPos.x), (float)(otherPos.z-ownPos.z));
            float destAngleFloat = destFloat.CalculateAngle();
            float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
            while (destAngle >= 360) destAngle -= 360;

            VC3 rotation = u->getRotation();
            if ((destAngle > rotation.y+fovAngle && destAngle <= rotation.y + 180)
              || (destAngle <= rotation.y - 180 && destAngle > rotation.y - (360-fovAngle)))
              fovRotate += 1;
            if ((destAngle < rotation.y-fovAngle && destAngle >= rotation.y - 180)
              || (destAngle >= rotation.y + 180 && destAngle < rotation.y + (360-fovAngle)))
              fovRotate -= 1;
          }

          if (fovRotate == 0)
          {
            // was within fov, continue to raytrace

						// TEMP!
						// ignore all small units (1.5m) of that target player...
						// except the one we're trying to hit
						LinkedList *oul = game->units->getOwnedUnits(other->getOwner());
						LinkedListIterator iter = LinkedListIterator(oul);
						while (iter.iterateAvailable())
						{
							Unit *ou = (Unit *)iter.iterateNext();
							if (ou != other && ou->getVisualObject() != NULL
								&& ou->isActive() && ou->getUnitType()->getSize() <= 1.5f)
								ou->getVisualObject()->setCollidable(false);
						}
						// and those friendly units too
						if (other->getOwner() != u->getOwner())
						{
							oul = game->units->getOwnedUnits(u->getOwner());
							iter = LinkedListIterator(oul);
							while (iter.iterateAvailable())
							{
								Unit *ou = (Unit *)iter.iterateNext();
								if (ou != u && ou->isActive() 
									&& ou->getUnitType()->getSize() <= 1.5f)
									ou->getVisualObject()->setCollidable(false);
							}
						}

            VC3 normDirection = distVector.GetNormalized();
						if (u->getVisualObject() != NULL)
							u->getVisualObject()->setCollidable(false);
//            game->getGameScene()->rayTrace(ownPos, normDirection, dist, cinfo, false, true);
#ifdef PROJECT_CLAW_PROTO
            game->getGameScene()->rayTrace(ownPos, normDirection, dist, cinfo, true, true, true, true);
#else
            game->getGameScene()->rayTrace(ownPos, normDirection, dist, cinfo, true, true);
#endif
						if (u->getVisualObject() != NULL)
	            u->getVisualObject()->setCollidable(true);
						didRaytrace = true;

            // did ray hit the other unit
            // or nothing (thus no obstacles between units, can be seen)
            if ((cinfo.hit && cinfo.hitUnit && cinfo.unit == other)
              || !cinfo.hit)
            {
              setSeen = true;
							// NOTE: new behaviour, seen units no-longer
							// necessarily are in radar
              //setRadar = true;
            }

						if (!setSeen)
						{
							// HACK: player 0 units predict enemy movements.
							// seeing them sooner coming behind corners.
							if (player == game->singlePlayerNumber)
							{
								// 2 seconds prediction.
								VC3 vel = other->getVelocity();
								// max 3 m/s.
								if (vel.GetSquareLength() > 3.0f * 3.0f) 
									vel = (vel * 3.0f) / vel.GetLength();
								VC3 otherPosPredict = otherPos;
								otherPosPredict += vel * GAME_TICKS_PER_SECOND * 2;
				        VC3 distVectorPredict = otherPosPredict - ownPos;

								GameCollisionInfo cinfoPredict;
								float distPredict = distVector.GetLength();

								VC3 normDirectionPredict = distVectorPredict.GetNormalized();
								u->getVisualObject()->setCollidable(false);
//								game->getGameScene()->rayTrace(ownPos, normDirectionPredict, distPredict, cinfoPredict, false, true);
#ifdef PROJECT_CLAW_PROTO
								game->getGameScene()->rayTrace(ownPos, normDirectionPredict, distPredict, cinfoPredict, true, true, true, true);
#else
								game->getGameScene()->rayTrace(ownPos, normDirectionPredict, distPredict, cinfoPredict, true, true);
#endif
								u->getVisualObject()->setCollidable(true);
								didRaytrace = true;

								if ((cinfoPredict.hit && cinfoPredict.hitUnit && cinfoPredict.unit == other)
									|| !cinfoPredict.hit)
								{
									setSeen = true;
									// NOTE: new behaviour, seen units no-longer
									// necessarily are in radar
									//setRadar = true;
								}
							}
						}

						// TEMP!
						// restore them all...
						oul = game->units->getOwnedUnits(other->getOwner());
						iter = LinkedListIterator(oul);
						while (iter.iterateAvailable())
						{
							Unit *ou = (Unit *)iter.iterateNext();
							if (ou != other 
								&& ou->isActive()
								&& ou->getUnitType()->getSize() <= 1.5f)
								ou->getVisualObject()->setCollidable(true);
						}
						// and those friendly units too
						if (other->getOwner() != u->getOwner())
						{
							oul = game->units->getOwnedUnits(u->getOwner());
							iter = LinkedListIterator(oul);
							while (iter.iterateAvailable())
							{
								Unit *ou = (Unit *)iter.iterateNext();
								if (ou != u 
									&& ou->isActive()
									&& ou->getUnitType()->getSize() <= 1.5f)
									ou->getVisualObject()->setCollidable(true);
							}
						}
          }

        }

        if (setSeen)
        {
					unitSeesUnit(u, other, dist);

					// and vice versa if very near / in building...
					// HACK: for enemy -> player 0 only...
					if (other->getOwner() == 0)
					{
						if (game->gameMap->isInScaledBoundaries(ownPos.x, ownPos.z)
							&& game->gameMap->isInScaledBoundaries(otherPos.x, otherPos.z))
						{
							int px = game->gameMap->scaledToPathfindX(ownPos.x);
							int py = game->gameMap->scaledToPathfindY(ownPos.z);
							if (dist < 10 
								|| game->getGameScene()->getBuildingModelAtPathfind(px, py) != NULL)
							{
								unitSeesUnit(other, u, dist);
							}
						}
					}
        }

        if (setRadar)
        {
          // set the other unit to be seen by this unit's player
          other->visibility.setToBeInRadarByPlayer(player, true);
        }
	    }

      // break when we've checked enough units for one pass.
      // except for last pass, in which all remaining units are checked.
      unitCount++;
      if (unitCount >= unitsInPass 
        && passCount != VISIBILITY_CHECK_IN_PASSES - 1) break;
    }

    passCount++;

  } else {
    // when done, next unit or reset to first unit...
    
    if (!unitIterator->iterateAvailable())
    {
			// no more units, thus must have passed thru all units, now need 
			// to take the new visibilities into use...

      // first person visibility...
      VC3 fpPosition = VC3(0,0,0);
      Unit *firstPerson = game->gameUI->getFirstPerson(game->singlePlayerNumber);
      if (firstPerson != NULL)
      {
        fpPosition = firstPerson->getPosition();
      }

      // take new visibilities into use
      LinkedList *allUnitsList = game->units->getAllUnits();
      LinkedListIterator allIter = LinkedListIterator(allUnitsList);
      while (allIter.iterateAvailable())
	    {
	      Unit *unit = (Unit *)allIter.iterateNext();
				bool wasSeenByPlayer = unit->visibility.isSeenByPlayer(game->singlePlayerNumber);
        unit->visibility.useToBeSeenByPlayer();
        unit->useToBeSeenUnit();
        unit->visibility.useToBeInRadarByPlayer();

				bool fpCheck = false;
				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
				{
					if (firstPerson != NULL && !game->gameUI->isThirdPersonView(game->singlePlayerNumber))
						fpCheck = true;
				} else {
					if (firstPerson != NULL)
						fpCheck = true;
				}

				if (fpCheck)
        {
          VC3 posDiff = unit->getPosition() - fpPosition;
          if (posDiff.GetSquareLength() < 
            FIRST_PERSON_VISION_RANGE * FIRST_PERSON_VISION_RANGE)
          {
            unit->visibility.setSeenByFirstPerson(true);
          } else {
            unit->visibility.setSeenByFirstPerson(false);
          }
        } else {
					bool isNowSeenByPlayer = unit->visibility.isSeenByPlayer(game->singlePlayerNumber);
					if ((!wasSeenByPlayer && isNowSeenByPlayer) || (wasSeenByPlayer && !isNowSeenByPlayer))
					{
						// pointers changed (add/remove torus for this unit)
						game->gameUI->setPointersChangedFlag(game->singlePlayerNumber);
						// and if it was hostile, put it to list of possible 
						// offscreen units...
						if (game->isHostile(game->singlePlayerNumber, unit->getOwner()))
						{
							if (isNowSeenByPlayer)
								game->gameUI->addHostileUnitPointer(0, unit);
							else
								game->gameUI->removeHostileUnitPointer(0, unit);
						}
					}
          unit->visibility.setSeenByFirstPerson(isNowSeenByPlayer);
        }
				if (SimpleOptions::getBool(DH_OPT_B_SHOW_ALL_UNITS))
				{
          unit->visibility.setSeenByFirstPerson(true);
				}


// TEMP: this has been disabled, as it conflicts with "hideAllHostiles" and hideUnit
//       (those rely on setting the visualobject's visiblity)
//
//				if (unit->getVisualObject() != NULL)
//					unit->getVisualObject()->setVisible(
//						unit->visibility.isSeenByFirstPerson());

        if (firstPerson != NULL && !game->gameUI->isThirdPersonView(game->singlePlayerNumber))
        {
          firstPerson->visibility.setSeenByFirstPerson(false);
					if (firstPerson->getVisualObject() != NULL)
						firstPerson->getVisualObject()->setVisible(false);
        }
      }

      // reset iteration to first unit

      delete unitIterator;
      unitIterator = new LinkedListIterator(allUnitsList);

			// recount units
			totalUnits = game->units->getAllUnitAmount();
			unitsInPass = totalUnits / VISIBILITY_CHECK_IN_PASSES;

    } else {

      // one unit done, select next unit...
      // skip any destroyed or inactive units

			// first set to null in case we have no more units to check
			currentUnit = NULL;

			// then try to find another unit to check
      while (unitIterator->iterateAvailable())
      {
	      currentUnit = (Unit *)unitIterator->iterateNext();
        if (currentUnit->isActive() && !currentUnit->isDestroyed())
        {
          break;
        }
      }

      passCount = 0;
      if (otherUnitIterator != NULL)
        delete otherUnitIterator;

			// HACK: other iterator for only hostiles...!!!
			if (currentUnit != NULL)
			{
				// HACK: assuming certain player sides (0 player,1 hostile,2 neutral,3 ally)

				if (currentUnit->getOwner() == 0)
				{
					// player vs hostiles
					otherUnitIterator = new LinkedListIterator(game->units->getOwnedUnits(1));
				}
				else if (currentUnit->getOwner() == 1)
				{
					// hostiles vs player+allies
					if (temp_player_and_allies_list == NULL
						|| temp_player_and_allies_list_amount != game->units->getOwnedUnitAmount(0) + game->units->getOwnedUnitAmount(3)) 
					{
						if (temp_player_and_allies_list != NULL)
							delete temp_player_and_allies_list;

						temp_player_and_allies_list_amount = game->units->getOwnedUnitAmount(0) + game->units->getOwnedUnitAmount(3);
						temp_player_and_allies_list = new LinkedList();
						LinkedListIterator pliter(game->units->getOwnedUnits(0));
						LinkedListIterator allyiter(game->units->getOwnedUnits(3));
						while (pliter.iterateAvailable())
						{
							Unit *copyuptr = (Unit *)pliter.iterateNext();
							temp_player_and_allies_list->append(copyuptr);
						}
						while (allyiter.iterateAvailable())
						{
							Unit *copyuptr = (Unit *)allyiter.iterateNext();
							temp_player_and_allies_list->append(copyuptr);
						}
					}
					otherUnitIterator = new LinkedListIterator(temp_player_and_allies_list);
				}
				else if (currentUnit->getOwner() == 2)
				{
					// neutrals vs none
					if (temp_empty_list == NULL) 
					{
						temp_empty_list = new LinkedList();
					}
					otherUnitIterator = new LinkedListIterator(temp_empty_list);
				}
				else if (currentUnit->getOwner() == 3)
				{
					// allies vs hostiles
					otherUnitIterator = new LinkedListIterator(game->units->getOwnedUnits(1));
				} else {
					// some other side???
					assert(!"UnitVisibilityChecker - unsupported unit owner.");
					otherUnitIterator = new LinkedListIterator(game->units->getAllUnits());
				}
			} else {
				otherUnitIterator = new LinkedListIterator(game->units->getAllUnits());
			}
    }
  }

	return didRaytrace;
}

void UnitVisibilityChecker::setUpdateEnabled(bool enabled)
{
	updateDisabled = !enabled;
}

void UnitVisibilityChecker::unitSeesUnit(Unit *unit, Unit *other, float dist)
{
	int player = unit->getOwner();

  // set the other unit to be seen by this unit's player
  other->visibility.setToBeSeenByPlayer(player, true);

  // if it's hostile set this unit "looking at" it
  // (choosing closest one)
  if (!other->isDestroyed()
    && game->isHostile(player, other->getOwner()))
  {
    if (unit->getToBeSeenUnit() == NULL
      || (unit->getToBeSeenUnitDistance() > dist
				&& (other->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
					|| unit->getToBeSeenUnit()->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS))
			|| (unit->getToBeSeenUnit()->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS
				&& other->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS))
    {
      unit->setToBeSeenUnit(other);
      unit->setToBeSeenUnitDistance(dist);
    }
  }
}

}

