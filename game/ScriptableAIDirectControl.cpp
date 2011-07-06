
#include "precompiled.h"

#include "ScriptableAIDirectControl.h"
#include "Unit.h"
#include "Game.h"
#include "GameMap.h"
#include "GameRandom.h"
#include "scripting/GameScripting.h"

namespace game
{

static const char *eventMaskNames[ScriptableAIDirectControl::NUM_EVENT_MASK + 1] = 
{
	"tick",
	"timer",

	"fall",
	"touchdown",

	"static_touch_left",
	"static_touch_right",
	"static_touch_below",
	"static_touch_above",

	"object_touch_left",
	"object_touch_right",
	"object_touch_below",
	"object_touch_above",

	"drop_on_left",
	"drop_on_right",

	"unit_touch_left",
	"unit_touch_right",
	"unit_touch_below",
	"unit_touch_above",

	"***"
};


static const char *aimModeNames[ScriptableAIDirectControl::NUM_AIM_MODE + 1] = 
{
	"offset_self",
	"absolute",
	"offset_unit_target",
	"offset_target_unified_handle",

	"***"
};

ScriptableAIDirectControl::AIM_MODE ScriptableAIDirectControl::getAimModeByName(const char *name)
{
	if (name == NULL)
		return ScriptableAIDirectControl::AIM_MODE_INVALID;

	for (int i = 0; i < NUM_AIM_MODE; i++)
	{
		assert(strcmp(aimModeNames[i], "***") != 0);
		if (strcmp(aimModeNames[i], name) == 0)
		{
			return (ScriptableAIDirectControl::AIM_MODE)i;
		}
	}

	return ScriptableAIDirectControl::AIM_MODE_INVALID;
}

ScriptableAIDirectControl::EVENT_MASK ScriptableAIDirectControl::getEventMaskByName(const char *name)
{
	if (name == NULL)
		return ScriptableAIDirectControl::EVENT_MASK_INVALID;

	for (int i = 0; i < NUM_EVENT_MASK; i++)
	{
		assert(strcmp(eventMaskNames[i], "***") != 0);
		if (strcmp(eventMaskNames[i], name) == 0)
		{
			return (ScriptableAIDirectControl::EVENT_MASK)(1<<i);
		}
	}

	return ScriptableAIDirectControl::EVENT_MASK_INVALID;
}

ScriptableAIDirectControl::ScriptableAIDirectControl(Game *game, Unit *unit)
{
	this->game = game;
	this->unit = unit;

	this->eventMask = 0;

	this->previousEventsOnMask = 0;
	//this->actions = AIDirectControlActions(); // does not need init
	//this->actionsToDisableAutomatically = AIDirectControlActions(); // does not need init

	this->aimMode = AIM_MODE_OFFSET_SELF;
	this->aimPosition = VC3(0,0,0);
	this->aimPositionResult = VC3(0,0,0);

	this->timerTicksAmount = 1;
	this->lastTimerCall = 0;

	this->customEventsUsed = 0;
	for (int i = 0; i < SCRIPTABLEAIDIRECTCONTROL_MAX_CUSTOM_EVENTS; i++)
	{
		this->customEventsTicksLeft[i] = 0;
		this->customEventsName[i] = NULL;
	}
}



void ScriptableAIDirectControl::doDirectControls(AIDirectControlActions &actionsOut)
{	
	for (int i = 0; i < DIRECT_CTRL_AMOUNT; i++)
	{
		if (actionsToDisableAutomatically.directControlOn[i])
		{
			actions.directControlOn[i] = false;
		}
	}


	bool onGround = unit->isOnGround();
	// TODO: how does isOnPhysicsObject relate to this?

	bool dropOnLeft = false;
	bool dropOnRight = false;
	bool objectTouchLeft = false;
	bool objectTouchRight = false;
	bool staticTouchLeft = false;
	bool staticTouchRight = false;

	// optimization, do this only if event mask allows. (even though that will 
	// result into slightly erronous behaviour when event mask is changed while 
	// these conditions are true!)
	if (this->eventMask & (EVENT_MASK_DROP_ON_LEFT | EVENT_MASK_DROP_ON_RIGHT))
	{
		static const int dropCheckDist = 3;
		if (unit->obstacleX >= dropCheckDist && unit->obstacleX < game->gameMap->getObstacleSizeX() - dropCheckDist 
			&& unit->obstacleY >= dropCheckDist && unit->obstacleY < game->gameMap->getObstacleSizeY() - dropCheckDist)
		{
			assert(dropCheckDist >= 3);
			if (game->gameMap->getObstacleHeight(unit->obstacleX - dropCheckDist, unit->obstacleY - 3) == 0)
			{
				dropOnLeft = true;
			}
			if (game->gameMap->getObstacleHeight(unit->obstacleX + dropCheckDist, unit->obstacleY - 3) == 0)
			{
				dropOnRight = true;
			}
		}
	}
	if (this->eventMask & (EVENT_MASK_STATIC_TOUCH_LEFT | EVENT_MASK_STATIC_TOUCH_RIGHT))
	{
		// TODO: read from physics object feedback instead!

		static const int touchCheckDist = 3;
		if (unit->obstacleX >= touchCheckDist && unit->obstacleX < game->gameMap->getObstacleSizeX() - touchCheckDist 
			&& unit->obstacleY >= touchCheckDist && unit->obstacleY < game->gameMap->getObstacleSizeY() - touchCheckDist)
		{
			assert(touchCheckDist >= 3);
			if (game->gameMap->getObstacleHeight(unit->obstacleX - touchCheckDist, unit->obstacleY + 3) > 0)
			{
				if (!game->gameMap->isMovingObstacle(unit->obstacleX - touchCheckDist, unit->obstacleY + 3))
					staticTouchLeft = true;
			}
			if (game->gameMap->getObstacleHeight(unit->obstacleX + touchCheckDist, unit->obstacleY + 3) > 0)
			{
				if (!game->gameMap->isMovingObstacle(unit->obstacleX + touchCheckDist, unit->obstacleY + 3))
					staticTouchRight = true;
			}
		}
	}

	bool runTouchdown = false;
	bool runFall = false;
	bool runTimer = false;
	bool runDropOnLeft = false;
	bool runDropOnRight = false;
	bool runObjectTouchLeft = false;
	bool runObjectTouchRight = false;
	bool runStaticTouchLeft = false;
	bool runStaticTouchRight = false;

	// compare state to previous events on, and call only ones that have changed.
	// this must be done before changing the previousEventsOnMask
	if (game->gameTimer >= lastTimerCall + timerTicksAmount)
	{
		runTimer = true;
	}

	if (onGround && (previousEventsOnMask & EVENT_MASK_TOUCHDOWN) == 0)
	{
		runTouchdown = true;
	}
	else if (!onGround && (previousEventsOnMask & EVENT_MASK_FALL) == 0)
	{
		runFall = true;
	}

	if (dropOnLeft && (previousEventsOnMask & EVENT_MASK_DROP_ON_LEFT) == 0)
	{
		runDropOnLeft = true;
	}
	if (dropOnRight && (previousEventsOnMask & EVENT_MASK_DROP_ON_RIGHT) == 0)
	{
		runDropOnRight = true;
	}

	if (objectTouchLeft && (previousEventsOnMask & EVENT_MASK_OBJECT_TOUCH_LEFT) == 0)
	{
		runObjectTouchLeft = true;
	}
	if (objectTouchRight && (previousEventsOnMask & EVENT_MASK_OBJECT_TOUCH_RIGHT) == 0)
	{
		runObjectTouchRight = true;
	}

	if (staticTouchLeft && (previousEventsOnMask & EVENT_MASK_STATIC_TOUCH_LEFT) == 0)
	{
		runStaticTouchLeft = true;
	}
	if (staticTouchRight && (previousEventsOnMask & EVENT_MASK_STATIC_TOUCH_RIGHT) == 0)
	{
		runStaticTouchRight = true;
	}


	// update previousEventsOnMask based on state now that changes were first solved
	if (runTimer)
	{
		this->lastTimerCall = game->gameTimer;
	}
	if (onGround)
	{
		// we're on ground
		previousEventsOnMask |= EVENT_MASK_TOUCHDOWN;
		previousEventsOnMask &= ~EVENT_MASK_FALL;
	} else {
		// we're falling
		previousEventsOnMask |= EVENT_MASK_FALL;
		previousEventsOnMask &= ~EVENT_MASK_TOUCHDOWN;
	}
	if (dropOnLeft)
	{
		previousEventsOnMask |= EVENT_MASK_DROP_ON_LEFT;
	} else {
		previousEventsOnMask &= ~EVENT_MASK_DROP_ON_LEFT;
	}
	if (dropOnRight)
	{
		previousEventsOnMask |= EVENT_MASK_DROP_ON_RIGHT;
	} else {
		previousEventsOnMask &= ~EVENT_MASK_DROP_ON_RIGHT;
	}
	if (objectTouchLeft)
	{
		previousEventsOnMask |= EVENT_MASK_OBJECT_TOUCH_LEFT;
	} else {
		previousEventsOnMask &= ~EVENT_MASK_OBJECT_TOUCH_LEFT;
	}
	if (objectTouchRight)
	{
		previousEventsOnMask |= EVENT_MASK_OBJECT_TOUCH_RIGHT;
	} else {
		previousEventsOnMask &= ~EVENT_MASK_OBJECT_TOUCH_RIGHT;
	}


	// and finally run the actual event scripts if event mask allows
	if ((eventMask & EVENT_MASK_TICK) != 0)
	{
		// NOTICE: enabling tick event is pretty much overkill in most cases!
		game->gameScripting->runEventScript(unit, "event_tick");
	}
	if (runTimer && (eventMask & EVENT_MASK_TIMER) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_timer");
	}
	if (runTouchdown && (eventMask & EVENT_MASK_TOUCHDOWN) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_touchdown");
	}
	if (runFall && (eventMask & EVENT_MASK_FALL) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_fall");
	}
	if (runDropOnLeft && (eventMask & EVENT_MASK_DROP_ON_LEFT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_drop_on_left");
	}
	if (runDropOnRight && (eventMask & EVENT_MASK_DROP_ON_RIGHT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_drop_on_right");
	}
	if (runStaticTouchLeft && (eventMask & EVENT_MASK_STATIC_TOUCH_LEFT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_static_touch_left");
	}
	if (runStaticTouchRight && (eventMask & EVENT_MASK_STATIC_TOUCH_RIGHT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_static_touch_right");
	}
	if (runObjectTouchLeft && (eventMask & EVENT_MASK_OBJECT_TOUCH_LEFT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_object_touch_left");
	}
	if (runObjectTouchRight && (eventMask & EVENT_MASK_OBJECT_TOUCH_RIGHT) != 0)
	{
		game->gameScripting->runEventScript(unit, "event_object_touch_right");
	}

	// custom events
	// NOTE: custom event time is based on unit ticks!
	// (whereas timer event time is based on actual game tick - even when unit is not acting)
	if (this->customEventsUsed > 0)
	{
		for (int ceve = 0; ceve < this->customEventsUsed; ceve++)
		{
			if (this->customEventsTicksLeft[ceve] > 0)
			{
				this->customEventsTicksLeft[ceve]--;
				if (this->customEventsTicksLeft[ceve] == 0)
				{
					// run custom event...
					if (this->customEventsName[ceve] != NULL)
					{
						game->gameScripting->runEventScript(unit, this->customEventsName[ceve]);
						delete [] this->customEventsName[ceve];
						this->customEventsName[ceve] = NULL;

						// if this was the last event, compact the customEventsUsed value...
						if (ceve == customEventsUsed - 1)
						{
							while (customEventsUsed > 0
								&& this->customEventsTicksLeft[customEventsUsed - 1] == 0)
							{
								customEventsUsed--;
							}
						}
					}
				}
			}
		}
	}


	// handle aim position based on the mode.

	if (this->aimMode == AIM_MODE_OFFSET_SELF)
	{
		aimPositionResult = unit->getPosition() + aimPosition;
	}
	else if (this->aimMode == AIM_MODE_ABSOLUTE)
	{
		aimPositionResult = aimPosition;
	}
	else if (this->aimMode == AIM_MODE_OFFSET_UNIT_TARGET)
	{
		if (unit->targeting.hasTarget())
		{
			if (unit->targeting.getTargetUnit() != NULL)
			{
				aimPositionResult = unit->targeting.getTargetUnit()->getPosition() + aimPosition;
			} else {
				aimPositionResult = unit->targeting.getTargetPosition() + aimPosition;
			}
		} else {
			// lost target, what should we do in this case?
			// for now, just not updating the aim position result at all.
		}
	}
	else if (this->aimMode == AIM_MODE_OFFSET_TARGET_UNIFIED_HANDLE)
	{
		assert(!"TODO: AIM_MODE_OFFSET_TARGET_UNIFIED_HANDLE");
		aimPositionResult = aimPosition;
	} else {
		assert(!"ScriptableAIDirectControl::doDirectControls - Unsupported aim mode.");
	}
	actions.aimPosition = aimPositionResult;

	actionsOut = actions;

	// (above should be array value assignment, not array pointer assignment)
	assert(actionsOut.directControlOn != actions.directControlOn);


}


void ScriptableAIDirectControl::enableEvent(EVENT_MASK eventMask)
{
	this->eventMask |= (int)eventMask;
}


void ScriptableAIDirectControl::disableEvent(EVENT_MASK eventMask)
{
	this->eventMask &= ~(int)eventMask;
}


void ScriptableAIDirectControl::clearEventFlag(EVENT_MASK eventMask)
{
	this->previousEventsOnMask &= ~((int)eventMask);
}


void ScriptableAIDirectControl::setTimerEventParameters(int ticksPerTimerEvent)
{
	this->timerTicksAmount = ticksPerTimerEvent;
}


void ScriptableAIDirectControl::enableAction(int directCtrl)
{
	assert(directCtrl >= 0 && directCtrl < DIRECT_CTRL_AMOUNT);
	this->actions.directControlOn[directCtrl] = true;
}


void ScriptableAIDirectControl::disableAction(int directCtrl)
{
	assert(directCtrl >= 0 && directCtrl < DIRECT_CTRL_AMOUNT);
	this->actions.directControlOn[directCtrl] = false;
}


void ScriptableAIDirectControl::setAimMode(AIM_MODE aimMode)
{
	this->aimMode = aimMode;
}


void ScriptableAIDirectControl::setAimPosition(const VC3 &position)
{
	this->aimPosition = position;
}


void ScriptableAIDirectControl::setActionsToDisableAutomatically(const AIDirectControlActions &actionsToDisable)
{
	this->actionsToDisableAutomatically = actionsToDisable;
}

void ScriptableAIDirectControl::addActionToDisableAutomatically(int directControl)
{
	assert(directControl >= 0 && directControl < DIRECT_CTRL_AMOUNT);
	this->actionsToDisableAutomatically.directControlOn[directControl] = true;
}

void ScriptableAIDirectControl::removeActionToDisableAutomatically(int directControl)
{
	assert(directControl >= 0 && directControl < DIRECT_CTRL_AMOUNT);
	this->actionsToDisableAutomatically.directControlOn[directControl] = false;
}

void ScriptableAIDirectControl::addCustomEvent(int afterTicks, const char *customEventName)
{
	if (customEventName == NULL)
	{
		LOG_ERROR("ScriptableAIDirectControl::addCustomEvent - Attempt to add a custom event with null name.");
		return;
	}
	if (afterTicks <= 0)
	{
		LOG_ERROR("ScriptableAIDirectControl::addCustomEvent - Attempt to add a custom event with zero or less tick delay.");
		return;
	}

	bool foundSlot = false;
	int addToSlot = -1;
	if (this->customEventsUsed < SCRIPTABLEAIDIRECTCONTROL_MAX_CUSTOM_EVENTS)
	{
		addToSlot = this->customEventsUsed;
		foundSlot = true;
	}

	if (this->customEventsUsed > 0)
	{
		for (int ceve = 0; ceve < this->customEventsUsed; ceve++)
		{
			if (this->customEventsTicksLeft[ceve] == 0)
			{
				foundSlot = true;
				addToSlot = ceve;
				break;
			}
		}
	} else {
		foundSlot = true;
		assert(addToSlot == 0);
	}

	if (foundSlot)
	{
		assert(addToSlot >= 0 && addToSlot < SCRIPTABLEAIDIRECTCONTROL_MAX_CUSTOM_EVENTS);

		int slen = strlen(customEventName);

		this->customEventsTicksLeft[addToSlot] = afterTicks;
		this->customEventsName[addToSlot] = new char[slen + 1];
		strcpy(this->customEventsName[addToSlot], customEventName);

		if (this->customEventsUsed < addToSlot + 1)
			this->customEventsUsed = addToSlot + 1;
	} else {
		LOG_WARNING_W_DEBUG("ScriptableAIDirectControl::addCustomEvent - Failed to add custom event, maximum amount reached.", customEventName);
	}
}


}

