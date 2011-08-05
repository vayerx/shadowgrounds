
#include "precompiled.h"

#include "GameScripting.h"

// ---------
// GAMESCRIPTING COMMAND AMOUNT

#include "gamescripting_amount.h"

// ---------

#ifndef COMBINE
#include "scriptcommands.h"
#endif

#include "GameScriptingUtils.h"

#include "DecorScripting.h"
#include "CameraScripting.h"
#include "SoundScripting.h"
#include "CinematicScripting.h"
#include "MissionScripting.h"
#include "MathScripting.h"
#include "StringScripting.h"
#include "PositionScripting.h"
#include "MiscScripting.h"
#include "DevScripting.h"
#include "UnitScripting.h"
#include "WaterScripting.h"
#include "OptionScripting.h"
#include "LightScripting.h"
#include "AnimationScripting.h"
#include "EnvironmentScripting.h"
#include "HitChainScripting.h"
#include "MapScripting.h"
#include "ItemScripting.h"
#include "TrackingScripting.h"
#include "SyncScripting.h"
#include "DirectControlScripting.h"
#include "../../util/fb_assert.h"

#include "../Projectile.h"

// FIXME: I DON'T LIKE THIS, CYCLIC DEPENDENCY..
#include "../UnitLevelAI.h"

//#include <assert.h>
#include <algorithm>
#include "../../container/LinkedList.h"
#include "../Game.h"
#include "../GameScene.h"
#include "../GameUI.h"
#include "../GameRandom.h"
#include "../Part.h"
#include "../Item.h"
#include "../ItemManager.h"
#include "../ItemType.h"
#include "../PartType.h"
#include "../Unit.h"
#include "../UnitType.h"
#include "../UnitList.h"
#include "../../convert/str2int.h"

#include "../../system/Logger.h"

#include "../../util/ClippedCircle.h"
#include "../../util/CircleAreaTracker.h"
#include "../../util/ITrackable.h"

#include "../../util/AI_PathFind.h"
#include "../../util/ScriptManager.h"
#include "../../util/ScriptProcess.h"
#include "../../util/AngleRotationCalculator.h"

#include "../tracking/ITrackerObject.h"
#include "../tracking/ScriptableTrackerObject.h"
#include "../tracking/ScriptableTrackerObjectType.h"
#include "../tracking/ObjectTracker.h"


#include "../../util/Debug_MemoryManager.h"


#define ALERTED_SUB_FAIL_LIMIT 100
#define HIT_SUB_FAIL_LIMIT 100
#define EXECUTE_SUB_FAIL_LIMIT 100
#define SPOTTED_SUB_FAIL_LIMIT 100
#define POINTED_SUB_FAIL_LIMIT 100
#define EVENT_SUB_FAIL_LIMIT 100
#define MISSION_SUBS_FAIL_LIMIT 100
#define ITEM_SUB_FAIL_LIMIT 100
#define OTHER_SUBS_FAIL_LIMIT 100

// just some id number to be used at the end of datatype array.
#define GS_DATATYPE_ARR_END 1234


#ifdef PHYSICS_PHYSX
#include "../../physics/IPhysicsLibScriptRunner.h"
class PhysicsLibScriptRunner : public frozenbyte::physics::IPhysicsLibScriptRunner
{
public:
	PhysicsLibScriptRunner(game::GameScripting *gs) { this->gs = gs; }
	bool runPhysicsLibScript(const char *scriptname, const char *subname)
	{
		assert(gs != NULL);
		return gs->runGamePhysicsScript(scriptname, subname);
	}
	game::GameScripting *gs;
};
#endif

using namespace util;

namespace game
{

	const char *gs_keywords[GS_CMD_AMOUNT + 1];
	int gs_datatypes[GS_CMD_AMOUNT + 1];

	Bullet *gs_hitscript_hit_bullet_type = NULL;

	static ScriptProcess *singleCommandScriptProcess = NULL;

	struct gs_commands_listtype
	{
		int id;
		const char *name;
		int datatype;
	};

	#define GS_EXPAND_GAMESCRIPTING_LIST
	
	#include "scriptcommands.h"

	#undef GS_EXPAND_GAMESCRIPTING_LIST


	bool GameScripting::process(util::ScriptProcess *sp, int command, floatint intFloat,
		char *stringData, ScriptLastValueType *lastValue)
	{
		// WARNING: unsafe cast!
		GameScriptData *gsd = (GameScriptData *)sp->getData(); 
		
		bool pause = false;
		//bool pause = true;

#ifdef PROJECT_SHADOWGROUNDS
		// (backward compatibility for sg, do nothing to stringData.)
#else
		// automagically convert "$" strings to gsd->stringValue
    if (stringData != NULL
			&& stringData[0] == '$' && stringData[1] == '\0')
		{
			// do it only if gsd->stringValue != NULL - to keep somewhat compatible with old stuff...
			if (gsd->stringValue != NULL)
				stringData = gsd->stringValue;
		}
#endif

		#define GS_EXPAND_GAMESCRIPTING_CASE
		#include "scripting_macros_start.h"

		switch(command)
		{

		// all of these commands sent to CameraScripting...
		#include "camera_script_commands.h"
			CameraScripting::process(sp, command, intFloat, stringData,
				lastValue, gsd, game);
			break;

		// all of these commands sent to SoundScripting...
		#include "sound_script_commands.h"
			SoundScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to DecorScripting...
		#include "decor_script_commands.h"
			DecorScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to WaterScripting...
		#include "water_script_commands.h"
			WaterScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to CinematicScripting...
		#include "cinematic_script_commands.h"
			CinematicScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to MissionScripting...
		#include "mission_script_commands.h"
			MissionScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to MathScripting...
		#include "math_script_commands.h"
			MathScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to PositionScripting...
		#include "position_script_commands.h"
			PositionScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to MiscScripting...
		#include "misc_script_commands.h"
			MiscScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to OptionScripting...
		#include "option_script_commands.h"
			OptionScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to DevScripting...
		#include "dev_script_commands.h"
			DevScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to UnitScripting...
		#include "unit_script_commands.h"
			UnitScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to StringScripting...
		#include "string_script_commands.h"
			StringScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to LightScripting...
		#include "light_script_commands.h"
			LightScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to AnimationScripting...
		#include "animation_script_commands.h"
			AnimationScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to EnvironmentScripting...
		#include "environment_script_commands.h"
			EnvironmentScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to HitChainScripting...
		#include "hitchain_script_commands.h"
			HitChainScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to MapScripting...
		#include "map_script_commands.h"
			MapScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game);
			break;

		// all these are sent forward to ItemScripting...
		#include "item_script_commands.h"
			ItemScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to TrackingScripting...
		#include "tracking_script_commands.h"
			TrackingScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		// all these are sent forward to SyncScripting...
#ifndef PROJECT_SURVIVOR
		#include "sync_script_commands.h"
			SyncScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;
#endif

		// all these are sent forward to DirectControlScripting...
		#include "directcontrol_script_commands.h"
			DirectControlScripting::process(sp, command, intFloat, stringData, 
				lastValue, gsd, game, &pause);
			break;

		default:
			sp->error("GameScripting::process - Unknown command.");
			sp->debug(sp->getScript()->getName());
			sp->debug("command id:");
			sp->debug(int2str(command));
			//assert(!"GameScripting::process - Unknown command.");
		}

		#include "scripting_macros_end.h"
		#undef GS_EXPAND_GAMESCRIPTING_CASE

		return pause;
	}



	void GameScripting::runScriptProcess(util::ScriptProcess *sp, bool pausable)
	{
		// WARNING: unsafe cast!
		GameScriptData *gsd = (GameScriptData *)sp->getData(); 
		if (gsd->waitCounter > 0)
		{
			gsd->waitCounter--;
		} else {
			bool doRun = true;
			if (gsd->waitDestination)
			{
				if (gsd->unit != NULL)
				{
					const VC3 &position = gsd->unit->getPosition();
					const VC3 &dest = gsd->unit->getFinalDestination();
					float pathAcc = gsd->unit->getUnitType()->getPathAccuracy();

// FIXME: should sticky units use inaccurate position checking too???
// (some seem to get stuck waiting for path complete???)

					if (((gsd->unit->getUnitType()->isSticky()
							&& (position.x != dest.x || position.z != dest.z))
						|| (!gsd->unit->getUnitType()->isSticky() 
							&& (fabs(position.x - dest.x) >= pathAcc 
							|| fabs(position.z - dest.z) >= pathAcc)))
						|| gsd->unit->isTurning())
					{
						doRun = false;
					} else {
						gsd->waitDestination = false;
					}
				} else {
					gsd->waitDestination = false;
				}
			}
			else if(gsd->waitCinematicScreen)
			{
				if(game && game->gameUI && game->gameUI->isCinematicScreenOpen())
				{
					doRun = false;
				}
				else
				{
					gsd->waitCinematicScreen = false;
				}
			}
			if (doRun)
				sp->getScript()->run(sp, pausable);
		}
	}



	// TODO: change scriptprocess parameter to script name, please.
	char *GameScripting::matchSuitableCommands(util::ScriptProcess *sp, 
		int *matches, const char *command, int *smallestMatchLength)
	{
		Script *s = sp->getScript();
		return s->matchSuitableCommands(matches, command, smallestMatchLength);
	}



	void GameScripting::makeAlert(Unit *unit, int distance, const VC3 &position)
	{
		if (unit == NULL)
		{
			Logger::getInstance()->error("GameScripting::makeAlert - Internal error, makeAlert called for null unit.");
			fb_assert(!"GameScripting::makeAlert - Internal error, makeAlert called for null unit.");
			return;
		}

		// HACK: alert distance for other unit types than this unit type is 2/3 of the
		// given alert distance :)
		int distanceForOthers = distance;
		if (distance >= 3)
		{
			distanceForOthers = distance * 2 / 3;
		}

		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u == NULL)
			{
				Logger::getInstance()->error("GameScripting::makeAlert - Fatal internal error. Unit list contains null unit pointer.");
				fb_assert(!"GameScripting::makeAlert - Fatal internal error. Unit list contains null unit pointer.");
			}
			if (u->isActive() && !u->isDestroyed() 
				&& u->getOwner() == unit->getOwner()
				&& u != unit)
			{
				//VC3 pos1 = unit->getPosition();
				VC3 pos1 = position;
				VC3 pos2 = u->getPosition();
				float distsq = (pos1 - pos2).GetSquareLength();
				if ((u->getUnitType() == unit->getUnitType()
					&& distsq < distance * distance)
					|| distsq < distanceForOthers * distanceForOthers)
				{ 				
					if (u->getScript() != NULL)
					{
						// WARNING: unsafe cast!
						UnitLevelAI *ai = (UnitLevelAI *)u->getAI();
						if (ai != NULL && ai->isThisAndAllEnabled())
						{
							ScriptProcess *sp = startUnitScript(u, u->getScript(), "alerted");
							
							// no such sub in script?
							if (sp == NULL) return;

							// WARNING: unsafe cast!
							GameScriptData *gsd = (GameScriptData *)sp->getData();
							assert(gsd != NULL);
							gsd->alertUnit = unit;
							
							int failCount = 0;
							while (!sp->isFinished())
							{
								sp->getScript()->run(sp, false);
								failCount++;
								if (failCount > ALERTED_SUB_FAIL_LIMIT)
								{
									sp->warning("GameScripting::makeAlert - Script sub \"alerted\" did not finish within defined limit.");
									sp->warning("GameScripting::makeAlert - Script process will be abnormally terminated.");
									break;
								}
							}
							if (gsd != NULL)
								delete gsd;
							delete sp;
						}
					}
				}
			}
		}
		return;
	}

	int GameScripting::runTrackerScript(const char *scriptname, const char *subname, UnifiedHandle trackerUnifiedHandle, const std::vector<int> *params)
	{
		int returnValue = 0;

		if (scriptname != NULL)
		{
			ScriptProcess *sp = startNonUnitScript(scriptname, subname, params);
			
			// no such sub in script?
			if (sp == NULL) return 0;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);

			tracking::ITrackerObject *tracker = game->objectTracker->getTrackerByUnifiedHandle(trackerUnifiedHandle);
			assert(tracker != NULL);
			assert(tracker->getType()->getTypeId() == tracking::ScriptableTrackerObjectType::typeId);

			// WARNING: unsafe cast! (assuming that this is called for ScriptableTrackerObjects only)
			tracking::ScriptableTrackerObject *st = (tracking::ScriptableTrackerObject *)tracker;

			gsd->position = st->getTrackerPosition();
			gsd->unifiedHandle = trackerUnifiedHandle;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > OTHER_SUBS_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runTrackerScript - Script sub did not finish within defined limit.");
					sp->warning("GameScripting::runTrackerScript - Script process will be abnormally terminated.");
					sp->debug(scriptname);
					sp->debug(subname);
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			returnValue = sp->getLastValue();
			delete sp;
		}

		return returnValue;
	}

	int GameScripting::runOtherScript(const char *scriptname, const char *subname, Unit *unit, const VC3 &position)
	{
		int returnValue = 0;

		if (scriptname != NULL)
		{
			ScriptProcess *sp = startNonUnitScript(scriptname, subname);
			
			// no such sub in script?
			if (sp == NULL) return 0;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);

			gsd->unit = unit;
			gsd->position = position;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > OTHER_SUBS_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runOtherScript - Script sub did not finish within defined limit.");
					sp->warning("GameScripting::runOtherScript - Script process will be abnormally terminated.");
					sp->debug(scriptname);
					sp->debug(subname);
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;

#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
			if (!sp->lastValue.wasInitialized())
				sp->lastValue = 0;
#endif
			returnValue = sp->getLastValue();
			delete sp;
		}

		return returnValue;
	}


	int GameScripting::runOtherScriptForUnifiedHandle(const char *scriptname, const char *subname, UnifiedHandle uh, const VC3 &position)
	{
		int returnValue = 0;

		if (scriptname != NULL)
		{
			ScriptProcess *sp = startNonUnitScript(scriptname, subname);
			
			// no such sub in script?
			if (sp == NULL) return 0;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);

			gsd->unifiedHandle = uh;
			gsd->position = position;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > OTHER_SUBS_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runOtherScriptForUnifiedHandle - Script sub did not finish within defined limit.");
					sp->warning("GameScripting::runOtherScriptForUnifiedHandle - Script process will be abnormally terminated.");
					sp->debug(scriptname);
					sp->debug(subname);
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			returnValue = sp->getLastValue();
			delete sp;
		}

		return returnValue;
	}


	void GameScripting::runMissionScript(const char *scriptname, const char *subname)
	{
		if (scriptname != NULL)
		{
			ScriptProcess *sp = startNonUnitScript(scriptname, subname);
			
			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > MISSION_SUBS_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runMissionScript - Script sub did not finish within defined limit.");
					sp->warning("GameScripting::runMissionScript - Script process will be abnormally terminated.");
					sp->debug(scriptname);
					sp->debug(subname);
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}

	bool gs_dont_propagate_hitscript = false;


	bool GameScripting::runExecuteScript(Unit *unit, Unit *shooter)
	{
		bool didRun = false;

		// HACK: doors - run the hit script for all nearby (counterpart) 
		// door units too...
		if (!gs_dont_propagate_hitscript
			&& unit->getUnitType()->hasDoorExecute())
		{
			gs_dont_propagate_hitscript = true;
			LinkedList *ulist = game->units->getOwnedUnits(unit->getOwner());
			LinkedListIterator iter(ulist);
			while (iter.iterateAvailable())
			{ 		
				Unit *other = (Unit *)iter.iterateNext();
				// TODO: some treshold to the position...?
				if (other != unit 
					&& other->getSpawnCoordinates().x == unit->getSpawnCoordinates().x
					&& other->getSpawnCoordinates().z == unit->getSpawnCoordinates().z)
				{
					bool success = runExecuteScript(other, shooter);
					if (success) 
						didRun = true;
				}
			}
			gs_dont_propagate_hitscript = false;
		}

		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "execute");

			// no such sub in script?
			if (sp == NULL) return false;

			didRun = true;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->shooter = shooter;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > EXECUTE_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runExecuteScript - Script sub \"execute\" did not finish within defined limit.");
					sp->warning("GameScripting::runExecuteScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}

		return didRun;
	}


	
	void GameScripting::runHitScript(Unit *unit, Unit *shooter, Bullet *hitBulletType)
	{
		// HACK: doors - run the hit script for all nearby (counterpart) 
		// door units too...
		if (!gs_dont_propagate_hitscript
			&& unit->getUnitType()->getName() != NULL
			&& strcmp(unit->getUnitType()->getName(), "Door") == 0)
		{
			gs_dont_propagate_hitscript = true;
			LinkedList *ulist = game->units->getOwnedUnits(unit->getOwner());
			LinkedListIterator iter(ulist);
			while (iter.iterateAvailable())
			{ 		
				Unit *other = (Unit *)iter.iterateNext();
				// TODO: some treshold to the position...?
				if (other != unit 
					&& other->getSpawnCoordinates().x == unit->getSpawnCoordinates().x
					&& other->getSpawnCoordinates().z == unit->getSpawnCoordinates().z)
				{
					runHitScript(other, shooter, hitBulletType);
				}
			}
			gs_dont_propagate_hitscript = false;
		}

		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "hit");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->shooter = shooter;

			gs_hitscript_hit_bullet_type = hitBulletType;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > HIT_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runHitScript - Script sub \"hit\" did not finish within defined limit.");
					sp->warning("GameScripting::runHitScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;

			gs_hitscript_hit_bullet_type = NULL;
		}
	}


	void GameScripting::runHitMissScript(Unit *unit, Unit *shooter)
	{
		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "hitmiss");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->shooter = shooter;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > HIT_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runHitMissScript - Script sub \"hitmiss\" did not finish within defined limit.");
					sp->warning("GameScripting::runHitMissScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}


	void GameScripting::runPointedScript(Unit *unit, Unit *shooter)
	{
		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "pointed");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->shooter = shooter;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > POINTED_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runPointedScript - Script sub \"pointed\" did not finish within defined limit.");
					sp->warning("GameScripting::runPointedScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}


	void GameScripting::runEventScript(Unit *unit, const char *eventName)
	{
		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), eventName);

			// no such sub in script?
			if (sp == NULL) 
			{
				Logger::getInstance()->warning("GameScripting::runEventScript - Could not start event script process (requested script/sub does not exist?).");
				Logger::getInstance()->debug(eventName);
				return;
			}

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > EVENT_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runEventScript - Script event sub did not finish within defined limit.");
					sp->warning("GameScripting::runEventScript - Script process will be abnormally terminated.");
					sp->debug(eventName);
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}


	bool GameScripting::runItemExecuteScript(Unit *unit, Item *item)
	{
		itemScriptItem = item;
		bool ret = runItemScriptImpl(unit, item, "execute", false);
		itemScriptItem = NULL;
		return ret;
	}

	void GameScripting::runItemPickupScript(Unit *unit, Item *item)
	{
		itemScriptItem = item;
		runItemScriptImpl(unit, item, "pickup", false);
		itemScriptItem = NULL;
	}

	void GameScripting::runItemProgressBarScript(Item *item, Unit *unit, const char *subName)
	{
		itemScriptItem = item;
		runItemScriptImpl(unit, item, subName, true);
		itemScriptItem = NULL;
	}

	bool GameScripting::runItemScriptImpl(Unit *unit, Item *item, const char *subName, bool ignoreMissingSub)
	{
		bool didRun = false;

		this->itemMarkedForRemove = false;
		this->itemMarkedForDisable = false;
		this->itemMarkedForDisableTime = 0;

		int itemTypeId = item->getItemTypeId();
		ItemType *itemType = game->itemManager->getItemTypeById(itemTypeId);
		if (itemType->getScript() != NULL)
		{
			const char *scriptName = itemType->getScript();
			if (item->getCustomScript() != NULL)
			{
				scriptName = item->getCustomScript();
			}

			if (scriptName == NULL)
			{
				assert(!"GameScripting::runItemScriptImpl - Null script name encountered.");
				return false;
			}

			Script *s = ScriptManager::getInstance()->getScript(scriptName);
			if (s == NULL)
			{
				Logger::getInstance()->warning("GameScripting::runItemScriptImpl - Script not loaded or does not exist.");
				Logger::getInstance()->debug(scriptName);
				assert(!"GameScripting::runItemScriptImpl - No such script.");
				return false;
			}

			if (ignoreMissingSub && !s->hasSub(subName))
			{
				Logger::getInstance()->debug("GameScripting::runItemScriptImpl - Item script sub does not exist (but ignoring that).");
				Logger::getInstance()->debug(scriptName);
				Logger::getInstance()->debug(subName);
				return false;
			}

			ScriptProcess *sp = NULL;
			if (unit != NULL)
			{
				sp = startUnitScript(unit, scriptName, subName);
			} else {
				sp = startNonUnitScript(scriptName, subName);
			}

			// no such sub in script?
			if (sp == NULL) return false;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);

			didRun = true;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > ITEM_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runItemScript - Script sub \"pickup\" did not finish within defined limit.");
					sp->warning("GameScripting::runItemScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}

		if (this->itemMarkedForRemove)
		{
			game->itemManager->deleteItem(item);
		} else {
			if (this->itemMarkedForDisable)
			{
				game->itemManager->disableItem(item, this->itemMarkedForDisableTime);
			}
		}

		this->itemMarkedForRemove = false;

		return didRun;
	}


	void GameScripting::runItemUseScript(Unit *unit, Item *item)
	{
		itemScriptItem = item;
		this->itemMarkedForRemove = false;

		int itemTypeId = item->getItemTypeId();
		ItemType *itemType = game->itemManager->getItemTypeById(itemTypeId);
		if (itemType->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, itemType->getScript(), "useitem");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > ITEM_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runItemUseScript - Script sub \"useitem\" did not finish within defined limit.");
					sp->warning("GameScripting::runItemUseScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}

		assert(!this->itemMarkedForRemove);
		itemScriptItem = NULL;
	}


	void GameScripting::runHearNoiseScript(Unit *unit, Unit *noisy)
	{
		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "hearnoise");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->noisy = noisy;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > HIT_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runHearNoiseScript - Script sub \"hearnoise\" did not finish within defined limit.");
					sp->warning("GameScripting::runHearNoiseScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}


	void GameScripting::runSpottedScript(Unit *unit, Unit *spotted)
	{
		if (unit->getScript() != NULL)
		{
			ScriptProcess *sp = startUnitScript(unit, unit->getScript(), "spotted");

			// no such sub in script?
			if (sp == NULL) return;

			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			assert(gsd != NULL);
			gsd->spottedUnit = spotted;
			
			int failCount = 0;
			while (!sp->isFinished())
			{
				sp->getScript()->run(sp, false);
				failCount++;
				if (failCount > HIT_SUB_FAIL_LIMIT)
				{
					sp->warning("GameScripting::runSpottedScript - Script sub \"spotted\" did not finish within defined limit.");
					sp->warning("GameScripting::runSpottedScript - Script process will be abnormally terminated.");
					break;
				}
			}
			if (gsd != NULL)
				delete gsd;
			delete sp;
		}
	}


	//struct GSCommandSorter: 
	//public std::binary_function<gs_commands_listtype, gs_commands_listtype, bool>
	//{
	//	bool operator() (const gs_commands_listtype &a, const gs_commands_listtype &b) const
	//	{
	//		return a.id < b.id;
	//	}
	//};

	GameScripting::GameScripting(Game *game)
	{
		this->game = game;

		//std::sort(&gs_commands[0], &gs_commands[GS_CMD_AMOUNT + 1], GSCommandSorter());
#ifdef _DEBUG
		//if (gs_commands[GS_CMD_AMOUNT].id != GS_MAX_COMMANDS)
		//{
		//	assert(!"GameScripting - Command name array not ok (after sort). Fix it.");
		//}
		//{
		//	for (int i = 0; i < GS_CMD_AMOUNT; i++)
		//	{
		//		void *foo = gs_commands;
		//		if (gs_commands[i].id != i)
		//		{
		//			assert(!"GameScripting - Command name array not ok, (after sort). Fix it.");
		//		}
		//	}
		//}
#endif

		for (int i = 0; i < GS_CMD_AMOUNT; i++)
		{
			gs_keywords[i] = "__reserved_no_command";
			gs_datatypes[i] = SCRIPT_DATATYPE_NONE;
		}
		for (int i = 0; i < GS_CMD_AMOUNT; i++)
		{
			if (gs_commands[i].id == GS_MAX_COMMANDS)
			{
				break;
			}
			if (gs_commands[i].id < GS_CMD_AMOUNT)
			{
				if (strcmp(gs_keywords[gs_commands[i].id], "__reserved_no_command") == 0)
				{
					gs_keywords[gs_commands[i].id] = gs_commands[i].name;
					gs_datatypes[gs_commands[i].id] = gs_commands[i].datatype;
				} else {
					assert(!"GameScripting - Duplicate command id. Fix it.");
				}
			} else {
				assert(!"GameScripting - Command id over maximum amount. Fix it.");
			}
		}

#ifdef _DEBUG
		//if (gs_commands[GS_CMD_AMOUNT].name != NULL)
		//{
		//	if (strcmp(gs_commands[GS_CMD_AMOUNT].name, "***") != 0)
		//	{
		//		assert(!"GameScripting - Command name array not ok. Fix it.");
		//	}
		//} else {
		//	assert(!"GameScripting - Command name array not ok. Fix it.");
		//}
		//if (gs_commands[GS_CMD_AMOUNT].datatype != GS_DATATYPE_ARR_END)
		//{
		//	assert(!"GameScripting - Command datatype array not ok. Fix it.");
		//}
#endif

		ScriptManager::getInstance()->setProcessor(this);
		ScriptManager::getInstance()->setKeywords(GS_CMD_AMOUNT, 
			gs_keywords, gs_datatypes);

		this->itemMarkedForRemove = false;

		VC2 trackerAreaSize = VC2(game->gameMap->getScaledSizeX(), game->gameMap->getScaledSizeY());
		this->areaTracker = new CircleAreaTracker(trackerAreaSize);

#ifdef PHYSICS_PHYSX
		gamePhysicsScriptRunnerImplementation = new PhysicsLibScriptRunner(this);
#else
		gamePhysicsScriptRunnerImplementation = NULL;
#endif

	}


	GameScripting::~GameScripting()
	{
#ifdef PHYSICS_PHYSX
		delete (PhysicsLibScriptRunner *)gamePhysicsScriptRunnerImplementation;
#endif

		if (singleCommandScriptProcess != NULL)
		{
			delete singleCommandScriptProcess;
			singleCommandScriptProcess = NULL;
		}
		assert(this->areaTracker != NULL);
		delete this->areaTracker;
	}



	void GameScripting::loadScripts(const char *filename, const char *relativeToFilenamePath)
	{
		ScriptManager::getInstance()->loadScripts(filename, relativeToFilenamePath);
	}


	// NOTE: ScriptManager may modify the buffer (inserts some null terminators!)
	void GameScripting::loadMemoryScripts(const char *memoryFilename, 
		char *buf, int buflen)
	{
		ScriptManager::getInstance()->loadMemoryScripts(memoryFilename,
			buf, buflen, true);
	}


	ScriptProcess *GameScripting::startUnitScript(Unit *unit, const char *script, const char *sub, const std::vector<int> *paramStack)
	{
		assert(unit != NULL);
		assert(script != NULL);
		assert(sub != NULL);

		Script *s = ScriptManager::getInstance()->getScript(script);
		if (s == NULL)
		{
			Logger::getInstance()->warning("GameScripting::startUnitScript - Script not loaded or does not exist.");
			Logger::getInstance()->debug(script);
			assert(!"startUnitScript - No such script.");
			return NULL;
		}

		ScriptProcess *sp = NULL;
		if (s->hasSub(sub))
		{
			sp = s->createNewProcess(sub);

			GameScriptData *gsd = new GameScriptData();
			gsd->unit = unit;
			gsd->originalUnit = unit;
			sp->setData(gsd);

			if (paramStack != NULL)
			{
				for (int i = 0; i < (int)paramStack->size(); i++)
				{
					sp->pushCallParamStack((*paramStack)[i]);
				}
			}
		} else {
			Logger::getInstance()->debug("GameScripting::startUnitScript - Given sub not defined in script.");
			Logger::getInstance()->debug(script);
			Logger::getInstance()->debug(sub);
		}

		return sp;
	}


	ScriptProcess *GameScripting::startNonUnitScript(const char *script, const char *sub, const std::vector<int> *paramStack)
	{
		assert(script != NULL);
		assert(sub != NULL);

		Script *s = ScriptManager::getInstance()->getScript(script);
		if (s == NULL)
		{
			Logger::getInstance()->warning("GameScripting::startNonUnitScript - Script not loaded or does not exist.");
			Logger::getInstance()->debug(script);
		}
//		assert(s != NULL);

		if (s == NULL) return NULL;

		ScriptProcess *sp = NULL;
		if (s->hasSub(sub))
		{
			sp = s->createNewProcess(sub);

			GameScriptData *gsd = new GameScriptData();
			sp->setData(gsd);

			if (paramStack != NULL)
			{
				for (int i = 0; i < (int)paramStack->size(); i++)
				{
					sp->pushCallParamStack((*paramStack)[i]);
				}
			}
		} else {
			Logger::getInstance()->warning("GameScripting::startNonUnitScript - Given sub not defined in script.");
			Logger::getInstance()->debug(script);
			Logger::getInstance()->debug(sub);
		}

		return sp;
	}

	void GameScripting::newGlobalIntVariable( const char* variablename, bool permanent )
	{
		Script::newGlobalIntVariable( variablename, permanent );
	}

	void GameScripting::setGlobalIntVariableValue(const char *variablename, int value)
	{
		bool success = Script::setGlobalIntVariableValue(variablename, value);
		if (!success)
		{
			Logger::getInstance()->warning("GameScripting::setGlobalIntVariableValue - Global variable with given name not found.");
			Logger::getInstance()->debug(variablename);
		}
	}

	int GameScripting::getGlobalIntVariableValue(const char *variablename)
	{
		int ret = 0;
		bool success = false;
		char *array_name = NULL;
		int array_index = -1;

		// support for array access
		if(variablename)
		{
			int i = 0;
			int array_start = 0;
			int array_end = 0;
			while(true)
			{
				if(variablename[i] == '\0') break;
				if(variablename[i] == '[') array_start = i;
				else if(variablename[i] == ']') array_end = i;
				i++;
			}
			// found array
			if(array_end > array_start + 1)
			{
				// copy name
				array_name = new char[array_start + 1];
				memcpy(array_name, variablename, array_start);
				array_name[array_start] = '\0';

				// get index
				char *array_index_text = new char[array_end - array_start];
				memcpy(array_index_text, variablename + array_start + 1, array_end - array_start - 1);
				array_index_text[array_end - array_start - 1] = '\0';
				array_index = str2int(array_index_text);
				delete[] array_index_text;
			}
		}

		if(array_index >= 0 && array_name)
		{
			success = Script::getGlobalArrayVariableValue(array_name, array_index, &ret);
			delete[] array_name;
		}
		else
			success = Script::getGlobalIntVariableValue(variablename, &ret);

		if (!success)
		{
			Logger::getInstance()->warning("GameScripting::getGlobalIntVariableValue - Global variable with given name not found.");
			Logger::getInstance()->debug(variablename);
		}
		return ret;
	}

	void GameScripting::deleteGameScript(util::ScriptProcess *sp)
	{
		assert(sp != NULL);

		// WARNING: unsafe cast!
		GameScriptData *gsd = (GameScriptData *)sp->getData();
		assert(gsd != NULL);
		if (gsd != NULL)
			delete gsd;
		delete sp;
	}

	void GameScripting::addAreaTrigger(Unit *unit, const VC3 &position, float range, int clipMask, const LinkedList *unitsToTrack)
	{
		//char buf[256];
		//sprintf(buf, "%f,%f - range %f - clip %d", position.x, position.z, range, clipMask);
		//Logger::getInstance()->error(buf);

		if (unit->getAreaCircleId() != -1)
		{
			assert(this->areaTracker != NULL);
			this->areaTracker->removeCircleTrigger(unit->getAreaCircleId());
			unit->setAreaCircleId(-1);
		}

		unit->setAreaTriggered(false);
		ClippedCircle circle(position, range, clipMask);
		int circleId = this->areaTracker->addCircleTrigger(circle, this, unit);
		LinkedListIterator iter(unitsToTrack);
		while (iter.iterateAvailable())
		{
			Unit *trackable = (Unit *)iter.iterateNext();

			// TEMP
			const char *idstr = trackable->getIdString();
			if (idstr == NULL || strcmp(idstr, "player1") != 0)
			{
				// assert(!"GameScripting::addAreaTrigger - Oops, tracking someone else other than player1...?");
			}

			this->areaTracker->addTrackable(circleId, trackable);
		}
		unit->setAreaCircleId(circleId);
	}

	void GameScripting::updateAreaTracker()
	{
		this->areaTracker->update(GAME_TICK_MSEC);
	}

	void GameScripting::activate(int circleId, void *data)
	{
		assert(data != NULL);

		// WARNING: unsafe cast!
		Unit *u = (Unit *)data;
		// WARNING: unsafe cast!
		UnitLevelAI *ai = (UnitLevelAI *)u->getAI();

		assert(ai != NULL);

		this->areaTracker->removeCircleTrigger(circleId);
		fb_assert(u->getAreaCircleId() != -1);
		u->setAreaCircleId(-1);

		u->setAreaTriggered(true);
		ai->skipMainScriptWait();
	}

  void GameScripting::runHitChainScript(const char *scriptname, Projectile *origin, 
		Unit *hitUnit, Unit *shooter, Bullet *chainBullet,
		const VC3 &position, int hitchain, const VC3 &direction, const VC3 &hitPlaneNormal)
	{
		if (scriptname == NULL)
			return;

		assert(origin != NULL);

		projs_originPosition = origin->getPosition();
		projs_originalPosition = position;
		projs_chainedPosition = position;

		projs_originDirection = origin->getDirection();
		projs_originalDirection = direction;
		projs_chainedDirection = direction;

		projs_hitPlaneNormal = hitPlaneNormal;

		// TODO: solve proper range somehow, don't know exactly how...
		// for now, just 25m
		projs_chainedRange = 25.0f;

		projs_chainedVelocityFactor = 1.0f;

		projs_lifeTime = -1;
		projs_hitChain = hitchain;

		projs_chainedRotationRandom = false;
		projs_chainedOriginToHitUnit = false;

		projs_chainedCustomValue = origin->getChainCustomValue();

		projs_hitUnit = hitUnit;
		projs_shooter = shooter;

    projs_chainedBulletType = chainBullet;
    projs_originBulletType = origin->getBulletType();


		ScriptProcess *sp = this->startNonUnitScript(scriptname, "hitchain");

		// no such sub in script?
		if (sp == NULL) 
		{
			// already logged by startNonUnitScript
			//Logger::getInstance()->error("GameScripting::runHitChainScript - Sub \"hitchain\" not defined in script.");
			return;
		}

		// WARNING: unsafe cast!
		GameScriptData *gsd = (GameScriptData *)sp->getData();
		assert(gsd != NULL);
		
		int failCount = 0;
		while (!sp->isFinished())
		{
			sp->getScript()->run(sp, false);
			failCount++;
			if (failCount > EXECUTE_SUB_FAIL_LIMIT)
			{
				sp->warning("GameScripting::runHitChainScript - Script sub \"hitchain\" did not finish within defined limit.");
				sp->warning("GameScripting::runHitChainScript - Script process will be abnormally terminated.");
				break;
			}
		}
		if (gsd != NULL)
			delete gsd;
		delete sp;

	}

	void GameScripting::runSingleCommand(int command, floatint intFloat, const char *stringData, int *lastValue, int *secondaryValue, GameScriptData *gsd)
	{
		assert(lastValue != NULL);
		assert(secondaryValue != NULL);
		assert(gsd != NULL);

		if (singleCommandScriptProcess == NULL)
		{
			ScriptManager *sman = ScriptManager::getInstance();
			assert(sman != NULL);

			Script *s = sman->getScript("_single_command");

			if (s == NULL)
			{
				char *buf = new char[256];
				strcpy(buf, "\nscript _single_command\nsub main\nnoOperation\nendSub\nendScript\n\n");
				sman->loadMemoryScripts("memory/single_command", buf, strlen(buf), false);
				delete [] buf;
			}

			s = sman->getScript("_single_command");
			assert(s != NULL);

			if (s == NULL)
			{
				// bug bug bug???
				return;
			}

			singleCommandScriptProcess = s->createNewProcess("main");
			assert(singleCommandScriptProcess != NULL);
		}

		singleCommandScriptProcess->setData(gsd);
		singleCommandScriptProcess->setLastValue(*lastValue);
		singleCommandScriptProcess->setSecondaryValue(*secondaryValue);

		// WARNING: const char * -> char * cast... as that should be the case (?)
		process(singleCommandScriptProcess, command, intFloat, (char *)stringData, &singleCommandScriptProcess->lastValue);

		*lastValue = singleCommandScriptProcess->lastValue;
		*secondaryValue = singleCommandScriptProcess->secondaryValue;
	}

	void GameScripting::runSingleSimpleCommand(int command, floatint intFloat, const char *stringData, int *lastValue, int *secondaryValue)
	{
		// NOTE: optimization, this could be a static variable that gets intialized only once...
		// but that might cause problems when calling some commands, as the previous values would persist.
		GameScriptData gsd;
		int dummy1 = 0, dummy2 = 0;
		if (lastValue == NULL) lastValue = &dummy1;
		if (secondaryValue == NULL) secondaryValue = &dummy2;
		runSingleCommand(command, intFloat, stringData, lastValue, secondaryValue, &gsd);
	}

	void GameScripting::runMultipleSimpleCommands(int commandAmount, int *command, floatint *intFloat, const char **stringData, int *lastValue, int *secondaryValue)
	{
		GameScriptData gsd;
		int dummy1 = 0, dummy2 = 0;
		if (lastValue == NULL) lastValue = &dummy1;
		if (secondaryValue == NULL) secondaryValue = &dummy2;
		for (int i = 0; i < commandAmount; i++)
		{
			runSingleCommand(command[i], intFloat[i], stringData[i], lastValue, secondaryValue, &gsd);
		}
	}

	bool GameScripting::runSingleSimpleStringCommand(const char *command, const char *param, int *lastValue, int *secondaryValue)
	{
		assert(command != NULL);

		int dummy1 = 0, dummy2 = 0;
		if (lastValue == NULL) lastValue = &dummy1;
		if (secondaryValue == NULL) secondaryValue = &dummy2;

		// seek correct id for command
		int cmdid = -1;
		for (int i = 0; i < GS_CMD_AMOUNT; i++)
		{
			if (strcmp(gs_keywords[i], command) == 0)
			{
				cmdid = i;
				break;
			}
		}

		if (cmdid == -1)
		{
			Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - No command of given name.");
			Logger::getInstance()->debug(command);
			return false;
		}

		if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_NONE)
		{
			floatint z;
			z.i = 0;
			runSingleSimpleCommand(cmdid, z, param, lastValue, secondaryValue);
		}
		else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_INT)
		{
			floatint tmp;
			if (param != NULL)
			{
				tmp.i = str2int(param);
				if (str2int_errno() != 0)
				{
					Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - Command expected int, but given parameter is not a valid int value.");
				}
			} else {
				tmp.i = 0;
				Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - Command expected int, but no parameter given.");
			}
			runSingleSimpleCommand(cmdid, tmp, param, lastValue, secondaryValue);
		}
		else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_STRING)
		{
			floatint z;
			z.i = 0;
			runSingleSimpleCommand(cmdid, z, param, lastValue, secondaryValue);
		}
		else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_FLOAT)
		{
			float tmpfloat = 0.0f;
			if (param != NULL)
			{
				tmpfloat = (float)atof(param);
				// TODO: a better float check... :)
				if ((param[0] < '0' || param[0] > '9') && param[0] != '-')
				{
					Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - Command expected float, but given parameter is not a valid float value.");
				}
			} else {
				Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - Command expected float, but no parameter given.");
			}
			floatint tmp;
			tmp.f = tmpfloat;
			runSingleSimpleCommand(cmdid, tmp, param, lastValue, secondaryValue);			
		} else {
			Logger::getInstance()->error("GameScripting::runSingleSimpleStringCommand - Unsupported command parameter type.");
			return false;
		}

		return true;
	}

	bool GameScripting::runMultipleSimpleStringCommands(int commandAmount, const char **command, const char **param, int *lastValue, int *secondaryValue)
	{
		assert(command != NULL);
		assert(param != NULL);

		int dummy1 = 0, dummy2 = 0;
		if (lastValue == NULL) lastValue = &dummy1;
		if (secondaryValue == NULL) secondaryValue = &dummy2;

		for (int i = 0; i < commandAmount; i++)
		{
			assert(command[i] != NULL);

			// seek correct id for command
			int cmdid = -1;
			for (int j = 0; j < GS_CMD_AMOUNT; j++)
			{
				if (strcmp(gs_keywords[j], command[i]) == 0)
				{
					cmdid = j;
					break;
				}
			}

			if (cmdid == -1)
			{
				Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - No command of given name.");
				Logger::getInstance()->debug(command[i]);
				return false;
			}

			if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_NONE)
			{
				floatint z;
				z.i = 0;
				runSingleSimpleCommand(cmdid, z, param[i], lastValue, secondaryValue);
			}
			else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_INT)
			{
				floatint tmp;
				if (param[i] != NULL)
				{
					tmp.i = str2int((char *)(param[i]));
					if (str2int_errno() != 0)
					{
						Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - Command expected int, but given parameter is not a valid int value.");
					}
				} else {
					tmp.i = 0;
					Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - Command expected int, but no parameter given.");
				}
				runSingleSimpleCommand(cmdid, tmp, param[i], lastValue, secondaryValue);
			}
			else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_STRING)
			{
				floatint z;
				z.i = 0;
				runSingleSimpleCommand(cmdid, z, param[i], lastValue, secondaryValue);
			}
			else if (gs_datatypes[cmdid] == SCRIPT_DATATYPE_FLOAT)
			{
				float tmpfloat = 0.0f;
				if (param != NULL)
				{
					tmpfloat = (float)atof(param[i]);
					// TODO: a better float check... :)
					if ((param[i][0] < '0' || param[i][0] > '9') && param[i][0] != '-')
					{
						Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - Command expected float, but given parameter is not a valid float value.");
					}
				} else {
					Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - Command expected float, but no parameter given.");
				}
				floatint tmp;
				tmp.f = tmpfloat;
				runSingleSimpleCommand(cmdid, tmp, param[i], lastValue, secondaryValue);			
			} else {
				Logger::getInstance()->error("GameScripting::runMultipleSimpleStringCommands - Unsupported command parameter type.");
				return false;
			}
		}

		return true;
	}

	bool GameScripting::runGamePhysicsScript(const char *scriptname, const char *subname)
	{
		VC3 pos = VC3(0,0,0);
		int ret = this->runOtherScript(scriptname, subname, NULL, pos);
		if (ret != 0)
			return true;
		else
			return false;		
	}

	void *GameScripting::getGamePhysicsScriptRunnerImplementation()
	{
		return this->gamePhysicsScriptRunnerImplementation;
	}

}

