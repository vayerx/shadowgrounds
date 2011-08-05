
#include "precompiled.h"

#include "UnitLevelAI.h"
#include "../system/Logger.h"
#include "../container/LinkedList.h"
#include "../game/UnitActor.h"
#include "../game/UnitType.h"
#include "../game/unittypes.h"
#include "../game/Part.h"
#include "../game/Unit.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameCollisionInfo.h"
#include "../game/scripting/GameScripting.h"
#include "../game/GameRandom.h"
#include "../util/ScriptProcess.h"

#include "../util/Debug_MemoryManager.h"

#define SPOTTED_SCRIPT_DELAY (5000 / GAME_TICK_MSEC)
// 5 sec delay before running the spotted script again
#define HIT_SCRIPT_DELAY (5000 / GAME_TICK_MSEC)
// 5 sec delay before running the hit script again
#define HITMISS_SCRIPT_DELAY (5000 / GAME_TICK_MSEC)
// 5 sec delay before running the hitmiss script again
#define HEARNOISE_SCRIPT_DELAY (10000 / GAME_TICK_MSEC)
// 10 sec delay before running the hearnoise script again
#define POINTED_SCRIPT_DELAY (500 / GAME_TICK_MSEC)
// 0.5 sec delay before running the pointed script again

namespace game
{

bool UnitLevelAI::allEnabled = true;
bool UnitLevelAI::playerAIEnabled[ABS_MAX_PLAYERS] = 
{ 
	true, true, true, true
//	true, true, true, true, true, true, true, true, 
//	true, true, true, true, true, true, true, true
};

UnitLevelAI::UnitLevelAI(game::Game *game, Unit *unit)
{
  this->game = game;
  this->unit = unit;
  this->mainScriptProcess = NULL;
	this->enabled = true;
	this->tempDisabled = false;
	this->eventMask = 0;
	this->lastSpeed = Unit::UNIT_SPEED_FAST;
	this->force_hitscript_enabled = false;
	rescriptRequested = false;
  reScriptMain();
  //this->alertedScriptProcess = NULL;
}

UnitLevelAI::~UnitLevelAI()
{
	terminateMainScript();
}

void UnitLevelAI::addEventListener(int eventMask)
{
	this->eventMask |= eventMask;
}

void UnitLevelAI::setAllEnabled(bool allEnabled)
{
	UnitLevelAI::allEnabled = allEnabled;
}

void UnitLevelAI::setPlayerAIEnabled(int player, bool enabled)
{
	// TODO: should possibly loop thru all units and set their 
	// individual ai enabled/disabled (but that might screw up some
	// specifically disabled unit AIs?)
	UnitLevelAI::playerAIEnabled[player] = enabled;
}

void UnitLevelAI::reScriptMain()
{
	terminateMainScript();

  if (unit->getScript() != NULL)
  {
    mainScriptProcess = 
      game->gameScripting->startUnitScript(unit, unit->getScript(), "main");
		if (mainScriptProcess != NULL)
		{
	    assert(mainScriptProcess->getData() != NULL);
		}
  }  
}


void UnitLevelAI::prepareMainScript()
{
  // run the main script until first pausing command encountered...
  if (mainScriptProcess != NULL)
  {
    if (!mainScriptProcess->isFinished())
    {
			if (isThisAndAllEnabled())
			{
				game->gameScripting->runScriptProcess(mainScriptProcess, true);
			}
    }
  }
}


void UnitLevelAI::skipMainScriptWait()
{
  if (mainScriptProcess != NULL)
  {
    if (!mainScriptProcess->isFinished())
    {
			// WARNING: unsafe cast
			GameScriptData *gsd = (GameScriptData *)mainScriptProcess->getData();
			gsd->waitCounter = 0;
			gsd->waitDestination = false;
			gsd->waitCinematicScreen = false;
    }
  }
}

void UnitLevelAI::requestReScriptMain()
{
	this->rescriptRequested = true;
}

void UnitLevelAI::terminateMainScript()
{
  if (mainScriptProcess != NULL)
  {
    GameScriptData *gsd = (GameScriptData *)mainScriptProcess->getData();
    if (gsd != NULL) 
    {
      delete gsd;
    }
    delete mainScriptProcess;
		this->mainScriptProcess = NULL;
  }
}


void UnitLevelAI::runUnitAI()
{
  if (mainScriptProcess != NULL)
  {
    if (!mainScriptProcess->isFinished())
    {
			if (isThisAndAllEnabled())
			{
	      game->gameScripting->runScriptProcess(mainScriptProcess, true);
				GameScriptData *gsd = (GameScriptData *)mainScriptProcess->getData();
				if (mainScriptProcess->isFinished())
				{
					if (!mainScriptProcess->isUserStackEmpty())
					{
						mainScriptProcess->warning("UnitLevelAI::runUnitAI - Script process user stack not empty when script finished.");
						//mainScriptProcess->warning("UnitLevelAI::runUnitAI - Script process user stack not empty at pause.");
						//assert(!"Script process user stack not empty at pause.");
					}
					if (gsd->originalUnit != gsd->unit)
					{
						//mainScriptProcess->error("UnitLevelAI::runUnitAI - Script process unit was not restored to original at main script.");
						mainScriptProcess->error("UnitLevelAI::runUnitAI - Script process unit was not restored to original at main script.");
						//assert(!"Script process unit was not restored to original at main script.");
					}
				}
			}
    } else {
			if (rescriptRequested)
			{
				rescriptRequested = false;
				reScriptMain();
			}
		}
  }

	// hit script sub?
  if (unit->getHitScriptDelay() > 0)
  {
    // if we have no sight to enemies, clear the delay immediately.
    if (unit->getSeeUnit() == NULL)
      unit->setHitScriptDelay(0);
    else
      unit->setHitScriptDelay(unit->getHitScriptDelay() - 1);
		if (unit->getHitScriptDelay() == 0)
		{
			unit->setHitByUnit(NULL, NULL);
		}
  } else {
		if (unit->getHitByUnit() != NULL
			// really bad hack to allow hitscript from terrain object explosions
			|| (unit->getHitByBullet() != NULL && unit->getUnitType()->doesRunHitScriptWithoutShooter()))
    {
			Unit *tmpUnit = unit->getHitByUnit();
			Bullet *tmpBullet = unit->getHitByBullet();
      //if (game->isHostile(unit->getOwner(), unit->getHitByUnit()->getOwner()))
      //{
				// FIXME: should check that...
				// if (enabled || unit->isDestroyed()) ???
        unit->setHitScriptDelay(HIT_SCRIPT_DELAY);
				unit->setHitByUnit(NULL, NULL);
				if (isThisAndAllEnabled() || force_hitscript_enabled)
				{
					game->gameScripting->runHitScript(unit, tmpUnit, tmpBullet);
				}
      //}
    }
  }

	// hitmiss script sub?
  if (unit->getHitMissScriptDelay() > 0)
  {
    unit->setHitMissScriptDelay(unit->getHitMissScriptDelay() - 1);
		if (unit->getHitMissScriptDelay() == 0)
			unit->setHitMissByUnit(NULL);
  } else {
    if (unit->getHitMissByUnit() != NULL)
    {
			Unit *tmpUnit = unit->getHitMissByUnit();
      unit->setHitMissScriptDelay(HITMISS_SCRIPT_DELAY);
      unit->setHitMissByUnit(NULL);
			if (isThisAndAllEnabled())
			{
				game->gameScripting->runHitMissScript(unit, tmpUnit);
			}
    }
  }

	// hearnoise script sub?
  if (unit->getHearNoiseScriptDelay() > 0)
  {
    unit->setHearNoiseScriptDelay(unit->getHearNoiseScriptDelay() - 1);
		if (unit->getHearNoiseScriptDelay() == 0)
			unit->setHearNoiseByUnit(NULL);
  } else {
    if (unit->getHearNoiseByUnit() != NULL)
    {
			Unit *tmpUnit = unit->getHearNoiseByUnit();
      unit->setHearNoiseScriptDelay(HEARNOISE_SCRIPT_DELAY);
      unit->setHearNoiseByUnit(NULL);
			if (isThisAndAllEnabled())
			{
	      game->gameScripting->runHearNoiseScript(unit, tmpUnit);
			}
    }
  }

	// pointed script sub?
  if (unit->getPointedScriptDelay() > 0)
  {
    unit->setPointedScriptDelay(unit->getPointedScriptDelay() - 1);
		if (unit->getPointedScriptDelay() == 0)
			unit->setPointedByUnit(NULL);
  } else {
    if (unit->getPointedByUnit() != NULL)
    {
			Unit *tmpUnit = unit->getPointedByUnit();
      unit->setPointedScriptDelay(POINTED_SCRIPT_DELAY);
      unit->setPointedByUnit(NULL);
			if (isThisAndAllEnabled())
			{
				game->gameScripting->runPointedScript(unit, tmpUnit);
			}
    }
  }

	// spotted script sub?
  if (unit->getSpottedScriptDelay() > 0)
  {
    // if we have no sight to enemies, clear the delay immediately.
    if (unit->getSeeUnit() == NULL)
      unit->setSpottedScriptDelay(0);
    else
      unit->setSpottedScriptDelay(unit->getSpottedScriptDelay() - 1);
  } else {
    if (unit->getSeeUnit() != NULL)
    {
			Unit *tmpUnit = unit->getSeeUnit();
      unit->setSpottedScriptDelay(SPOTTED_SCRIPT_DELAY);
			if (isThisAndAllEnabled())
			{
	      game->gameScripting->runSpottedScript(unit, tmpUnit);
			}
    }
  }

	// special event subs...

	if (this->eventMask)
	{
		// fire complete script sub?
		if (this->eventMask & UNITLEVELAI_EVENT_MASK_FIRE_COMPLETE)
		{
			// NOTE: not a very nice solution - does not give the accurate
			// time of fire complete (1 tick before that instead)
			// NOTE: thus may not work properly if reload time <= 1

			if (unit->getSelectedWeapon() != -1)
			{
				if (unit->getFireReloadDelay(unit->getSelectedWeapon()) == 1)
				{
					game->gameScripting->runEventScript(unit, "event_fire_complete");
				}
			}
		}

		// jump start/end script sub
		if (this->eventMask & (UNITLEVELAI_EVENT_MASK_JUMP_START | UNITLEVELAI_EVENT_MASK_JUMP_END))
		{
			if (unit->getSpeed() != this->lastSpeed)
			{
				if ((this->eventMask & UNITLEVELAI_EVENT_MASK_JUMP_START) != 0
					&& unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
				{
					game->gameScripting->runEventScript(unit, "event_jump_start");
				}
				else if ((this->eventMask & UNITLEVELAI_EVENT_MASK_JUMP_END) != 0
					&& this->lastSpeed == Unit::UNIT_SPEED_JUMP)
				{
					game->gameScripting->runEventScript(unit, "event_jump_end");
				}
				this->lastSpeed = unit->getSpeed();
			}
		}
	}

	// lost target out of sight?
  if (unit->targeting.hasTarget())
  {
		if (unit->targeting.getTargetUnit() != NULL)
		{
			if (!unit->targeting.getTargetUnit()->visibility.isSeenByPlayer(unit->getOwner()))
			{
				// HACK: computer does not lose the target immediately
				if (unit->getOwner() == game->singlePlayerNumber
					|| (game->gameTimer & 63) == 0)
				{
					if (!unit->isGhostOfFuture())
					{
						unit->targeting.clearTargetUsingCounter();
						game->gameUI->setPointersChangedFlag(unit->getOwner());
					}
				}
			}
		}
	}

  if (unit->getMode() != Unit::UNIT_MODE_KEEP_TARGET)
	{
		if (unit->getMode() != Unit::UNIT_MODE_HOLD_FIRE)
		{
			if (unit->targeting.hasTarget())
			{
				// TODO... something wise here?
				if (unit->targeting.getTargetUnit() != NULL)
				{
					if (unit->getSeeUnit() != NULL
						&& unit->getSeeUnit() != unit->targeting.getTargetUnit()
						&& unit->targeting.isAutoTarget())
					{
						VC3 distVector = unit->targeting.getTargetUnit()->getPosition() 
							- unit->getPosition();
						// (2*see_unit_dist)^2 < (target_dist)^2
						if (unit->getSeeUnitDistance() * unit->getSeeUnitDistance() * 4
							< distVector.GetSquareLength()
							|| (unit->getSeeUnit()->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& unit->targeting.getTargetUnit()->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS))
						{
							// choose a more near by hostile unit (dist less than half)
							if (isThisAndAllEnabled())
							{
								if (!unit->isGhostOfFuture())
									unit->targeting.setAutoTarget(unit->getSeeUnit());
							}
						}
					}
				}
			} else {
				if (unit->getSeeUnit() != NULL)
				{
					float maxAttackRange = unit->getMaxWeaponRange() / 3;
					if (unit->getMode() == Unit::UNIT_MODE_AGGRESSIVE)
						maxAttackRange = unit->getMaxWeaponRange();

					// TODO: should rather react to actual shot than 
					// the targeting...
					if ((unit->getSeeUnitDistance() < maxAttackRange
						|| unit->getSeeUnit()->targeting.hasTarget())
						&& (unit->getSeeUnit()->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS 
						|| unit->getSeeUnitDistance() < 5))
					{
						if (isThisAndAllEnabled())
						{
							if (!unit->isDirectControl())
							{
								if (!unit->getUnitType()->hasOnlyOneWeaponSelected())
								{
									unit->setWeaponsActiveByFiretype(Unit::FireTypeBasic);
								}
								if (!unit->isGhostOfFuture())
									unit->targeting.setAutoTarget(unit->getSeeUnit());
							}
						}
					}
				}
			}
		} else {
			if (unit->targeting.hasTarget())
			{
				if (unit->targeting.isAutoTarget())
				{
					if (!unit->isGhostOfFuture())
						unit->targeting.clearTarget();
				}
			}
		}
	}
}

void UnitLevelAI::setEnabled(bool aiEnabled, bool force_hitscript_enabled)
{
	this->enabled = aiEnabled;
	this->force_hitscript_enabled = force_hitscript_enabled;
}

bool UnitLevelAI::isThisAndAllEnabled()
{
	if (this->enabled && !this->tempDisabled && allEnabled
		&& (this->unit->getOwner() != NO_UNIT_OWNER
		&& UnitLevelAI::playerAIEnabled[this->unit->getOwner()]))
	{
		return true;
	} else {
		return false;
	}
}

void UnitLevelAI::copyStateFrom(UnitLevelAI *otherAI)
{
	if (otherAI->mainScriptProcess != NULL)
	{
		this->mainScriptProcess->copyFrom(otherAI->mainScriptProcess);
		// TODO: should copy some data from gsd too.
	} else {
		if (mainScriptProcess != NULL)
		{
			GameScriptData *gsd = (GameScriptData *)mainScriptProcess->getData();
			if (gsd != NULL) 
			{
				delete gsd;
			}
			delete mainScriptProcess;
			this->mainScriptProcess = NULL;
		}		
	}
}

bool UnitLevelAI::isScriptProcessMainScriptProcess(util::ScriptProcess *sp)
{
	return (this->mainScriptProcess == sp);
}

void UnitLevelAI::setTempDisabled(bool tempDisabled)
{
	this->tempDisabled = tempDisabled;
}


} // end namespace game

