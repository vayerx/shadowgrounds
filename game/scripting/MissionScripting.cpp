
#include "precompiled.h"

#include "MissionScripting.h"

#include "scripting_macros_start.h"
#include "mission_script_commands.h"
#include "scripting_macros_end.h"

// NOTE: Some problems with the DatatypeDef.h including...
// unless this exists here, some math stuff will not be included.
// (even though this DatatypeDef is included by other headers)
#include <DatatypeDef.h>

#include "../Game.h"
#include "../GameUI.h"
#include "../GameStats.h"
#include "../GameScene.h"
#include "../GameMap.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../Checkpoints.h"
#include "../CheckpointChecker.h"
#include "../UnitList.h"
#include "../BuildingAdder.h"
#include "../PlayerPartsManager.h"
#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../options/options_physics.h"
#include "../options/options_game.h"
#include "../../ui/CombatWindow.h"
#include "../../ui/ElaborateHintMessageWindow.h"
#include "../game/DHLocaleManager.h"

#ifdef PROJECT_SURVIVOR
	#include "../../ui/SurvivalMenu.h"
#endif

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	void MissionScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_SETMISSIONSUCCESSCOUNTER:
			if(game::SimpleOptions::getBool(DH_OPT_B_FORCE_MISSION_FAILURE))
			{
				if (game->missionFailureCounter >= 0)
					game->missionFailureCounter = intData * GAME_TICKS_PER_SECOND;
				game->missionAborting = true;
			}
			else
			{
				if (game->missionSuccessCounter >= 0)
					game->missionSuccessCounter = intData * GAME_TICKS_PER_SECOND;
			}
			break;

		case GS_CMD_SETMISSIONFAILURECOUNTER:
			if (game->missionFailureCounter >= 0)
				game->missionFailureCounter = intData * GAME_TICKS_PER_SECOND;
			break;

		case GS_CMD_COUNTALIVEUNITS:
			*lastValue = calculateAliveUnits(game, gsd->player); 
			break;

		case GS_CMD_COUNTCONSCIOUSUNITS:
			*lastValue = calculateConsciousUnits(game, gsd->player);
			break;

		case GS_CMD_COUNTALIVEHOSTILEUNITS:
			*lastValue = calculateAliveHostileUnits(game, gsd->player);
			break;

		case GS_CMD_ISMISSIONSUCCESS:
			if (game->missionSuccessCounter < 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ISMISSIONFAILURE:
			if (game->missionFailureCounter < 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ELAPSEDMISSIONTIME:
				*lastValue = (game->gameTimer - game->missionStartTime) / GAME_TICKS_PER_SECOND;
			break;

		case GS_CMD_SETMISSION:
			if (stringData != NULL)
			{
				if (strcmp(stringData, "successMission") == 0)
				{
					game->setCurrentMission(game->getSuccessMission());
					break;
				}
				if (strcmp(stringData, "failureMission") == 0)
				{
					game->setCurrentMission(game->getFailureMission());
					break;
				}
				sp->error("GameScripting::process - setMission parameter bad.");
			} else {
				sp->error("GameScripting::process - setMission parameter expected.");
			}
			break;

		case GS_CMD_SETMISSIONFILE:
			if (stringData != NULL)
			{
				game->setCurrentMission(stringData);
			} else {
				sp->error("GameScripting::process - setMissionFile parameter expected.");
			}
			break;

		case GS_CMD_SETLOADMISSIONFILE:
			if (stringData != NULL)
			{
				// this just sets the failure mission, loading actually just makes the
				// current mission fail (thus effectively changing to this one)
				// 
				// but if in menus, set the current mission.
				if (game->inCombat)
				{
					game->setFailureMission(stringData);
				} else {
					game->setCurrentMission(stringData);
				}
			} else {
				sp->error("GameScripting::process - setLoadMissionFile parameter expected.");
			}
			break;

		case GS_CMD_ENDCOMBAT:
			game->requestEndCombat();
			break;

		case GS_CMD_FRIENDLYSNEARPOSITION:
			//if (unit != NULL)
			//{
				if (intData <= 0)
				{
					sp->warning("GameScripting::process - friendlysNearPosition parameter out of range.");
				} else {
					*lastValue = countFriendlysInRange(game, gsd->position, gsd->player, intData);
				}
			//} else {
			//	sp->warning("GameScripting::process - Attempt to friendlysNearPosition for null unit.");
			//}
			break;

		case GS_CMD_HOSTILESNEARPOSITION:
			//if (unit != NULL)
			//{
				if (intData <= 0)
				{
					sp->warning("GameScripting::process - hostilesNearPosition parameter out of range.");
				} else {
					*lastValue = countHostilesInRange(game, gsd->position, gsd->player, intData, false);
				}
			//} else {
			//	sp->warning("GameScripting::process - Attempt to hostilesNearPosition for null unit.");
			//}
			break;

		case GS_CMD_CONSCIOUSHOSTILESNEARPOSITION:
			//if (unit != NULL)
			//{
				if (intData <= 0)
				{
					sp->warning("GameScripting::process - consciousHostilesNearPosition parameter out of range.");
				} else {
					*lastValue = countHostilesInRange(game, gsd->position, gsd->player, intData, true);
				}
			//} else {
			//	sp->warning("GameScripting::process - Attempt to hostilesNearPosition for null unit.");
			//}
			break;

		case GS_CMD_ISEVERYUNITNEARCHECKPOINT:
			if (CheckpointChecker::isEveryUnitNearCheckpoint(game, 
				(float)intData, gsd->player))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ISANYUNITNEARCHECKPOINT:
			if (CheckpointChecker::isAnyUnitNearCheckpoint(game, 
				(float)intData, gsd->player))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ISPOSITIONNEARCHECKPOINT:
			if (CheckpointChecker::isPositionNearCheckpoint(game, 
				(float)intData, gsd->position))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_SETCHECKPOINTTOPOSITION:
			game->checkpoints->setCheckpoint(0, gsd->position.x, gsd->position.z);
			game->checkpoints->enableCheckpoint(0);
			break;

		case GS_CMD_CLEARCHECKPOINT:
			game->checkpoints->disableCheckpoint(0);
			break;

		case GS_CMD_ISANYUNITNEARPOSITION:
			if (isAnyUnitNearPosition(game, gsd->player, 
				(float)intData, gsd->position))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ISEVERYUNITNEARPOSITION:
			if (isEveryUnitNearPosition(game, gsd->player, 
				(float)intData, gsd->position))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ADDBUILDINGTOPOSITION:
			if (stringData != NULL)
			{
				VC3 bpos = gsd->position;
				bpos.y = game->gameMap->getScaledHeightAt(bpos.x, bpos.z);
				BuildingAdder::addBuilding(game, bpos, stringData, game->getGamePhysics());
			} else {
				sp->warning("GameScripting::process - addBuildingToPosition parameter missing.");
			}
			break;

		case GS_CMD_ADDMONEY:
			game->money[gsd->player] += intData;
			*lastValue = game->money[gsd->player];
			break;

		case GS_CMD_SETHOSTILE:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				game->hostile[gsd->player][intData] = true;
			} else {
				sp->error("GameScripting::process - setHostile parameter value out of range.");
			}
			break;

		case GS_CMD_SETFRIENDLY:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				game->hostile[gsd->player][intData] = false;
			} else {
				sp->error("GameScripting::process - setFriendly parameter value out of range.");
			}
			break;

		case GS_CMD_SETSPAWN:
			if (stringData != NULL)
			{
				if (strlen(stringData) < 16)
				{
					char splitbuf[16];
					strcpy(splitbuf, stringData);
					int spawnx = 0;
					int spawny = 0;
					int buflen = strlen(splitbuf);
					for (int splitp = 0; splitp < buflen; splitp++)
					{
						if (splitbuf[splitp] == ',')
						{
							splitbuf[splitp] = '\0';
							spawnx = str2int(splitbuf);
							spawny = str2int(&splitbuf[splitp + 1]);
							break;
						}
					}
					if (spawnx <= 0 || spawny <= 0)
					{
						sp->error("GameScripting::process - setSpawn parameter bad.");
					} else {
						game->spawnX[gsd->player] = spawnx;
						game->spawnY[gsd->player] = spawny;
					}
				} else {
					sp->error("GameScripting::process - setSpawn parameter format bad.");
				}
			} else {
				sp->error("GameScripting::process - setSpawn parameter missing.");
			}
			break;

		case GS_CMD_ALLOWPARTTYPE:
			if (stringData != NULL)
			{
				bool success = game->playerPartsManager->allowPartType(
					gsd->player, stringData);
				if (!success)
				{
					sp->error("GameScripting::process - allowPartType failed.");
				}
			} else {
				sp->error("GameScripting::process - allowPartType parameter missing.");
			}
			break;

		case GS_CMD_ADDSTORAGEPART:
			if (stringData != NULL)
			{
				gsd->part = game->playerPartsManager->addStoragePart(
					gsd->player, stringData);
				if (gsd->part == NULL)
				{
					sp->error("GameScripting::process - addStoragePart failed.");
				}
			} else {
				sp->error("GameScripting::process - addStoragePart parameter missing.");
			}
			break;

		case GS_CMD_PLAYERUNITSINACTION:
			if (playerUnitsInAction(game, gsd->player))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_OWNEDNEARPOSITION:
			//if (unit != NULL)
			//{
				if (intData <= 0)
				{
					sp->warning("GameScripting::process - ownedNearPosition parameter out of range.");
				} else {
					*lastValue = countOwnedInRange(game, gsd->position, gsd->player, intData);
				}
			//} else {
			//	sp->warning("GameScripting::process - Attempt to friendlysNearPosition for null unit.");
			//}
			break;

		case GS_CMD_OWNEDNEARPOSITIONBYVALUE:
			//if (unit != NULL)
			//{
				if (*lastValue <= 0)
				{
					sp->warning("GameScripting::process - ownedNearPositionByValue value out of range.");
				} else {
					int useRange = *lastValue;
					*lastValue = countOwnedInRange(game, gsd->position, gsd->player, useRange);
				}
			//} else {
			//	sp->warning("GameScripting::process - Attempt to friendlysNearPosition for null unit.");
			//}
			break;

		case GS_CMD_ISMISSIONRUNNING:
			if (game->inCombat)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETTERRAINCUT:
			if (intData != 0)
			{
				BuildingAdder::setTerrainCut(true);
			} else {
				BuildingAdder::setTerrainCut(false);
			}
			break;

		case GS_CMD_GETMISSIONIDSTRING:
			gsd->setStringValue(game->getMissionId());
			break;

		case GS_CMD_SETMISSIONID:
			if (stringData != NULL)
			{
				game->setMissionId(stringData);
			} else {
				sp->error("GameScripting::process - setMissionId parameter missing.");
			}
			break;

		case GS_CMD_setTerrainMaterial:
			if (stringData != NULL)
			{
				game->gameScene->setTerrainMaterial(stringData);
			} else {
				sp->error("GameScripting::process - setTerrainMaterial parameter missing.");
			}
			break;
		case GS_CMD_setSuccessMission:
			game->setSuccessMission (stringData);
			break;

		case GS_CMD_setPhysicsWaterSettings:
			if (stringData != NULL)
			{
				unsigned int splitPos = 0;
				unsigned int i = 0;
				while(true)
				{
					if(stringData[i] == 0) break;
					if(stringData[i] == ',')
					{
						splitPos = i;
						stringData[i] = 0;
						break;
					}
					i++;
				}
				if(splitPos >= 0)
				{
					SimpleOptions::setFloat(DH_OPT_F_PHYSICS_WATER_HEIGHT, (float)atof(stringData));
					SimpleOptions::setFloat(DH_OPT_F_PHYSICS_WATER_DAMPING, (float)atof(stringData + splitPos + 1));
				}
				else
				{
					sp->error("GameScripting::process - setPhysicsWaterSettings expects two parameters; \"height,damping\".");
				}
			} else {
				sp->error("GameScripting::process - setPhysicsWaterSettings parameter missing.");
			}
			break;

		case GS_CMD_setAlphaTestPassEnabled:
			game->gameUI->enableAlphaTestPass(intData == 0 ? false : true);
			break;
		
		case GS_CMD_unlockSurvivalMission:
			if(stringData != NULL)
			{
#ifdef PROJECT_SURVIVOR
				std::string name = stringData;
				std::vector<std::string> locked;
				SurvivalMenu::readLockedMissions(locked);

				// unlocking all
				if(name == "*")
				{
					for(unsigned int i = 0; i < locked.size(); i++)
					{
						SurvivalMenu::unlockMission(locked[i]);
						*lastValue = 1;
					}
				}
				// not unlocked yet
				else if(std::find(locked.begin(), locked.end(), name) != locked.end())
				{
					SurvivalMenu::unlockMission(name);
					
					if(game->gameUI->getCombatWindow( 0 ))
					{
						SurvivalMenu::MissionInfo mi;
						ElaborateHintMessageWindow* win = ((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( 0 )->getSubWindow( "ElaborateHintMessageWindow" ));
						if(win && SurvivalMenu::loadMissionInfo(name, mi))
						{
							win->setStyle("upgradeunlock");

							std::string mission_name = mi.description;
							std::string::size_type pos1 = mission_name.find("<h1>");
							std::string::size_type pos2 = mission_name.find("</h1>");
							std::string::size_type pos3 = mission_name.find("(Survival)");
							if(pos1 != std::string::npos && pos2 != std::string::npos)
							{
								if(pos3 != std::string::npos)
								{
									pos2 = pos3;
								}
								pos1 += 4;
								mission_name = mission_name.substr(pos1, pos2 - pos1);
							}
							std::string msg = std::string(getLocaleGuiString("gui_elaboratehint_survival_mission_unlocked")) + "<br><h1>" + mission_name + "</h1> ";
							win->showMessage(msg);
						}
					}

					*lastValue = 1;
				}
				else
				{
					*lastValue = 0;
				}
#endif
			}
			break;

		case GS_CMD_setMissionAbortCounter:
			if (game->missionFailureCounter >= 0)
			{
				game->missionFailureCounter = intData * GAME_TICKS_PER_SECOND;
			}
			game->missionAborting = true;
			break;


		default:
			sp->error("MissionScripting::process - Unknown command.");
			assert(0);
		}
	}



	int MissionScripting::calculateAliveUnits(Game *game, int player)
	{
		int ret = 0;
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{
				ret++;
			}
		}
		return ret;
	}



	int MissionScripting::calculateConsciousUnits(Game *game, int player)
	{
		int ret = 0;
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed()
				&& u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
			{
				ret++;
			}
		}
		return ret;
	}



	int MissionScripting::calculateAliveHostileUnits(Game *game, int player)
	{
		int ret = 0;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed()
				&& u->getOwner() != player
				&& game->isHostile(player, u->getOwner()))
			{
				ret++;
			}
		}
		return ret;
	}



	int MissionScripting::countHostilesInRange(Game *game, const VC3 &position, int player, int range, bool consciousOnly)
	{
		int amount = 0;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed() 
				&& game->isHostile(player, u->getOwner())
				&& (u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
				|| !consciousOnly))
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if (distsq < range * range)
				{
					amount++;
				}
			}
		}
		return amount;
	}



	int MissionScripting::countFriendlysInRange(Game *game, const VC3 &position, int player, int range)
	{
		int amount = 0;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed() 
				&& !game->isHostile(player, u->getOwner()))
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if (distsq < range * range)
				{
					amount++;
				}
			}
		}
		return amount;
	}



	int MissionScripting::countOwnedInRange(Game *game, const VC3 &position, int player, int range)
	{
		int amount = 0;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed() 
				&& player == u->getOwner())
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if (distsq < range * range)
				{
					amount++;
				}
			}
		}
		return amount;
	}



	bool MissionScripting::isEveryUnitNearPosition(Game *game, int player, 
		float range, const VC3 &position)
	{
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && u->isDestroyed())
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if (!(distsq <= range * range))
				{
					return false;
				}
			}
		}
		return true;
	}



	bool MissionScripting::isAnyUnitNearPosition(Game *game, int player, 
		float range, const VC3 &position)
	{
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && u->isDestroyed())
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if ((distsq <= range * range))
				{
					return true;
				}
			}
		}
		return false;
	}



	bool MissionScripting::playerUnitsInAction(Game *game, int player)
	{
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		bool hostilesNear = false;
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{ 							
				if (u->getOwner() == player)
				{
					if (u->getSeeUnit() != NULL)
					{
						hostilesNear = true;
						break;
					}
				} else {
					if (game->isHostile(player, u->getOwner()))
					{
						if (u->getSeeUnit() != NULL
							&& u->getSeeUnit()->getOwner() == player)
						{
							hostilesNear = true;
							break;
						}
					}
				}
			}
		}

		return hostilesNear;
	}


}

