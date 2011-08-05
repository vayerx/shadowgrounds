
#include "precompiled.h"

#include <math.h>

#include "ArmorUnitActor.h"

#include "UnitActAnimationRequests.h"
#include "ObstacleMapUnitObstacle.h"
#include "scaledefs.h"
#include "unittypes.h"
#include "Unit.h"
#include "UnitList.h"
#include "UnitType.h"
#include "CoverMap.h"
#include "Game.h"
#include "GameUI.h"
#include "GameRandom.h"
#include "GameScene.h"
#include "GameMap.h"
#include "SlopeTypes.h"
#include "physics/AbstractPhysicsObject.h"
#include "SimpleOptions.h"
#include "options/options_game.h"
#ifdef PROJECT_CLAW_PROTO
#include "../ui/Animator.h"
#include "../ui/AnimationSet.h"
#endif

#include "../util/AI_PathFind.h"

// temp debugging
#include "../convert/str2int.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

ArmorUnitActor::ArmorUnitActor(Game *game)
{
	this->game = game;
}


void ArmorUnitActor::addUnitObstacle(Unit *unit)
{
	if (!unit->doesCollisionBlockOthers())
		return;

	if (!unit->getUnitType()->isFlying() && ((!unit->isDestroyed()
		&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		|| unit->getUnitType()->isBlockIfDestroyed()))
	{
		bool lineblock = unit->getUnitType()->isLineBlock();
		bool rightside = unit->isUnitMirrorSide();
		bool bothLineSides = unit->getUnitType()->hasBothLineSides();
		int lineWidth = unit->getUnitType()->getLineBlockWidth();
		ObstacleMapUnitObstacle::addObstacle(game, unit, unit->getUnitType()->getBlockRadius(), 
			(int)(UNIT_OBSTACLE_HEIGHT / game->gameMap->getScaleHeight()),
			lineblock, !rightside || bothLineSides, rightside || bothLineSides, lineWidth);
	}
}


void ArmorUnitActor::removeUnitObstacle(Unit *unit)
{
	if (!unit->getUnitType()->isFlying() 
		&& ((!unit->isDestroyed()
		&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		 || unit->obstacleExists))
	{
		bool lineblock = unit->getUnitType()->isLineBlock();
		bool rightside = unit->isUnitMirrorSide();
		bool bothLineSides = unit->getUnitType()->hasBothLineSides();
		int lineWidth = unit->getUnitType()->getLineBlockWidth();
		ObstacleMapUnitObstacle::removeObstacle(game, unit, unit->getUnitType()->getBlockRadius(), 
			(int)(UNIT_OBSTACLE_HEIGHT / game->gameMap->getScaleHeight()),
			lineblock, !rightside || bothLineSides, rightside || bothLineSides, lineWidth);
	}
}


void ArmorUnitActor::moveUnitObstacle(Unit *unit, int x, int y)
{
	if (!unit->doesCollisionBlockOthers())
		return;

	if (
		!unit->getUnitType()->isFlying() 
		&& ( ( !unit->isDestroyed()
		       && unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS )
	        || unit->getUnitType()->isBlockIfDestroyed()
			  || unit->obstacleExists
			)
	   )
	{
		bool lineblock = unit->getUnitType()->isLineBlock();
		bool rightside = unit->isUnitMirrorSide();
		bool bothLineSides = unit->getUnitType()->hasBothLineSides();
		int lineWidth = unit->getUnitType()->getLineBlockWidth();
		ObstacleMapUnitObstacle::moveObstacle(game, unit, x, y, unit->getUnitType()->getBlockRadius(), 
			(int)(UNIT_OBSTACLE_HEIGHT / game->gameMap->getScaleHeight()),
			lineblock, !rightside || bothLineSides, rightside || bothLineSides, lineWidth);
	}
}


void ArmorUnitActor::stopUnit(Unit *unit)
{
	/*
	while (!unit->isAtPathEnd())
	{
		unit->setPathIndex(unit->getPathIndex() + 1);
	}
	*/
	unit->setPath(NULL);
	unit->setWaypoint(unit->getPosition());
	unit->setFinalDestination(unit->getPosition());
}


frozenbyte::ai::Path *ArmorUnitActor::solvePath(Unit *unit, const VC3 &startPosition, VC3 &endPosition, 
	int maxDepth)
{
	// WARNING: may modify the endPosition value, which is a reference!

	UnitType *unitType = unit->getUnitType();

	int coverAvoid = unitType->getCoverAvoid();
	int coverBlock = unitType->getCoverBlock();
	int lightAvoid = unitType->getLightAvoid();

	VC3 coverCheckPos = unit->getPosition();
	if (coverBlock > 0
		&& game->gameMap->isWellInScaledBoundaries(coverCheckPos.x, coverCheckPos.z))
	{
		int covx = game->gameMap->scaledToPathfindX(coverCheckPos.x);
		int covy = game->gameMap->scaledToPathfindY(coverCheckPos.z);
		int coverDist = game->gameMap->getCoverMap()->getDistanceToNearestCover(covx, covy);
		if (coverDist < coverBlock)
			coverBlock = coverDist;
	}

	// HACK: slowly moving units don't care so much about cover avoiding
	// (most likely patrolling units)
	if (coverAvoid > 2 && unit->getSpeed() == Unit::UNIT_SPEED_SLOW)
		coverAvoid =- 1;

	// HACK: sprinting units do care more about cover avoiding...
	// WARNING: may cause some problems when destination in cover..?
	// (may result into unability to reach that place)
	if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
		coverAvoid =+ 2;

	game->gameMap->keepWellInScaledBoundaries(&endPosition.x, &endPosition.z);

	// if the destination is blocked get somewhere near...

  // WARNING: this may cause some major malfunction to the pathfind when outside map...
	// (changing start position)
	// HOWEVER, without this, it would crash (because of pathfind data access outside map)
	VC3 startPositionInBound = startPosition;
	game->gameMap->keepWellInScaledBoundaries(&startPositionInBound.x, &startPositionInBound.z);

	// HACK: or if destination covered, get somewhere near...
	int px = game->gameMap->scaledToPathfindX(endPosition.x);
	int py = game->gameMap->scaledToPathfindY(endPosition.z);

	VC3 origEndPos = endPosition;

	int failCount = 0;
	//while (game->getGameScene()->isBlockedAtScaled(endPosition.x, endPosition.z, game->gameMap->getScaledHeightAt(endPosition.x, endPosition.z)))
	while (game->getGameScene()->isBlockedAtScaled(endPosition.x, endPosition.z, 0)
		|| game->gameMap->getCoverMap()->getDistanceToNearestCover(px, py) < coverAvoid)
	{
		failCount++;
		if (failCount > 1000) 
		{
			Logger::getInstance()->debug("ArmorUnitActor::solvePath - Failed to find suitable end position.");
			break;
		}
		// randomly move one meter on x- and z-axis
		//endPosition.x += (float)((game->gameRandom->nextInt() % 3) - 1);
		//endPosition.z += (float)((game->gameRandom->nextInt() % 3) - 1);
		// randomly move further away from the preferred position
		// the more failures, the further away the random goes (max. 15m per axis)
		endPosition.x = origEndPos.x + (float)((game->gameRandom->nextInt() % 31) - 15) * (float)failCount / 1000.0f;
		endPosition.z = origEndPos.z + (float)((game->gameRandom->nextInt() % 31) - 15) * (float)failCount / 1000.0f;
		game->gameMap->keepWellInScaledBoundaries(&endPosition.x, &endPosition.z);
		px = game->gameMap->scaledToPathfindX(endPosition.x);
		py = game->gameMap->scaledToPathfindY(endPosition.z);
	}

	assert(game->getGameScene()->getBlockingCount(px, py, 0) == 0);


	float maxClimb = unitType->getMaxClimb();
	float climbCost = unitType->getClimbCost();

	bool foundPath;
	frozenbyte::ai::Path *path = NULL;
	float sx = startPositionInBound.x;
	float sy = startPositionInBound.z;
	float ex = endPosition.x;
	float ey = endPosition.z;
	VC3 unitPos = unit->getPosition();

	if (unitType->isFlying())
	{
		path = new frozenbyte::ai::Path();
		//path->addPoint((int)((ex + game->gameMap->getScaledSizeX()/2.0f) / game->gameMap->getScaleX()), 
		//	(int)((ey + game->gameMap->getScaledSizeY()/2.0f) / game->gameMap->getScaleY()));
		path->addPoint(game->gameMap->scaledToPathfindX(ex), game->gameMap->scaledToPathfindY(ey));
		path->addPoint(0, 0); // gets skipped
		foundPath = true;
	} else {
		removeUnitObstacle(unit);

		if (game->getGameScene()->isBlockedAtScaled(sx, sy, 0))
		{
			int psx = game->gameMap->scaledToPathfindX(sx);
			int psy = game->gameMap->scaledToPathfindY(sy);

			int psizeX = game->gameMap->getPathfindSizeX();
			int psizeY = game->gameMap->getPathfindSizeY();
			if (psx >= 1 && psy >= 1 && psx < psizeX - 1 && psy < psizeY - 1)
			{
				if (game->getGameScene()->getBlockingCount(psx, psy - 1, 0) == 0)
					{ psy -= 1; }
				else if (game->getGameScene()->getBlockingCount(psx, psy + 1, 0) == 0)
					{ psy += 1; }
				else if (game->getGameScene()->getBlockingCount(psx - 1, psy + 1, 0) == 0)
					{ psx -= 1; }
				else if (game->getGameScene()->getBlockingCount(psx + 1, psy, 0) == 0)
					{ psx += 1; }
				else if (game->getGameScene()->getBlockingCount(psx + 1, psy + 1, 0) == 0)
					{ psx += 1; psy += 1; }
				else if (game->getGameScene()->getBlockingCount(psx - 1, psy + 1, 0) == 0)
					{ psx -= 1; psy += 1; }
				else if (game->getGameScene()->getBlockingCount(psx - 1, psy - 1, 0) == 0)
					{ psx -= 1; psy -= 1; }
				else if (game->getGameScene()->getBlockingCount(psx + 1, psy - 1, 0) == 0)
					{ psx += 1; psy -= 1; }
				//assert(game->getGameScene()->getBlockingCount(psx, psy, 0) == 0);
			
				sx = game->gameMap->pathfindToScaledX(psx);
			  sy = game->gameMap->pathfindToScaledY(psy);
			}
		}

		path = new frozenbyte::ai::Path();

		// HACK: human player gets full depth pathfind 
		// (if path not found here while units block), other have to do with 
		// half of that 
		// first start out with this 30% pathfind (when units are blocking)
		int depth = 30;

		if (depth > maxDepth / 2)
			depth = maxDepth / 2;

		// TEMP!!!
		/*
		int psizeX = game->gameMap->getPathfindSizeX();
		int psizeY = game->gameMap->getPathfindSizeY();
		int psx = game->gameMap->scaledToPathfindX(sx);
		int psy = game->gameMap->scaledToPathfindY(sy);
		int pex = game->gameMap->scaledToPathfindX(ex);
		int pey = game->gameMap->scaledToPathfindY(ey);
		if(!(psx > 100 && psy > 100 && psx < psizeX - 100 && psy < psizeY - 100))
		{
			assert(!"asdfasdf");
		}
		if(!(pex > 100 && pey > 100 && pex < psizeX - 100 && pey < psizeY - 100))
		{
			assert(!"asdfasdf");
		}
		*/


		foundPath = game->getGameScene()->findPath(path, 
			sx, sy, ex, ey, maxClimb, climbCost, coverAvoid, coverBlock,
			depth, lightAvoid);

		// no path? maybe blocked by another unit... 
		// try again, ignoring units...
		if (!foundPath)
		{
			delete path;
			path = new frozenbyte::ai::Path();

			// first remove all friendly unit obstacles...
			LinkedList *ulist = game->units->getOwnedUnits(unit->getOwner());
			LinkedListIterator iter = LinkedListIterator(ulist);
			while (iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (u != unit && u->isActive() && !u->isDestroyed())
				{
					UnitActor *ua = getUnitActorForUnit(u);
					VC3 otherupos = u->getPosition();
					if (game->gameMap->isWellInScaledBoundaries(otherupos.x, otherupos.z))
					{
						ua->removeUnitObstacle(u);
					}
				}
			}

			// HACK: human player gets full depth pathfind, other have to do with 
			// half of that
			depth = 50;
			if (unit->getOwner() == game->singlePlayerNumber)
				depth = 100;

			if (depth > maxDepth)
				depth = maxDepth;

			foundPath = game->getGameScene()->findPath(path, 
				sx, sy, ex, ey, maxClimb, climbCost, coverAvoid, coverBlock,
				depth, lightAvoid);

			// then restore the friendly unit obstacles...
			iter = LinkedListIterator(ulist);
			while (iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (u != unit && u->isActive() && !u->isDestroyed())
				{
					UnitActor *ua = getUnitActorForUnit(u);
					VC3 otherupos = u->getPosition();
					if (game->gameMap->isWellInScaledBoundaries(otherupos.x, otherupos.z))
					{
						ua->addUnitObstacle(u);
					}
				}
			}
		}

		// still no path. just fail then. :(
		if (!foundPath)
		{
			delete path;
			path = NULL;
		}
		addUnitObstacle(unit);

		/*
		if (foundPath)
		{
			Logger::getInstance()->error("FOUND PATH");
		} else {
			Logger::getInstance()->error("DID NOT FIND PATH");
		}
		*/
	}
	return path;
}


bool ArmorUnitActor::setPathTo(Unit *unit, const VC3 &destination_)
{
	VC3 destination = destination_;
	frozenbyte::ai::Path *path = solvePath(unit, unit->getPosition(), destination);
	
	// luckily, getPath may modify the destination value, if it is blocked
	// so no need to check that here.

	if (path != NULL)
	{
		unit->setPath(path);
		unit->setPathIndex(unit->getPathIndex() + 1);
		// (...path object is now contained within the unit, 
		// unit will handle it's proper deletion)
		unit->setWaypoint(unit->getPosition());
		unit->setFinalDestination(VC3(destination.x, 
			game->gameMap->getScaledHeightAt(destination.x, destination.z), 
			destination.z));
		return true;
	} else {		
		unit->setPath(NULL);
		unit->setFinalDestination(unit->getPosition());
		unit->setWaypoint(unit->getPosition());
		return false;
	} 
}


void ArmorUnitActor::act(Unit *unit)
{
	// here we may want to cast the unit to ArmorUnit or something...

	UnitType *unitType = unit->getUnitType();
	assert(unitType != NULL);

	//GameMap *gameMap = game->gameMap;

	VC3 oldPosition = unit->getPosition();
	//VC3 position = unit->getPosition();
	//VC3 oldPosition = position;
	//VC3 velocity = unit->getVelocity();
	//VC3 rotation = unit->getRotation();

	bool rotated = false;
	bool accelerated = false;

	float oldYRotation = unit->getRotation().y;

	UnitActAnimationRequests animRequests;
	
	bool normalState = true;

	// breathing outdoor steam animation thing
	actOutdoorBreathingSteam( unit );

	// animated units don't do normal acting...
	if (unit->isAnimated())
	{
		if(game->gameMap->isWellInScaledBoundaries(oldPosition.x, oldPosition.z))
		{
			// but update obstacles...
			removeUnitObstacle(unit);
			addUnitObstacle(unit);
		}

		return;
	}

	if (unitType->isStationary())
	{
		// a stationary unit...
		actStationary(unit, &animRequests);
	}

	if (unit->isDestroyed())
	{
#ifdef PROJECT_CLAW_PROTO
// HACK: for claw proto..
if (unit->isPhysicsObjectLock()
	&& unit->getGamePhysicsObject() == NULL)
	unit->setPhysicsObjectLock(false);
#endif

		// a destroyed unit, don't do much...
		actDestroyed(unit, &animRequests);
	} else {

		// see if the unit is idle and possibly act based on that...
		if (unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		{
			// this done before anything else, cos it may set the movestate
			decideIdle(unit, &animRequests);
		}

		// the idle acting may have set the unit in an abnormal state
		// (idle state, that is)
		if (unit->getMoveState() != Unit::UNIT_MOVE_STATE_NORMAL)
		{
			normalState = false;
		}

		VC3 waypoint = unit->getWaypoint();

		bool doRotation = false;
		bool doMove = false;
		bool doForwardMove = false;
		bool doLeftMove = false;
		bool doRightMove = false;
		bool doBackMove = false;
		bool doLeftRotation = false;
		bool doRightRotation = false;
		bool doFire = false;

		if (unit->getWalkDelay() > 0)
		{
			unit->setWalkDelay(unit->getWalkDelay() - 1);

			// player cannot move yet...
			// (probably fired a heavy weapon a moment ago)
			actNotYetAllowedToWalk(unit, &animRequests);

			actJump(unit, &doMove, &doRotation, &doForwardMove, &doBackMove, &doLeftMove, &doRightMove);

		} else {

			float rotationAngle = 0;

			if (unit->isDirectControl())
			{
				if (unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER)
				{
					// the unit is directly controlled by the player...
					decideBasedOnLocalPlayerDirectControl(unit, &doMove, &doRotation, 
						&doForwardMove, &doBackMove, &doLeftMove, &doRightMove, 
						&doLeftRotation, &doRightRotation, &doFire, &rotationAngle);
				}
				else if (unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI)
				{
					decideBasedOnAIDirectControl(unit, &doMove, &doRotation, 
						&doForwardMove, &doBackMove, &doLeftMove, &doRightMove, 
						&doLeftRotation, &doRightRotation, &doFire, &rotationAngle);
				} else {
					// nop?
				}
			} else {
				// the unit is not directly controlled by the player,
				// thus we need to use the AI to decide walking and turning...
				decideTurnAndWalk(unit, &doMove, &doRotation, &rotationAngle);

			}

			actJump(unit, &doMove, &doRotation, &doForwardMove, &doBackMove, &doLeftMove, &doRightMove);

			if (normalState)
			{
				actTurn(unit, &animRequests, doRotation, rotationAngle, doMove,
					&rotated);

				if (!unitType->isStationary())
				{
					actWalk(unit, &animRequests, doMove, doForwardMove, doBackMove,
						doLeftMove, doRightMove, rotated, &accelerated);

					// WARNING: actNoisy feature disabled! (performance optimization)
					// NOTE: actNoisy feature disabled! (performance optimization)
					/*
					if (accelerated) 
					{
						if (unit->getSpeed() != Unit::UNIT_SPEED_SLOW 
							&& unit->getSpeed() != Unit::UNIT_SPEED_CRAWL
							&& unit->getReconValue() <= 0)
						{
							if (unit->makeStepNoise())
							{
								actNoisy(unit);
							}
						}
					}
					*/
				}
			} else {

				actPossiblyUnconscious(unit, &animRequests);

				actPossiblyRising(unit, &animRequests);

				actPossiblyProning(unit, &animRequests);

				actPossiblyStagger(unit, &animRequests);

				actPossiblyImpact(unit, &animRequests);

				actPossiblyElectrified(unit, &animRequests);

				actPossiblyStunned(unit, &animRequests);

				actPossiblyIdle(unit, &animRequests);

				VC3 velocity = unit->getVelocity();
				if (unitType->isSticky() && unit->isOnGround() && !unit->isSideways())
				{
					velocity.x = 0;
					velocity.y = 0;
					velocity.z = 0;
				} else {
					frictionVelocity(&velocity, unitType->getFriction());
				}
				unit->setVelocity(velocity);
			}
		}

		// target destroyed?
		if (unit->targeting.hasTarget())
		{
			if (unit->targeting.getTargetUnit() != NULL
				&& unit->targeting.getTargetUnit()->isDestroyed())
			{
				unit->targeting.clearTarget();
				game->gameUI->setPointersChangedFlag(unit->getOwner());
			}
		}

		if (normalState)
		{
			actTargeting(unit, &animRequests, doFire, rotated, doMove,
				doForwardMove, doBackMove, doLeftMove, doRightMove);
		}

		actWeaponry(unit, &animRequests, doFire, doMove);

		actMisc(unit, &animRequests);

		actPointed(unit);
	}

	if (!unitType->isStationary())
	{
		if (!unit->isDestroyed()
			|| (unit->getDisappearCounter() < (SimpleOptions::getInt(DH_OPT_I_CORPSE_DISAPPEAR_TIME) * 1000) / GAME_TICK_MSEC - UNIT_DISAPPEAR_FADE_TICKS)
			|| SimpleOptions::getInt(DH_OPT_I_CORPSE_DISAPPEAR_TIME) == 0)
		{
			// change position based on velocity
			actVelocity(unit, &animRequests, oldPosition);

			if (!unit->isPhysicsObjectLock())
			{
				actGravity(unit, &animRequests);

				if (unit->getSideGravityX() != 0.0f || unit->getSideGravityZ() != 0.0f)
				{
					actSideGravity(unit);
				}
			}

			actHover(unit, &animRequests, accelerated);

			// are we on ground or airborne? act based on that.
			{
				VC3 position = unit->getPosition();
				bool onground = false;
				if (unit->isSideways())
				{
					/*
					position.x += unit->getSideGravityX() / GAME_TICKS_PER_SECOND;
					position.z += unit->getSideGravityZ() / GAME_TICKS_PER_SECOND;
					if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
					{
						// TODO: optimize, doing lots of remove/add obst is not effective...
						removeUnitObstacle(unit);
						if (game->getGameScene()->isBlockedAtScaled(position.x, position.z, game->gameMap->getScaledHeightAt(position.x, position.y)))
						{
							onground = true;
						}
						addUnitObstacle(unit);
					} else {
						onground = true;
					}
					*/
					onground = true;
				} else {
					float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);
					if ((unitType->isSticky() && unit->isOnGround())
						|| position.y <= mapY + 0.001f)
					{
	
						onground = true;
					}
				}
				if (onground
					&& !unit->isPhysicsObjectLock())
				{
					actOnGround(unit, &animRequests);
				} else {
					actAirborne(unit, &animRequests, accelerated);
				}
			}
		}
	}

	actDelayedProjectiles(unit);

	// WARNING: actStealth feature disabled! (performance optimization)
	// NOTE: actStealth feature disabled! (performance optimization)
	/*
	if (unit->getStealthValue() > 0)
		actStealth(unit);
	*/

	if (unitType->isSquashable())
		actSquashable(unit);

	if (unit->getOnFireCounter() > 0) 
	{
		unit->setOnFireCounter(unit->getOnFireCounter() - 1);
	}

	// NOTE: can't do this or doors will break!
	//if (!unitType->isStationary())
	//{
	{
		VC3 curPosition = unit->getPosition();
		if (game->gameMap->isWellInScaledBoundaries(oldPosition.x, oldPosition.z)
			&& game->gameMap->isWellInScaledBoundaries(curPosition.x, curPosition.z))
		{
			actCollisions(unit, &animRequests, oldPosition);
		}
		/*
		if (unit->isSideways())
		{
			// if after collision the position (z) is higher than it was before collision, then we must
			// have collided with something underneath.
			if (unit->getPosition().z > curPosition.z)
			{
				//unit->setOnGroundSidewaysCounter();
			} else {
				if (unit->getPosition().z < curPosition.z)
				{
					//unit->decreaseOnGroundSidewaysCounter();
				}
			}
		}
		*/
	}
	//}

	actEffects(unit);

	float newYRotation = unit->getRotation().y;

	if (newYRotation > oldYRotation + 1.0f 
		|| newYRotation < oldYRotation - 180.0f)
	{
		animRequests.turnRight = true;
	}
	if (newYRotation < oldYRotation - 1.0f
		|| newYRotation > oldYRotation + 180.0f)
	{
		animRequests.turnLeft = true;
	}

#ifdef PROJECT_CLAW_PROTO
if (unit->getUnitType()->getUnitTypeId() == 455611187)
{
if (
		//unit->isPhysicsObjectLock()
		unit->variables.getVariable("isinclaw")
		&& !unit->isDestroyed()
		&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
{
	Animator::endBlendAnimation(unit, 1);
	Animator::endBlendAnimation(unit, 2);
	//Animator::endBlendAnimation(unit, 3);
	unit->getAnimationSet()->animate(unit, ANIM_SPECIAL1);
	return;
}
else
// warning: evil hack ahead
if(    unit->isPhysicsObjectLock()
	&& !unit->isOnGround()
	&& !unit->variables.getVariable("isinclaw")
	&& !unit->isDestroyed()
	)
{

	// Force visual object rotation towards opposite of direction thrown, looks better.
	VC3 vel = unit->getGamePhysicsObject()->getVelocity();
	MAT m;
	m.CreateCameraMatrix( VC3(), VC3(-vel.x, 0, vel.z) , VC3(0, 1, 0));
	QUAT rotat = m.GetRotation();
	
	unit->getVisualObject()->setRotationQuaternion( rotat );
	unit->getGamePhysicsObject()->setRotation( rotat );
	// Animation when thrown.
	if( !unit->getForcedAnimation() )
	{
		unit->setForcedAnimation( ANIM_SPECIAL11 );
		VC3 unitRot = unit->getRotation();
		unit->setRotation( unitRot.x, unit->getVisualObject()->getRenderYAngle(), unitRot.z);
	}

}
else
if( unit->getForcedAnimation() == ANIM_SPECIAL11 )
{
	// Make sure the forced flying animation won't continue.
	unit->setForcedAnimation( 0 );
}
}
#endif

	// finally set animation based on what we did...
	applyAnimations(unit, animRequests);
}


void ArmorUnitActor::actSideGravity(Unit *unit)
{
	VC3 position = unit->getPosition();
	VC3 velocity = unit->getVelocity();

	// for sideways units, do the coll check one block below the actual position
	if (unit->isSideways())
	{
		position.z -= game->gameMap->getScaledSizeY() / game->gameMap->getObstacleSizeY();
	}

	// NOTE: pos is apparently a temporary, that will not get put back to unit position.
	VC3 pos = position;

	pos.x += unit->getSideGravityX() / GAME_TICKS_PER_SECOND;
	pos.z += unit->getSideGravityZ() / GAME_TICKS_PER_SECOND;
	if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z)
		&& (unit->getSpeed() != Unit::UNIT_SPEED_JUMP || unit->getJumpCounter() < unit->getJumpTotalTime() - 1))
	{
		bool onground = false;
		bool onslope = false;

		// TODO: optimize, doing lots of remove/add obst is not effective...
		removeUnitObstacle(unit);

		SLOPE_TYPE slopeType;
		solveSlopeType(game->gameMap, game->getGameScene(), pos, onground, onslope, slopeType, position);

		if (!onground)
		{
			onground = unit->isOnPhysicsObject();
		}

		unit->setOnSlope(onslope);

		if (onground)
		{
			if (unit->getSideGravityX() != 0.0f)
				velocity.x = 0.0f;
			if (unit->getSideGravityZ() != 0.0f)
				velocity.z = 0.0f;
			unit->setGroundFriction(true);
			if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
			{
				unit->restoreDefaultSpeed();
				unit->setJumpCounter(0);
				unit->setJumpTotalTime(0);
			}
		} else {
			velocity.x += unit->getSideGravityX() / GAME_TICKS_PER_SECOND;
			velocity.z += unit->getSideGravityZ() / GAME_TICKS_PER_SECOND;
			unit->setGroundFriction(false);
		}

		// clamp max vel...
		float sideVelMax = unit->getSideVelocityMax() / GAME_TICKS_PER_SECOND;
		if (velocity.x > sideVelMax)
		{
			velocity.x = sideVelMax;
		} 
		else if (velocity.x < -sideVelMax)
		{
			velocity.x = -sideVelMax;
		} 
		if (velocity.z > sideVelMax)
		{
			velocity.z = sideVelMax;
		} 
		else if (velocity.z < -sideVelMax)
		{
			velocity.z = -sideVelMax;
		} 
		//if (velocity.GetSquareLength() > sideVelMax * sideVelMax)
		//{
		//	velocity = velocity.GetNormalized() * sideVelMax;
		//}

		addUnitObstacle(unit);
	}

	// end: for sideways units, do the coll check one block below the actual position
	if (unit->isSideways())
	{
		position.z += game->gameMap->getScaledSizeY() / game->gameMap->getObstacleSizeY();
	}

	unit->setPosition(position);
	unit->setVelocity(velocity);
}

}

