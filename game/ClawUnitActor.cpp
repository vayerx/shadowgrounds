
#include "precompiled.h"

#include <math.h>

#include "ClawUnitActor.h"

#include "UnitActAnimationRequests.h"
#include "scaledefs.h"
#include "unittypes.h"
#include "Unit.h"
#include "UnitList.h"
#include "UnitType.h"
#include "Game.h"
#include "GameUI.h"
#include "GameUI.h"
#include "GameMap.h"
#include "Weapon.h"
#include "direct_controls.h"
#include "SimpleOptions.h"
#include "options/options_game.h"
#include "options/options_physics.h"

#include "../util/PositionsDirectionCalculator.h"
#include "../util/AngleRotationCalculator.h"

#include "../game/ClawController.h"
#include "../ui/GameController.h"
#include "../ui/Animator.h"
#include "../ui/AnimationSet.h"
#include "../ui/animdefs.h"
#include "keyb3.h"

#include "..\util\Debug_MemoryManager.h"

static const int TARGET_ADVANCE_TICKS = 20;


namespace game
{

ClawUnitActor::ClawUnitActor(Game *game)
{
	this->game = game;
}


void ClawUnitActor::addUnitObstacle(Unit *unit)
{
}


void ClawUnitActor::removeUnitObstacle(Unit *unit)
{
}


void ClawUnitActor::moveUnitObstacle(Unit *unit, int x, int y)
{
}


void ClawUnitActor::stopUnit(Unit *unit)
{
	unit->setWaypoint(unit->getPosition());
	unit->setFinalDestination(unit->getPosition());
	unit->setPath(NULL);
}


frozenbyte::ai::Path *ClawUnitActor::solvePath(Unit *unit, const VC3 &startPosition, VC3 &endPosition, 
	int maxDepth)
{
	frozenbyte::ai::Path *path = new frozenbyte::ai::Path();
	path->addPoint(game->gameMap->scaledToPathfindX(endPosition.x), game->gameMap->scaledToPathfindY(endPosition.z));
	path->addPoint(0, 0); // gets skipped
	return path;
}


bool ClawUnitActor::setPathTo(Unit *unit, VC3 &destination_)
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



void ClawUnitActor::actDirectClawControls(Unit *unit)
{
	int weapTypeNum = 0;
	static int rotating = 0;
  static bool claw_near = true;

	UnitType *unitType = unit->getUnitType();

	if (rotating) rotating--;

	// For longer claw -> 5.5, 5.0

	const game::ControlSettings &controlSettings = game->getClawController()->getControlSettings();
	if(game->getClawController()->hasActor())
	{
		if(game->getClawController()->getTargetPosition(TARGET_ADVANCE_TICKS).GetRangeTo(unit->getPosition()) > controlSettings.animationSwitchFarPoseRange) 
		 claw_near = false;
    if(game->getClawController()->getTargetPosition(TARGET_ADVANCE_TICKS).GetRangeTo(unit->getPosition()) < controlSettings.animationSwitchNearPoseRange && !rotating) 
		 claw_near = true;
	}
	else
	{
    if(game->getClawController()->getTargetPosition(TARGET_ADVANCE_TICKS).GetRangeTo(unit->getPosition()) > controlSettings.animationSwitchFarPoseRange) 
		 claw_near = false;
    if(game->getClawController()->getTargetPosition(TARGET_ADVANCE_TICKS).GetRangeTo(unit->getPosition()) < controlSettings.animationSwitchNearPoseRange && !rotating) 
		 claw_near = true;
	}

	//bool sprint_key = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_SPRINT, unit);

	bool sprint = false;
	// TODO: ask the clawcontroller if we should sprint - if claw (= pad aiming/mouse) is not moving?
	// if sprint key is pressed, then force the claw not to move

static int thefoocounter = 0;

	if (!game->getClawController()->hasActor()
		&& !game->getClawController()->isClawMoving())
	{
		thefoocounter++;
		if (thefoocounter > 67)
		{
			sprint = true;
		}
	} else {
		thefoocounter = 0;
	}

// SPRINT DISABLED FOR NOW!
sprint = false;

	if (game->getClawController()->getPrepareAction() != ClawController::ActionNone)
	{
		weapTypeNum = 6;
	} else
	{
		if(sprint)
		{
			weapTypeNum = 5;
		} else {
			if(!claw_near)
			{
				if(game->getClawController()->hasActor())
				{
					weapTypeNum = 4;
				}
				else
				{
					weapTypeNum = 2;
				}
			}
			else
			{
				if(game->getClawController()->hasActor())
				{
					weapTypeNum = 3;
				}
				else
				{
					weapTypeNum = 1;
				}
			}
		}
	}

	bool weaponAnimationTypeChanged = false;

	// HACK: hack hack hack static!
	static int lastWeapTypeNum = -1;
	static int weaponChanging = 0;
	if (weapTypeNum != lastWeapTypeNum)
	{
		lastWeapTypeNum = weapTypeNum;
		weaponAnimationTypeChanged = true;
		weaponChanging = GAME_TICKS_PER_SECOND;
	}
  if (weaponChanging) weaponChanging--;
	int anim = ANIM_STAND_TYPE0 + weapTypeNum;

	// get the controls...
	bool up = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FORWARD, unit);
	bool left = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_LEFT, unit);
	bool right = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_RIGHT, unit);
	bool down = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_BACKWARD, unit);

	// initial control coord system (controls in camera coord sys)
	VC3 upvec = VC3(0,1,0);
	VC3 controlFrontVec = VC3(0,0,-1);
	VC3 controlSideVec = controlFrontVec.GetCrossWith(upvec);

	// HACK: hack hack hack static!
	// (if we haven't moved, this is the last moved direction)
	static VC3 controlDirVec = VC3(1,0,0);
	bool doMovement = false;

	if (up || down || left || right)
	{
		VC3 newControlDirVec = VC3(0,0,0);
		if(!game::SimpleOptions::getBool(DH_OPT_B_CLAW_MOUSE))
		{
			int jx = 0;
			int jy = 0;
			Keyb3_ReadJoystick(0, &jx, &jy, 0, 0, 0, 0);
			//if(abs(jx) < 4)
			//	jx = 0;
			//if(abs(jy) < 4)
			//	jy = 0;

			newControlDirVec -= controlSideVec * float(jx);
			newControlDirVec -= controlFrontVec * float(jy);
		}
		else
		{
			if (up)
				newControlDirVec += controlFrontVec;
			if (down)
				newControlDirVec -= controlFrontVec;
			if (left)
				newControlDirVec += controlSideVec;
			if (right)
				newControlDirVec -= controlSideVec;
		}

		if (newControlDirVec.x != 0 || newControlDirVec.y != 0 || newControlDirVec.z != 0)
		{
			newControlDirVec.Normalize();
			controlDirVec = newControlDirVec;
			doMovement = true;
		}
	}

	// camera front/side vectors (height is flattened off)
	VC3 cameraFrontVec = game->gameUI->getGameCamera()->getTargetPosition() - game->gameUI->getGameCamera()->getPosition();
	cameraFrontVec.y = 0.0f;
	if (cameraFrontVec.GetSquareLength() < 0.0001f)
	{
		cameraFrontVec = controlFrontVec;
	}
	cameraFrontVec.Normalize();
	VC3 cameraSideVec = cameraFrontVec.GetCrossWith(upvec);

	// translate movement direction from camera coord system to global ocoord. system	
	// this is the actual movement direction vector.
	VC3 globalDirVec = (cameraFrontVec * controlDirVec.GetDotWith(controlFrontVec)) + (cameraSideVec * controlDirVec.GetDotWith(controlSideVec));

	VC3 velocity = unit->getVelocity();

	// move
	if (doMovement)
	{
		float accel = unitType->getAcceleration() / GAME_TICKS_PER_SECOND;
		velocity += globalDirVec * accel;

		float maxspeed = unitType->getMaxSpeed() / GAME_TICKS_PER_SECOND;
		if (sprint)
		{
			maxspeed = unitType->getSprintSpeed() / GAME_TICKS_PER_SECOND;
		}
		if (weapTypeNum == 6)  maxspeed = unitType->getSlowSpeed() / GAME_TICKS_PER_SECOND;
		float velTotalSq = velocity.GetSquareLength();
		if (velTotalSq > maxspeed*maxspeed)
		{
			velocity *= maxspeed / (float)sqrtf(velTotalSq);
		}

	}

	// friction...
	frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);

	unit->setVelocity(velocity);

	// where do we move to?
	float moveAngle = util::PositionDirectionCalculator::calculateDirection(VC3(0,0,0), globalDirVec);
	float rotateToAngle = moveAngle;



	bool doStrafeLeftAnim = false;
	bool doStrafeRightAnim = false;
	bool doStrafeBackAnim = false;

	static float standAngle = 0;
	
	// where do we aim?
	float aimDestAngle = moveAngle;
	char *aimBone = unitType->getAimBone();
	if (aimBone != NULL)
	{
		VC3 unitPos = unit->getPosition();

		
/*		static VC3 aimPos = VC3(0,0,0);
		VC3 aimDiff = game->getClawController()->getClawPosition() - unitPos;
		if (aimDiff.GetSquareLength() > 2.0f * 2.0f)
		{
			aimPos = game->getClawController()->getClawPosition();
		}*/
		
//		VC3 aimPos = game->getClawController()->getClawPosition();
		VC3 aimPos = game->getClawController()->getTargetPosition(TARGET_ADVANCE_TICKS);

		aimDestAngle = util::PositionDirectionCalculator::calculateDirection(unitPos, aimPos);

		if (doMovement)
		{
			// --- THE HORRIBLE STRAFE LOGIC BEGINS ---
			float insideFwd = util::AngleRotationCalculator::getRotationForAngles(moveAngle, aimDestAngle, 45.0f);
			if (insideFwd == 0)
			{
				rotateToAngle = moveAngle;
			} else {
				float insideFwdDiag = util::AngleRotationCalculator::getRotationForAngles(moveAngle, aimDestAngle, 90.0f+45.0f);
				if (insideFwdDiag == 0)
				{
					if (insideFwd < 0)
					{
						rotateToAngle = moveAngle - 90.0f;
						if (rotateToAngle < 0.0f) rotateToAngle += 360.0f;
						doStrafeRightAnim = true;
					} else {
						rotateToAngle = moveAngle + 90.0f;
						if (rotateToAngle >= 360.0f) rotateToAngle -= 360.0f;
						doStrafeLeftAnim = true;
					}
				} else {
					rotateToAngle = moveAngle + 180.0f;
					if (rotateToAngle >= 360.0f) rotateToAngle -= 360.0f;
					doStrafeBackAnim = true;
				}
			}
			// --- THE HORRIBLE STRAFE LOGIC ENDS ---

		} else {
			if (weaponAnimationTypeChanged)
			{
				standAngle = aimDestAngle;
			}
			rotateToAngle = standAngle;

			// HACK: ...
			float maxTwist = 45.0f;
			if (weapTypeNum == 6)
			{
				maxTwist = 10.0f;
			}

			while (true)
			{
				float tooFarTwist = util::AngleRotationCalculator::getRotationForAngles(rotateToAngle, aimDestAngle, maxTwist+10);
				if (tooFarTwist > 0)
				{
					rotateToAngle += maxTwist*2;
					if (rotateToAngle >= 360.0f) rotateToAngle -= 360.0f;
				}
				else if (tooFarTwist < 0)
				{
					rotateToAngle -= maxTwist*2;
					if (rotateToAngle < 0.0f) rotateToAngle += 360.0f;
				} else {
					break;
				}
			}
		}
	}

	// When aiming, force rotation to dest.
	if (weapTypeNum == 6)
	{
		rotateToAngle = util::PositionDirectionCalculator::calculateDirection(
			unit->getPosition(),
			game->getClawController()->getTargetPosition(0));
	}

	if (doMovement)
	{
		standAngle = moveAngle;
		weaponChanging = GAME_TICKS_PER_SECOND / 4;
	}

	// rotate towards movement/strafe/whatever we just decided
	VC3 rot = unit->getRotation();

	float turnAccuracy = unitType->getTurningAccuracy();
	float turnSpeed = unitType->getTurning();
  // HACK, if unit is walking, slow it's turning speed down
	if (doMovement) {
		turnSpeed *= 0.7f;
	}

	float turnDir = util::AngleRotationCalculator::getRotationForAngles(rot.y, rotateToAngle, turnAccuracy);
//Logger::getInstance()->error(int2str((int)turnDir));
	rot.y += turnSpeed / GAME_TICKS_PER_SECOND * turnDir;
//	Logger::getInstance()->warning(int2str(rot.y));
//	Logger::getInstance()->warning(int2str(aimDestAngle));
	if (rotateToAngle - rot.y < -180) rotateToAngle +=360;
	if (rotateToAngle - rot.y > 180) rotateToAngle -=360;
	if (turnDir > 0 && rot.y > rotateToAngle)
	{
		rot.y = rotateToAngle;
	}
	if (turnDir < 0 && rot.y < rotateToAngle)
	{
		rot.y = rotateToAngle;
	}
	if (rot.y >= 360.0f) rot.y -= 360.0f;
	if (rot.y < 0.0f) rot.y += 360.0f;

	unit->setRotation(rot.x, rot.y, rot.z);

	// aim towards final aim direction...
	if (aimBone != NULL)
	{
		float aimAngle = aimDestAngle - rot.y;
		if (aimAngle < 0) aimAngle += 360.0f;

		unit->setLastBoneAimDirection(aimAngle);
		if (unit->getVisualObject() != NULL)
		{
			unit->getVisualObject()->rotateBone(aimBone, aimAngle, 0);
		}
	}

	// TODO: select correct animation
	if (doMovement)
	{
		if (doStrafeLeftAnim)
		{
			anim = ANIM_STRAFE_LEFT_TYPE0 + weapTypeNum;
		}
		else if (doStrafeRightAnim)
		{
			anim = ANIM_STRAFE_RIGHT_TYPE0 + weapTypeNum;
		}
		else if (doStrafeBackAnim)
		{
			anim = ANIM_RUN_BACKWARD_TYPE0 + weapTypeNum;
		} else {
			anim = ANIM_RUN_TYPE0 + weapTypeNum;
		}
	} else if (!weaponChanging) {

		// HACK: ... total hack!
		bool nappi = game->gameUI->getController(0)->isKeyDown(DH_CTRL_ATTACK_SECONDARY);

		if (turnDir < 0)
		{
			if (nappi && weapTypeNum == 4)
			{
				anim = ANIM_SPECIAL2;
			} else {
				anim = ANIM_TURN_LEFT_TYPE0 + weapTypeNum;
			}
			rotating = int(0.5*GAME_TICKS_PER_SECOND);
		}
		if (turnDir > 0)
		{
			if (nappi && weapTypeNum == 4)
			{
				anim = ANIM_SPECIAL1;
			} else {
				anim = ANIM_TURN_RIGHT_TYPE0 + weapTypeNum;
			}
			rotating = int(0.5*GAME_TICKS_PER_SECOND);
		}
	}

//Logger::getInstance()->error(int2str(anim));
	// apply the animation...
	if (unit->getAnimationSet() != NULL)
	{
		if (unit->getAnimationSet()->isAnimationInSet(anim))
		{
			unit->getAnimationSet()->animate(unit, anim);
			if (unit->getAnimationSet()->isAnimationStaticFactored(anim))
			{
				float fact = unit->getAnimationSet()->getAnimationStaticFactor(anim);
				ui::Animator::setAnimationSpeedFactor(unit, fact);
			} else {
				ui::Animator::setAnimationSpeedFactor(unit, 1.0f);
			}
		}	else {
			Logger::getInstance()->warning("ClawUnitActor::actDirectClawControls - Requested anim not found in animation set.");
			Logger::getInstance()->debug("anim id number follows:");
			Logger::getInstance()->debug(int2str(anim));
		}
	} else {
		Logger::getInstance()->warning("ClawUnitActor::actDirectClawControls - Unit has null animation set.");
	}
}



void ClawUnitActor::act(Unit *unit)
{
	// here we may want to cast the unit to ClawUnit or something...

	UnitType *unitType = unit->getUnitType();
	assert(unitType != NULL);

	VC3 oldPosition = unit->getPosition();

	bool rotated = false;
	bool accelerated = false;

	float rotationAngle = 0.0f;

	float oldYRotation = unit->getRotation().y;

	UnitActAnimationRequests animRequests;
	
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

	if (unit->isDestroyed())
	{
		// a destroyed unit, don't do much...
		actDestroyed(unit, &animRequests);
	} else {

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
			//actNotYetAllowedToWalk(unit, &animRequests);
// HACK: ..
unit->setVelocity(VC3(0,0,0));
animRequests.setSpecialAnim = ANIM_STAND_TYPE1;

		} else {
			if (unit->isDirectControl())
			{
				// the unit is directly controlled by the player...

				actDirectClawControls(unit);

			} else {
				// the unit is not directly controlled by the player,
				// thus we need to use the AI to decide walking and turning...
				decideTurnAndWalk(unit, &doMove, &doRotation, &rotationAngle);

				actTurn(unit, &animRequests, doRotation, rotationAngle, doMove,
					&rotated);

				actWalk(unit, &animRequests, doMove, doForwardMove, doBackMove,
					doLeftMove, doRightMove, rotated, &accelerated);
			}
		}

		// target destroyed?
		/*
		if (unit->targeting.hasTarget())
		{
			if (unit->targeting.getTargetUnit() != NULL
				&& unit->targeting.getTargetUnit()->isDestroyed())
			{
				unit->targeting.clearTarget();
				game->gameUI->setPointersChangedFlag(unit->getOwner());
			}
		}
		*/

		//actTargeting(unit, &animRequests, doFire, rotated, doMove,
		//	doForwardMove, doBackMove, doLeftMove, doRightMove);

		//actWeaponry(unit, &animRequests, doFire, doMove);

		actMisc(unit, &animRequests);
	}

	if (!unit->isDestroyed()
		|| (unit->getDisappearCounter() < (SimpleOptions::getInt(DH_OPT_I_CORPSE_DISAPPEAR_TIME) * 1000) / GAME_TICK_MSEC - UNIT_DISAPPEAR_FADE_TICKS)
		|| SimpleOptions::getInt(DH_OPT_I_CORPSE_DISAPPEAR_TIME) == 0)
	{
		// change position based on velocity
		actVelocity(unit, &animRequests, oldPosition);

		actGravity(unit, &animRequests);

		actHover(unit, &animRequests, accelerated);

		VC3 position = unit->getPosition();
		bool onground = false;

		float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);
		if ((unitType->isSticky() && unit->isOnGround())
			|| position.y <= mapY + 0.001f)
		{
			onground = true;
		}
		if (onground)
		{
			actOnGround(unit, &animRequests);
		} else {
			actAirborne(unit, &animRequests, accelerated);
		}
	}

	actDelayedProjectiles(unit);

	/*
	VC3 curPosition = unit->getPosition();
	if (game->gameMap->isWellInScaledBoundaries(oldPosition.x, oldPosition.z)
		&& game->gameMap->isWellInScaledBoundaries(curPosition.x, curPosition.z))
	{
		actCollisions(unit, &animRequests, oldPosition);
	}
	*/

	//actEffects(unit);

/*
	float newYRotation = unit->getRotation().y;

if (unit->getIdString() != NULL
	&& strcmp(unit->getIdString(), "player1") == 0)
{
static int fooRight = 0;
static int fooLeft = 0;

	if (newYRotation > oldYRotation + 1.0f 
		|| newYRotation < oldYRotation - 180.0f)
	{
		//animRequests.turnRight = true;
		if (fooRight < 16)
			fooRight += 2;
	} else {
		if (fooRight > 0)
			fooRight--;
	}
	if (newYRotation < oldYRotation - 1.0f
		|| newYRotation > oldYRotation + 180.0f)
	{
		//animRequests.turnLeft = true;
		if (fooLeft < 16)
			fooLeft += 2;
	} else {
		if (fooLeft > 0)
			fooLeft--;
	}

	if (fooRight > 14)
		animRequests.turnRight = true;
	if (fooLeft > 14)
		animRequests.turnLeft = true;
}
*/

	// finally set animation based on what we did...
	if (!unit->isDirectControl())
	{
		applyAnimations(unit, animRequests);
	}
}



}

