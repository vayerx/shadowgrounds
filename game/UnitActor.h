
#ifndef UNITACTOR_H
#define UNITACTOR_H

#include <DatatypeDef.h>


#define UNIT_OBSTACLE_HEIGHT 2

// how many ticks does it take to fade out a disappearing destroyed unit
#define UNIT_DISAPPEAR_FADE_TICKS (67*6)


namespace frozenbyte
{
	namespace ai
	{
		class Path;
	}
}

namespace sfx
{
	class SoundSample;
}

namespace game
{
	class Game;
	class Unit;
	class Part;
	class UnitActAnimationRequests;
	class Weapon;

	// HACK: ...
	extern int unitactor_acttargeting_counter;

/**
 *
 * A class that moves units. 
 * This is a baseclass for subclasses having actual implementations.
 * A proper subclass of this should be selected based on unit's type.
 * For example, an armor unit should be moved by the ArmorUnitActor class.
 *
 * @version 1.0, 23.6.2002
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 * @see Unit
 *
 */

class UnitActor
{
public:

  /** 
   * Extending classes should implement this. 
   * @param unit  Unit to move
   */
	virtual void act(Unit *unit);

	virtual bool setPathTo(Unit *unit, const VC3 &destination) = 0;

	virtual frozenbyte::ai::Path *solvePath(Unit *unit, const VC3 &startPosition, VC3 &endPosition, int maxDepth = 100) = 0;

	virtual void stopUnit(Unit *unit) = 0;

	virtual void addUnitObstacle(Unit *unit) = 0;

	virtual void removeUnitObstacle(Unit *unit) = 0;

	// this should be protected maybe...
	virtual void moveUnitObstacle(Unit *unit, int x, int y) = 0;

	void reload(Unit *unit);

	// returns true if another unit was executed, else false
	bool doExecute(Unit *unit);

	// note, moveTime is given in game ticks
	void doSpecialMove(Unit *unit, bool forward, bool backward, bool left, bool right,
		int moveTime);

	// returns true if warp succeeded, else false (did not find a suitable position)
	bool warpToPosition(Unit *unit, const VC3 &position);

	// returns true if position can be warped to (accurately)
	bool canWarpToPosition(Unit *unit, const VC3 &position);

	// returns true if position can be warped to (accurately)
	bool canDoUnitExecute(Unit *unit, Unit *other);

	// resets the unit to "normal" state (for proper transition to anis, etc.)
	void resetToNormalState(Unit *unit);

	// creates a muzzleflash for this unit, for given weapon
	void createMuzzleflash(Unit *unit, int weaponNumber);

	// creates an eject effect for this unit, for given weapon
	void createEject(Unit *unit, int weaponNumber);

	UnitActor() : 
		game( NULL ), 
		movementParticleEffectCreationTime( 0 ), 
		movementSoundEffectCreationTime( 0 ),
		breathingParticleEffectCreationTime( 0 ),
		forceFootStep(0)
	{ }

	virtual ~UnitActor() 
	{ 
	// nop
	}

protected:
	Game *game;
	int movementParticleEffectCreationTime;
	int movementSoundEffectCreationTime;
	int breathingParticleEffectCreationTime;
	int forceFootStep;

	// plays a weapon sound.
	void makeFireSound(Unit *unit, int weapon, const char *fireSound);

	// will make the given velocity smaller based on given friction
	// the bigger the friction, the bigger the effect
	void frictionVelocity(VC3 *velocity, float friction);

	// moves the given (uint) rotation toward balance based on given factor.
	// the bigger the factor, the bigger (faster) the effect
	void balanceRotation(VC3 *rotation, float factor);

	// like balance, but not toward zero rotation.
	// rotateTo is an offset value to balance, between -180 to 180.
	// (not a normal 0-360 value).
	void rotateTo(const VC3 &rotateTo, VC3 *rotation, float factor);

	// a little collection of different kinds of sub-procedures for act...

	// makes the unit act as if it was destroyed (call this if the unit
	// really is destroyed)
	void actDestroyed(Unit *unit, UnitActAnimationRequests *animRequests);

	// act as a stationary unit (pretty much do nothing, just stand still)
	void actStationary(Unit *unit, UnitActAnimationRequests *animRequests);

	// do targeting and stuff...
	void actTargeting(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doFire, bool rotated,
		bool doMove, bool doForwardMove, bool doBackMove, 
		bool doLeftMove, bool doRightMove);

	// fire/reload the weapons and stuff...
	void actWeaponry(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doFire, bool doMove);

	// add velocity to current position
	void actVelocity(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition);

	// gravity
	void actGravity(Unit *unit, UnitActAnimationRequests *animRequests);

	// hovering
	void actHover(Unit *unit, UnitActAnimationRequests *animRequests,
		bool accelerated);

	// on ground
	void actOnGround(Unit *unit, UnitActAnimationRequests *animRequests);

	// in the air
	void actAirborne(Unit *unit, UnitActAnimationRequests *animRequests,
		bool accelerated);

	// collisions
	void actCollisions(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition);

	// misc stuff, such as cooling, recharging, hp raising, ...
	void actMisc(Unit *unit, UnitActAnimationRequests *animRequests);

	// see if the unit seems idle and act based on that (idle animations)
	void decideIdle(Unit *unit, UnitActAnimationRequests *animRequests);

	// should be called when the unit is not allowed to walk, due to 
	// unit's walkdelay, caused by shooting a big gun, for example.
	void actNotYetAllowedToWalk(Unit *unit, UnitActAnimationRequests *animRequests);

	// call when the unit is supposed to turn
	void actTurn(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doRotation, float rotationAngle, bool doMove, bool *rotated);

	// call when the unit is supposed to walk (move forward)
	void actWalk(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doMove, bool doForwardMove, bool doBackMove, 
		bool doLeftMove, bool doRightMove, bool rotated, bool *accelerated);

	// call if the unit is under direct player control (first person mode),
	// will return player's movement for the unit.
	void decideBasedOnLocalPlayerDirectControl(Unit *unit,
		bool *doMove, bool *doRotation, bool *doForwardMove, bool *doBackMove, 
		bool *doLeftMove, bool *doRightMove, 
		bool *doLeftRotation, bool *doRightRotation, 
		bool *doFire, float *rotationAngle);

	// direct control mode for AI
	void decideBasedOnAIDirectControl(Unit *unit,
		bool *doMove, bool *doRotation, bool *doForwardMove, bool *doBackMove, 
		bool *doLeftMove, bool *doRightMove, 
		bool *doLeftRotation, bool *doRightRotation, 
		bool *doFire, float *rotationAngle);

	// call if the unit is NOT under direct player control
	// will return the AI movement for the unit.
	void decideTurnAndWalk(Unit *unit, 
		bool *doMove, bool *doRotation, float *rotationAngle);

  // act jumping (call after decideTurnAndWalk)
	void actJump(Unit *unit, bool *doFire, bool *doMove, 
		bool *doForwardMove, bool *doBackMove, bool *doLeftMove, bool *doRightMove);

	// unconscious?
	void actPossiblyUnconscious(Unit *unit, UnitActAnimationRequests *animRequests);

	// idling?
	void actPossiblyIdle(Unit *unit, UnitActAnimationRequests *animRequests);

	// rising up from unconsciousness?
	void actPossiblyRising(Unit *unit, UnitActAnimationRequests *animRequests);

	// going prone or rising from prone?
	void actPossiblyProning(Unit *unit, UnitActAnimationRequests *animRequests);

	// staggering backward?
	void actPossiblyStagger(Unit *unit, UnitActAnimationRequests *animRequests);

	// electrified
	void actPossiblyElectrified(Unit *unit, UnitActAnimationRequests *animRequests);

	// stunned (temporarily paralyzed, but not unconscious)
	void actPossiblyStunned(Unit *unit, UnitActAnimationRequests *animRequests);

	// blown off by impact?
	void actPossiblyImpact(Unit *unit, UnitActAnimationRequests *animRequests);

	// finally apply the animations based on given requests
	void applyAnimations(Unit *unit, const UnitActAnimationRequests &animRequests);

	// internal, actual shooting implementation
	bool shoot(Unit *unit, const VC3 &target);

	// make a noise (gameplay noise, not actual sound)
	void actNoisy(Unit *unit);

	// make a stealthing unit go in stealth mode if necessary
	void actStealth(Unit *unit);

	// see if the unit is pointed...
	void actPointed(Unit *unit);

	// for squashable little bugs
	void actSquashable(Unit *unit);

	// for effect layer...
	void actEffects(Unit *unit);

	// act idle breathing
	void actOutdoorBreathingSteam( Unit* unit );

	// handle delayed projectiles bound to unit (delayed poison damage for one)
	void actDelayedProjectiles(Unit *unit);

	// for autoaiming...
	void getAutoAimNear(Unit *unit, const VC3 &target, Unit **autoAimed, bool *aimedVertical, bool *aimedHorizontal);

	VC3 getAimTargetPosition(Unit *unit);

	void actBoneAiming(Unit *unit, const VC3 &rotation, float destAngle);

	// rotation _not_ const
	void actDirectControlStrafing(Unit *unit, VC3 &rotation, float destAngle,
		bool doLeftMove, bool doRightMove, bool doForwardMove, bool doBackMove);

private:
	// internal collisions impl...
	void actCollisionsImpl(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition);

	void stopJumpBecauseCollided(Unit *unit);

	void handleRechargeableWeapons(Unit *unit, bool &primaryPressed, bool &secondaryPressed);

	void handlePointerWeapon(Unit *unit);
	void handleTargetLocker(Unit *unit);
};

}

#endif
