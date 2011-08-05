
#ifndef UNITTARGETING_H
#define UNITTARGETING_H

#include <DatatypeDef.h>

namespace game
{
  class Unit;

  /**
   *
   * Unit's targeting.
   * Contains the data needed by a unit maintain targets.
   * Moved here from the actual Unit class to make the code easier
   * to read and maintain.
   *
   * @version 1.0, 9.12.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see Unit
   *
   */
  class UnitTargeting
  {
  public:
    /**
     * Default constructor.
     */
    UnitTargeting();

    /**
     * Destructor (does not really do anything, thus commented out)
     */
    //~UnitTargeting();

		void setTargetLoseTime(int targetLoseTime);

    /**
     * Sets a target world position for the unit.
     * Used to give ground targets for unit.
     * @param target VC3&, the position of the target.
     * Target coordinates unit (x, y, z) = map (x, height, y).
     */
    void setTarget(VC3 &target);

    /**
     * Sets a target unit for the unit.
     * Used to give enemy unit targets for the unit.
     * @param targetUnit Unit*, the unit to be targeted.
     */
    void setTarget(Unit *targetUnit); // targetUnit can't be const

    /**
     * Sets a target, should be used by unit level ai.
     * @param targetUnit Unit*, the unit to be targeted.
     */
    void setAutoTarget(Unit *targetUnit);

    /**
     * Clears any targets this unit has.
     */
    void clearTarget();

		/**
		 * Increases target lose counter and clears target if counter reaches maximum
		 */
    void clearTargetUsingCounter();

    /**
     * Returns true if the unit has a target.
     * @return bool, true if the unit has a target.
     */
    inline bool hasTarget() const
    {
      return targetSet;
    }

    /**
     * Returns true if the unit's target was aquired by unit level ai.
     * @return bool, true if ai selected this target.
     */
    bool isAutoTarget();

    /**
     * Returns the target unit of this unit.
     * @return Unit*, pointer to target unit or NULL if no unit targeted.
     *         can't return const
     */
    Unit *getTargetUnit() const;

    /**
     * Returns the target position of this unit.
     * @return VC3, position of the target for this unit.
     * May be the position of the targeted unit or just ground coordinates.
     * Use hasTarget and getTargetUnit to check which it is.
     * Result of this call is undefined if hasTarget returns false.
     */
    VC3 getTargetPosition();

    bool hasLineOfFireToTarget();

    void setLineOfFireToTarget(bool lineOfFire, int loseTimeOut, int currentTime);

    bool hasLastTargetPosition();

    VC3 getLastTargetPosition();

    void clearLastTargetPosition();

    VC3 getAimingPosition();

    void setAimingPosition(const VC3 &aimingPosition);

    VC3 getSweepTargetPosition();
    void setSweepTargetPosition(const VC3 &sweepPosition);
    void clearSweepTargetPosition();
    bool hasSweepTargetPosition();

		int getFireSweepsSinceTarget();
		void setFireSweepsSinceTarget(int sweepsSinceTarget);

  private:
    VC3 targetPosition;
	Unit *targetUnit; // can't be const
    bool groundTarget;
    bool targetSet;
    bool autoTarget;  // if target was acquired by unit level ai

    VC3 lastTargetPosition;
    bool lastTargetPositionExists;

    bool lineOfFireToTarget;

		VC3 aimingPosition;
		VC3 sweepTarget;
		int sweepsSinceTarget;

		int targetLoseCounter;

		int targetLoseTime;
		int lofLoseStartedTime;

    friend class Unit;
  };
}

#endif

