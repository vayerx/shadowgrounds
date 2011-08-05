
#include "precompiled.h"

#include "UnitTargeting.h"
#include "Unit.h"

// TEMP: for debugging
#include "../system/Logger.h"
#include "../convert/str2int.h"

// this should result into about 8 seconds (slightly less).
// (note: this value is not seconds though!!!) 
// NOTE: default value for this found in unittype now...
//#define DEFAULT_UNITTARGETING_TARGET_LOSE_MAX_COUNT 8


namespace game
{

  UnitTargeting::UnitTargeting()
  {
    targetSet = false;
    targetPosition = VC3(0, 0, 0); 
    targetUnit = NULL;
    autoTarget = false;
    groundTarget = false;
    lineOfFireToTarget = false;
    lastTargetPosition = VC3(0,0,0);
    lastTargetPositionExists = false;
		aimingPosition = VC3(0,0,0);
		sweepTarget = VC3(0,0,0);
		sweepsSinceTarget = 0;
		targetLoseCounter = 0;
		targetLoseTime = 0;
		lofLoseStartedTime = 0;
  }

	void UnitTargeting::setTargetLoseTime(int targetLoseTime)
	{
		this->targetLoseTime = targetLoseTime;
	}


  /*
  UnitTargeting::~UnitTargeting()
  {
    // nop
  }
  */


  bool UnitTargeting::isAutoTarget()
  {
    return autoTarget;
  }

  
  void UnitTargeting::setAutoTarget(Unit *targetUnit)
  {
    setTarget(targetUnit);
    autoTarget = true;
  }


  void UnitTargeting::setTarget(VC3 &target)
  {
    this->targetUnit = NULL;
    this->targetPosition = target;
    this->lastTargetPosition = target;
    this->lastTargetPositionExists = true;
    targetSet = true;
    autoTarget = false;
    groundTarget = true;
		targetLoseCounter = 0;
  }


  void UnitTargeting::setTarget(Unit *targetUnit)
  {
    this->targetUnit = targetUnit;
		if(targetUnit != NULL)
		{
	    this->targetPosition = targetUnit->getPosition();
		}
    this->lastTargetPosition = targetPosition;
    this->lastTargetPositionExists = true;
    targetSet = true;
    autoTarget = false;
    groundTarget = false;
		targetLoseCounter = 0;
  }


  void UnitTargeting::clearTarget()
  {
    if (targetUnit != NULL)
    {
      lastTargetPosition = targetUnit->getPosition();
    }
    targetSet = false;
    groundTarget = false;
    autoTarget = false;
    targetUnit = NULL;
		sweepsSinceTarget = 0;
		targetLoseCounter = 0;
  }


  void UnitTargeting::clearTargetUsingCounter()
  {
		targetLoseCounter++;
		//if (targetLoseCounter >= UNITTARGETING_TARGET_LOSE_MAX_COUNT)
		if (targetLoseCounter >= targetLoseTime)
		{
			clearTarget();
		}
  }


  void UnitTargeting::clearLastTargetPosition()
  { 
    lastTargetPositionExists = false;
  }


  bool UnitTargeting::hasLastTargetPosition()
  {
    return lastTargetPositionExists;
  }


  VC3 UnitTargeting::getLastTargetPosition()
  {
    return lastTargetPosition;
  }


  /*
  bool UnitTargeting::hasTarget()
  {
    if (targetUnit != NULL)
    {
      return true;
    } else {
      if (groundTarget)
      {
        return true;
      } else {
        return false;
      }
    }
  }
  */


  VC3 UnitTargeting::getTargetPosition()
  {
		// NEW: returns the unit position always correctly, not just the original 
		// position where the target was set
		if (targetUnit != NULL)
			return targetUnit->getPosition();

    return targetPosition;
  }


  Unit *UnitTargeting::getTargetUnit() const
  {
    return targetUnit;
  }


  bool UnitTargeting::hasLineOfFireToTarget()
  {
    return lineOfFireToTarget;
  }


  void UnitTargeting::setLineOfFireToTarget(bool lineOfFire, int loseTimeOut, int currentTime)
  {
		// NEW: if unit has some lof lose time, handle that...
		if (loseTimeOut > 0)
		{
			if (!lineOfFire)
			{
				if (currentTime < lofLoseStartedTime + loseTimeOut)
				{
					// do nothing.
					return;
				}
			} else {
				// TODO: this is a bit incorrect, as this is the last successful lof check time
				// not the actual first non-successful lof check time
				lofLoseStartedTime = currentTime;
			}
		}
    lineOfFireToTarget = lineOfFire;
  }


  VC3 UnitTargeting::getAimingPosition()
	{
		return aimingPosition;
	}

  void UnitTargeting::setAimingPosition(const VC3 &aimingPosition)
	{
		this->aimingPosition = aimingPosition;
	}

	VC3 UnitTargeting::getSweepTargetPosition()
	{
		return sweepTarget;
	}

	void UnitTargeting::setSweepTargetPosition(const VC3 &sweepPosition)
	{
		this->sweepTarget = sweepPosition;
		this->sweepsSinceTarget++;
	}

	void UnitTargeting::clearSweepTargetPosition()
	{
		this->sweepTarget = VC3(0,0,0);
	}

  bool UnitTargeting::hasSweepTargetPosition()
	{
		if (sweepTarget.x != 0.0f
			|| sweepTarget.y != 0.0f
			|| sweepTarget.z != 0.0f)
			return true;
		else
			return false;
	}

	int UnitTargeting::getFireSweepsSinceTarget()
	{
		return sweepsSinceTarget;
	}

	void UnitTargeting::setFireSweepsSinceTarget(int sweepsSinceTarget)
	{
		// SHOULD NOT USE THIS DIRECTLY, HANDLED INTERNALLY?
		this->sweepsSinceTarget = sweepsSinceTarget;
	}


}

