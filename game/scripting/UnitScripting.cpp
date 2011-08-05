#include "precompiled.h"

#include <string>
#include <vector>
#ifdef _WIN32
#include <malloc.h>
#endif
#include <boost/lexical_cast.hpp>

#include "UnitScripting.h"

#include "scripting_macros_start.h"
#include "unit_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "../scaledefs.h"
#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../physics/AbstractPhysicsObject.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../../ui/CombatWindow.h"
#include "../UnitInventory.h"
#include "../GameMap.h"
#include "../GameScene.h"
//#include "../HideMap.h"
#include "../CoverMap.h"
#include "../CoverFinder.h"
#include "../GameRandom.h"
#include "../WeaponObject.h"
#include "../Weapon.h"
#include "GameScripting.h"
#include "../Character.h"
#include "../UnitSpawner.h"
#include "../ItemManager.h"
#include "../UnitList.h"
#include "../UnitActor.h"
#include "../Part.h"
#include "../Item.h"
#include "../UnitLevelAI.h"
#include "../UnitType.h"
#include "../unittypes.h"
#include "../AmmoPackObject.h"
#include "../Flashlight.h"
#include "../../ui/AnimationSet.h"
#include "../../ui/Spotlight.h"
#include "../LineOfJumpChecker.h"
#include "../UnitPhysicsUpdater.h"
#include "../VisualObjectModelStorage.h"

#include <IStorm3D_Mesh.h>
#include <IStorm3D_Scene.h>

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../util/AngleRotationCalculator.h"
#include "../../util/PositionsDirectionCalculator.h"
#include "../../util/PathDeformer.h"
#include "../../util/HelperPositionCalculator.h"
#include "../../util/fb_assert.h"
#include "../../system/Logger.h"

#include "../../util/Debug_MemoryManager.h"

#include "../../ui/TargetDisplayWindowUpdator.h"
#include "../../ui/WeaponWindow.h"

#include "../PlayerWeaponry.h"
#include "../../util/StringUtil.h"
#include "../options/options_physics.h"

#define GROUP_FIND_DIST 50

#define MOVE_DIR_LEFT 1
#define MOVE_DIR_RIGHT 2
#define MOVE_DIR_FORWARD 3
#define MOVE_DIR_BACKWARD 4

#define LINEOFJUMP_TARG_AREA 3.0f


using namespace ui;

namespace game
{

	extern Bullet *gs_hitscript_hit_bullet_type;

	void UnitScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, const char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		Unit *unit = gsd->unit;
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_SETMODE:
			if (unit != NULL)
			{
				if (strcmp(stringData, "aggressive") == 0)
				{
					unit->setMode(Unit::UNIT_MODE_AGGRESSIVE);
					break;
				}
				if (strcmp(stringData, "defensive") == 0)
				{
					unit->setMode(Unit::UNIT_MODE_DEFENSIVE);
					break;
				}
				if (strcmp(stringData, "holdfire") == 0)
				{
					unit->setMode(Unit::UNIT_MODE_HOLD_FIRE);
					break;
				}
				if (strcmp(stringData, "keeptarget") == 0)
				{
					unit->setMode(Unit::UNIT_MODE_HOLD_FIRE);
					break;
				}
				sp->warning("UnitScripting::process - setMode parameter invalid.");
			} else {
				sp->warning("UnitScripting::process - Attempt to setMode for null unit.");
			}
			break;

		case GS_CMD_FINDGROUP:
			if (findGroup(game, unit))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_HASTARGET:
			if (unit != NULL)
			{
				if (unit->targeting.hasTarget())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to hasTarget for null unit.");
			}
			break;

		case GS_CMD_SETTARGET:
			if (unit != NULL)
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - setTarget parameter expected.");
					break;
				}
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					unit->targeting.clearTarget();
					unit->targeting.setTarget(gsd->spottedUnit);
					break;
				}
				if (strcmp(stringData, "idvalue") == 0)
				{
					if (*lastValue >= UNITID_LOWEST_POSSIBLE_VALUE
						&& *lastValue <= UNITID_HIGHEST_POSSIBLE_VALUE)
					{
						unit->targeting.clearTarget();
						Unit *o = game->units->getUnitById(*lastValue);
						if (o != NULL)
						{
							unit->targeting.setTarget(o);
						} else {
							sp->warning("GameScripting::process - setTarget idvalue, no unit found with given id number.");
						}
					} else {
						sp->warning("GameScripting::process - setTarget idvalue, value is not a valid id number.");
					}
					break;
				}
				if (strcmp(stringData, "idstring") == 0)
				{
					if (gsd->stringValue != NULL)
					{
						unit->targeting.clearTarget();
						Unit *o = game->units->getUnitByIdString(gsd->stringValue);
						if (o != NULL)
						{
							unit->targeting.setTarget(o);
						} else {

							std::string temp = "GameScripting::process - setTarget idvalue, no unit found with given id string: ";
							temp += std::string( gsd->stringValue );
							sp->warning( temp.c_str() );
						
						}
					} else {
						sp->error("UnitScripting::process - setTarget called with idstring parameter, but string value is null.");
					}
					break;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					unit->targeting.clearTarget();
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						unit->targeting.setTarget(gsd->alertUnit->targeting.getTargetUnit());
					}
					break;
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					unit->targeting.clearTarget();
					unit->targeting.setTarget(gsd->alertUnit);
					break;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					unit->targeting.clearTarget();
					unit->targeting.setTarget(gsd->shooter);
					break;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					unit->targeting.clearTarget();
					unit->targeting.setTarget(gsd->noisy);
					break;
				}
				if (strcmp(stringData, "position") == 0)
				{
					unit->targeting.clearTarget();
					unit->targeting.setTarget(gsd->position);
					break;
				}
				if (strcmp(stringData, "nearest_to_position") == 0)
				{
					unit->targeting.clearTarget();
					LinkedList *ntplist = game->units->getAllUnits();
					LinkedListIterator ntpiter(ntplist);
					Unit *closest = NULL;
					float closestDistSq = 0;
					while (ntpiter.iterateAvailable())
					{
						Unit *ntpu = (Unit *)ntpiter.iterateNext();
						if (ntpu != unit && ntpu->isActive() && !ntpu->isDestroyed())
						{
							VC3 distVec = ntpu->getPosition() - unit->getPosition();
							if (closest == NULL || 
								distVec.GetSquareLength() < closestDistSq)
							{
								closest = ntpu;
								closestDistSq = distVec.GetSquareLength();
							}
						}
					}
					if (closest != NULL)
						unit->targeting.setTarget(closest); 
					break;
				}
				if (strcmp(stringData, "nearest_of_player_to_position") == 0)
				{
					unit->targeting.clearTarget();
					LinkedList *ntplist = game->units->getOwnedUnits(gsd->player);
					LinkedListIterator ntpiter(ntplist);
					Unit *closest = NULL;
					float closestDistSq = 0;
					while (ntpiter.iterateAvailable())
					{
						Unit *ntpu = (Unit *)ntpiter.iterateNext();
						if (ntpu != unit && ntpu->isActive() && !ntpu->isDestroyed())
						{
							VC3 distVec = ntpu->getPosition() - unit->getPosition();
							if (closest == NULL || 
								distVec.GetSquareLength() < closestDistSq)
							{
								closest = ntpu;
								closestDistSq = distVec.GetSquareLength();
							}
						}
					}
					if (closest != NULL)
						unit->targeting.setTarget(closest); 
					break;
				}
				sp->warning("UnitScripting::process - setTarget parameter invalid.");
			} else {
				sp->warning("UnitScripting::process - Attempt to setTarget for null unit.");
			}
			break;

		case GS_CMD_REPATH:
			sp->error("UnitScripting::process - repath TODO.");
			break;

		case GS_CMD_PATHTO:
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;

					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					VC3 oldPos = gsd->position;
					gsd->position = VC3(
						x, game->gameMap->getScaledHeightAt(x,y), y);
					if (unit != NULL)
					{
						UnitActor *ua = getUnitActorForUnit(unit);
						if (ua != NULL)
						{
							frozenbyte::ai::Path *path = ua->solvePath(unit, oldPos, 
								gsd->position);
							// notice: gsd->position may have been changed by getPath
							// if it was blocked.
							unit->setStoredPath(0, path, oldPos, gsd->position);
							if (path == NULL)
							{
								sp->debug("UnitScripting::process - pathTo failed to find path.");
								*lastValue = 0;
							} else {
								*lastValue = 1;
							}
						}
					}
				} else {
					sp->error("UnitScripting::process - Missing or bad pathTo parameter.");
				}
			}
			break;

		case GS_CMD_PATHTOPOSITION:
			{
				if (unit != NULL)
				{
					UnitActor *ua = getUnitActorForUnit(unit);
					if (ua != NULL)
					{
						frozenbyte::ai::Path *path = ua->solvePath(unit, unit->getPosition(), 
							gsd->position, intData);
						// notice: gsd->position may have been changed by getPath
						// if it was blocked.
						unit->setStoredPath(0, path, unit->getPosition(), gsd->position);
						if (path == NULL)
						{
							sp->debug("UnitScripting::process - pathTo failed to find path.");
							*lastValue = 0;
						} else {
							*lastValue = 1;
						}
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to pathToPosition for null unit.");
				}
			}
			break;

		case GS_CMD_STOREPATH:
			if (unit != NULL)
			{
				if (intData == 0)
				{
					gsd->lastStoredPath = 0;
				} else {
					if (intData < 0)
					{
						gsd->lastStoredPath++;
						unit->scriptPaths.moveStoredPath(0, gsd->lastStoredPath);
						assert(!"deprecated storePath -1 command");
					} else {
						unit->scriptPaths.moveStoredPath(0, intData);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to storePath for null unit.");
			}
			break;

		case GS_CMD_STORENEXTPATH:
			if (unit != NULL)
			{
				gsd->lastStoredPath++;
				unit->scriptPaths.moveStoredPath(0, gsd->lastStoredPath);
			} else {
				sp->warning("UnitScripting::process - Attempt to storeNextPath for null unit.");
			}
			break;

		case GS_CMD_PATHNAME:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					gsd->setStorePathName(stringData);
				} else {
					sp->error("UnitScripting::process - pathName parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to pathName for null unit.");
			}
			break;

		case GS_CMD_GROUPCENTERPOSITION:
			if (unit != NULL)
			{
				if (unit->getLeader() != NULL)
					gsd->position = unit->getLeader()->getPosition();
				else
					gsd->position = unit->getPosition();
			} else {
				sp->warning("UnitScripting::process - Attempt to groupCenter for null unit.");
			}
			break;

		case GS_CMD_MOVETOPATH:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);
				if (ua != NULL)
				{
					VC3 upos = unit->getPosition();
					VC3 startpos = unit->scriptPaths.getStoredPathStartPosition(unit->scriptPaths.getStoredPathNumber());
					
					// NOTE: precalced paths disabled!!!!

					/*
					if (fabs(startpos.x-upos.x) > unitType->getPathAccuracy()
						|| fabs(startpos.z-upos.z) > unitType->getPathAccuracy()
						|| unit->scriptPaths.isStoredPathEnd(unit->scriptPaths.getStoredPathNumber())
						|| unit->scriptPaths.isStoredPathStart(unit->scriptPaths.getStoredPathNumber()))
					{
					*/
						//sp->debug("dynamic path");
						bool success = ua->setPathTo(unit, unit->scriptPaths.getStoredPathEndPosition(unit->scriptPaths.getStoredPathNumber()));
						unit->setLastPathfindSuccess(success);
					/*
					} else {
						//sp->debug("precalced path");
						VC3 endpos = unit->scriptPaths.getStoredPathEndPosition(unit->scriptPaths.getStoredPathNumber());
						unit->useStoredPath(unit->scriptPaths.getStoredPathNumber());
						if (!unit->isAtPathEnd())
						{
							unit->setPathIndex(unit->getPathIndex() + 1);
							unit->setWaypoint(unit->getPosition());
							unit->setFinalDestination(endpos);
							unit->setLastPathfindSuccess(true);
						} else {
							unit->setLastPathfindSuccess(false);
						}
					}
					*/
				} else {
					sp->error("UnitScripting::process - Unable to get actor for unit in moveToPath.");
				}
				gsd->waitDestination = true;
				gsd->waitCounter = 10; // 1 sec? or 0.1 sec? or 100/67 sec?
				
			} else {
				sp->warning("UnitScripting::process - Attempt to moveToPath for null unit.");
			}
			*pause = true;
			break;

		case GS_CMD_MOVETOPOSITION:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);
				if (ua != NULL)
				{
					bool success = ua->setPathTo(unit, gsd->position);
					unit->setLastPathfindSuccess(success);
				} 			 
				gsd->waitDestination = true;
				gsd->waitCounter = 100; // 1 sec? or 0.1 sec? or 100/67 sec?
			} else {
				sp->warning("UnitScripting::process - Attempt to moveToPosition for null unit.");
			}
			*pause = true;
			break;

		case GS_CMD_MOVETOPOSITIONNOWAIT:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);
				if (ua != NULL)
				{
					bool success = ua->setPathTo(unit, gsd->position);
					unit->setLastPathfindSuccess(success);
				} 			 
			} else {
				sp->warning("UnitScripting::process - Attempt to moveToPositionNoWait for null unit.");
			}
			// WARNING: COMMENTED THIS OUT, CAN IT CAUSE PROBLEMS?
			// WHY WAS IT HERE BEFORE?
			//*pause = true;
			break;

		case GS_CMD_NEXTPATH:
			if (unit != NULL)
			{
				int num = unit->scriptPaths.getStoredPathNumber() + 1;
				unit->scriptPaths.setStoredPathNumber(num);
				gsd->position = unit->scriptPaths.getStoredPathStartPosition(num);
			} else {
				sp->warning("UnitScripting::process - Attempt to nextPath for null unit.");
			}
			break;

		case GS_CMD_SETSPEED:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					if (unit->getJumpCounter() == 0)
					{
						if (unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
						{
							bool speedok = false;
							if (strcmp(stringData, "slow") == 0)
							{
								if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								{
									unit->setMoveState(Unit::UNIT_MOVE_STATE_RISE_PRONE);
									unit->setMoveStateCounter(100); // 1 sec
								}
								unit->setSpeed(Unit::UNIT_SPEED_SLOW);
								speedok = true;
							}
							if (strcmp(stringData, "fast") == 0)
							{
								if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								{
									unit->setMoveState(Unit::UNIT_MOVE_STATE_RISE_PRONE);
									unit->setMoveStateCounter(100); // 1 sec
								}
								unit->setSpeed(Unit::UNIT_SPEED_FAST);
								speedok = true;
							}
							if (strcmp(stringData, "crawl") == 0)
							{
								if (unit->getAnimationSet() != NULL
									&& unit->getAnimationSet()->isAnimationInSet(ANIM_PRONE))
								{
									unit->setMoveState(Unit::UNIT_MOVE_STATE_GO_PRONE);
									unit->setMoveStateCounter(100); // 1 sec
									unit->setSpeed(Unit::UNIT_SPEED_CRAWL);
								} else {
									sp->warning("UnitScripting::process - setSpeed, attempt to set crawl speed when no proper animation.");
								}
								speedok = true;
							}
							if (strcmp(stringData, "sprint") == 0)
							{
								if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								{
									unit->setMoveState(Unit::UNIT_MOVE_STATE_RISE_PRONE);
									unit->setMoveStateCounter(100); // 1 sec
								}
								unit->setSpeed(Unit::UNIT_SPEED_SPRINT);
								speedok = true;
							}
							if (strcmp(stringData, "jump") == 0)
							{
								if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								{
									unit->setMoveState(Unit::UNIT_MOVE_STATE_RISE_PRONE);
									unit->setMoveStateCounter(100); // 1 sec
								}
								unit->setSpeed(Unit::UNIT_SPEED_JUMP);
								speedok = true;
							}
							if (!speedok)
							{
								sp->error("UnitScripting::process - setSpeed parameter invalid.");
							}
						}
					} else {
						sp->debug("UnitScripting::process - setSpeed ignored as unit is currently jumping.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to setSpeed for null unit.");
				}
			} else {
				sp->warning("UnitScripting::process - setSpeed parameter missing.");
			}
			break;

		case GS_CMD_TARGETCLOSERTO:
			if (unit != NULL)
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - targetCloserTo parameter expected.");
					break;
				}
				Unit *u2 = NULL;
				float dist1 = 999999;
				float dist2 = 999999;
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					u2 = gsd->spottedUnit;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						u2 = gsd->alertUnit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					u2 = gsd->alertUnit;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					u2 = gsd->shooter;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					u2 = gsd->noisy;
				}
				// if "position" special case, handled seperately below...

				if (unit->targeting.hasTarget()) 
				{
					VC3 pos1;
					Unit *u1 = unit->targeting.getTargetUnit();
					VC3 tmp;
					if (u1 != NULL)
					{
						tmp = u1->getPosition() - unit->getPosition();
					} else {
						tmp = unit->targeting.getTargetPosition() - unit->getPosition();
					}
					dist1 = tmp.GetSquareLength();
				}
				if (u2 != NULL) 
				{
					VC3 tmp = u2->getPosition() - unit->getPosition();
					dist2 = tmp.GetSquareLength();
				} else {
					if (strcmp(stringData, "position") == 0)
					{
						VC3 tmp = gsd->position - unit->getPosition();
						dist2 = tmp.GetSquareLength();
					} else {
						sp->warning("UnitScripting::process - targetCloserTo parameter invalid.");
					}
				}
				if (dist1 < dist2)
					*lastValue = 1;
				else
					*lastValue = 0;
				break;
			} else {
				sp->warning("UnitScripting::process - Attempt to targetCloserTo for null unit.");
			}
			break;

		case GS_CMD_UNITPOSITION:
			if (unit != NULL)
			{
				gsd->position = unit->getPosition();
			} else {
				sp->warning("UnitScripting::process - Attempt to unitPosition for null unit.");
			}
			break;

		case GS_CMD_TURNTOWARD:
			if (unit != NULL)
			{
				unit->setTurnToAngle(unit->getAngleTo(gsd->position));
			}
			break;

		case GS_CMD_getRelativeAngleToPosition:
			if (unit != NULL)
			{
				float posangle = unit->getAngleTo(gsd->position);
				float curangle = unit->getRotation().y; 
				float relRot = util::AngleRotationCalculator::getFactoredRotationForAngles(curangle, posangle, 0.0f);
				*lastValue = (int)(relRot);
			}
			break;

		case GS_CMD_TARGETPOSITION:
			if (unit != NULL)
			{
				if (unit->targeting.hasTarget())
				{
					if (unit->targeting.getTargetUnit() != NULL)
						gsd->position = unit->targeting.getTargetUnit()->getPosition();
					else
						gsd->position = unit->targeting.getTargetPosition();
				}
			}
			break;

		case GS_CMD_TURNBYANGLE:
			if (unit != NULL)
			{
				float tmpangle = unit->getRotation().y + (float)intData;
				if (tmpangle < 0) tmpangle += 360;
				if (tmpangle >= 360) tmpangle -= 360;
				unit->setTurnToAngle(tmpangle);
			}
			break;

		case GS_CMD_CHOOSEPATH:
			if (unit != NULL)
			{
				//if (intData == 0)
				//{
				//	sp->warning("UnitScripting::process - choosePath with zero parameter, probably unintended.");
				//}
				if (intData == 0)
				{
					unit->scriptPaths.setStoredPathNumber(*lastValue);
					sp->warning("UnitScripting::process - choosePath with zero parameter deprecated (use choosePathByValue instead).");
				} else {
					unit->scriptPaths.setStoredPathNumber(intData);
				}
			}
			break;

		case GS_CMD_CHOOSEPATHBYVALUE:
			if (unit != NULL)
			{
				unit->scriptPaths.setStoredPathNumber(*lastValue);
			}
			break;

		case GS_CMD_PATHSTART:
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					gsd->position = VC3(
						x, game->gameMap->getScaledHeightAt(x,y), y);
					if (unit != NULL)
					{
						unit->scriptPaths.setStoredPathStart(0, gsd->position, unit, gsd->storePathName);
					}
				} else {
					sp->error("UnitScripting::process - Missing or bad pathStart parameter.");
				}
			}
			break;

		case GS_CMD_PATHEND:
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					gsd->position = VC3(
						x, game->gameMap->getScaledHeightAt(x,y), y);
					if (unit != NULL)
					{
						unit->scriptPaths.setStoredPathEnd(0, gsd->position, unit);
					}
				} else {
					sp->error("UnitScripting::process - Missing or bad pathEnd parameter.");
				}
			}
			break;

		case GS_CMD_ATPATHEND:
			if (unit != NULL)
			{
				if (unit->scriptPaths.isStoredPathEnd(unit->scriptPaths.getStoredPathNumber()))
					*lastValue = 1;
				else
					*lastValue = 0;
			}
			break;

		case GS_CMD_ALERT:
			if (intData > 0)
			{
				if (unit != NULL)
				{
					game->gameScripting->makeAlert(unit, intData, unit->getPosition());
				} else {
					sp->warning("UnitScripting::process - Attempt to call alert for null unit.");
				}
			} else {
				sp->error("UnitScripting::process - alert value out of range.");
			}
			break;


		case GS_CMD_ALERTATPOSITION:
			if (intData > 0)
			{
				if (unit != NULL)
				{
					game->gameScripting->makeAlert(unit, intData, gsd->position);
				} else {
					sp->warning("UnitScripting::process - Attempt to call alertAtPosition for null unit.");
				}
			} else {
				sp->error("UnitScripting::process - alertAtPosition value out of range.");
			}
			break;

		case GS_CMD_HASGROUP:
			if (unit != NULL)
			{
				Unit *leader = unit->getLeader();
				if (leader != NULL
					&& leader->isActive()
					&& !leader->isDestroyed())
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to hasGroup for null unit.");
			}
			break;

		case GS_CMD_ADAPTGROUPSCRIPT:
			if (unit != NULL)
			{
				Unit *leader = unit->getLeader();
				if (leader != NULL
					&& leader->isActive()
					&& !leader->isDestroyed())
				{
					bool reScript = false;
					if (leader->getScript() == NULL)
					{
						if (unit->getScript() != NULL)
						{
							unit->setScript(NULL);
							reScript = true;
						}
					} else {
						if (unit->getScript() == NULL
							|| strcmp(unit->getScript(), leader->getScript()) != 0)
						{
							unit->setScript(leader->getScript());
							reScript = true;
						}
					}

					if (reScript)
					{
						// WARNING: unsafe cast!
						UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
						ai->requestReScriptMain();
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}

				} else {
					sp->debug("UnitScripting::process - adaptGroupScript for unit with no leader.");
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to adaptGroupScript for null unit.");
			}
			break;

		case GS_CMD_RESCRIPTMAIN:
			if (unit != NULL)
			{
				// WARNING: unsafe cast
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				assert(ai != NULL);
				ai->requestReScriptMain();
				sp->warning("UnitScripting::process - reScriptMain, main script will restart once the current main script process finishes.");
			} else {
				sp->warning("UnitScripting::process - Attempt to reScriptMain for null unit.");
			}
			break;

		case GS_CMD_TERMINATEMAINSCRIPT:
			if (unit != NULL)
			{
				// WARNING: unsafe cast
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				assert(ai != NULL);
				if (!ai->isScriptProcessMainScriptProcess(sp))
				{
					sp->warning("UnitScripting::process - terminateMainScript, main script process will be immediately terminated.");
					sp->warning("Unexpected behaviour may appear due to unclean exit.");
					UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
					assert(ai != NULL);
					ai->terminateMainScript();
				} else {
					sp->warning("UnitScripting::process - terminateMainScript, main script self termination not allowed.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to terminateScriptMain for null unit.");
			}
			break;

		case GS_CMD_CHANGEMAINSCRIPT:
			if (unit != NULL)
			{
				if (stringData == NULL 
					|| stringData[0] == '\0')
				{
					unit->setScript(NULL);
				} else {
					unit->setScript(stringData);
				}
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				assert(ai != NULL);
				ai->requestReScriptMain();
				// enough warnings already..
				//sp->warning("UnitScripting::process - changeMainScript, main script will change once the current main script process finishes.");
			} else {
				sp->warning("UnitScripting::process - Attempt to changeMainScript for null unit.");
			}
			break;

		case GS_CMD_CHOOSEGROUPPATH:
			if (unit != NULL)
			{
				Unit *leader = unit->getLeader();
				if (leader != NULL
					&& leader->isActive()
					&& !leader->isDestroyed())
				{
					if (unit->getScript() != NULL
						&& leader->getScript() != NULL
						&& strcmp(unit->getScript(), leader->getScript()) == 0)
					{
						unit->scriptPaths.setStoredPathNumber(leader->scriptPaths.getStoredPathNumber());
						*lastValue = 1;
					} else {
						sp->debug("UnitScripting::process - groupPath for unit running different script than leader.");
						*lastValue = 0;
					}
				} else {
					sp->debug("UnitScripting::process - groupPath for unit with no leader.");
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to hasGroup for null unit.");
			}
			break;

		case GS_CMD_POSITIONCLOSERTHAN:
		case GS_CMD_POSITIONCLOSER:
			int val;
			if (command == GS_CMD_POSITIONCLOSER)
				val = *lastValue;
			else
				val = intData;
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				float dist1 = tmp.GetSquareLength();
				if (dist1 < val * val)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to positionCloser or positionCloserThan for null unit.");
			}
			break;

		case GS_CMD_DISTANCETOPOSITION:
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				float dist = tmp.GetLength();
				*lastValue = (int)dist;
			} else {
				sp->warning("UnitScripting::process - Attempt to distanceToPosition for null unit.");
			}
			break;

		case GS_CMD_ACCURATEDISTANCETOPOSITION:
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				float dist = tmp.GetLength() * 100.0f;
				*lastValue = (int)dist;
			} else {
				sp->warning("UnitScripting::process - Attempt to accurateDistanceToPosition for null unit.");
			}
			break;

		case GS_CMD_INTERPOLATEPOSITION:
			if (unit != NULL)
			{
				if (intData > 100)
				{
					sp->debug("UnitScripting::process - interpolatePosition parameter over 100, thus extrapolating.");
				}
				VC3 tmp = gsd->position - unit->getPosition();
				float dist = tmp.GetLength();
				dist *= ((float)intData / 100.0f);
				if (tmp.GetSquareLength() > 0.0001f)
				{
					tmp.Normalize();
				}
				gsd->position = unit->getPosition() + tmp * dist;
			} else {
				sp->warning("UnitScripting::process - Attempt to interpolatePosition for null unit.");
			}
			break;

		case GS_CMD_interpolatePositionByValue:
			if (unit != NULL)
			{
				if (*lastValue > 100)
				{
					sp->debug("UnitScripting::process - interpolatePositionByValue value over 100, thus extrapolating.");
				}
				VC3 tmp = gsd->position - unit->getPosition();
				float dist = tmp.GetLength();
				dist *= ((float)(*lastValue) / 100.0f);
				if (tmp.GetSquareLength() > 0.0001f)
				{
					tmp.Normalize();
				}
				gsd->position = unit->getPosition() + tmp * dist;
			} else {
				sp->warning("UnitScripting::process - Attempt to interpolatePositionByValue for null unit.");
			}
			break;

		case GS_CMD_ABSOLUTEINTERPOLATEPOSITION:
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				if (tmp.GetSquareLength() > 0.0001f)
				{
					tmp.Normalize();
				}
				tmp *= (float)intData;
				gsd->position = unit->getPosition() + tmp;
			} else {
				sp->warning("UnitScripting::process - Attempt to absoluteInterpolatePosition for null unit.");
			}
			break;

		case GS_CMD_MOVETOPOSITIONCUT:
			if (unit != NULL)
			{
				if (intData > 100 || intData <= 0)
				{
					sp->error("UnitScripting::process - moveToPositionCut paremeter out of range.");
				} else {
					UnitActor *ua = getUnitActorForUnit(unit);
					if (ua != NULL)
					{
						// this could be optimized!
						// (now we copy part of the full path to another path)
						frozenbyte::ai::Path *fullpath = ua->solvePath(unit, unit->getPosition(), gsd->position);
						int copyAmount = 0;
						if (fullpath != NULL)
						{
							int lastX = fullpath->getPointX(0);
							int lastY = fullpath->getPointY(0);
							float fullpathSize = 0;
							int i;
							for (i = 1; i < fullpath->getSize(); i++)
							{
								int nextX = fullpath->getPointX(i);
								int nextY = fullpath->getPointY(i);
								fullpathSize += sqrtf((float)(((nextX - lastX) * (nextX - lastX)) + ((nextY - lastY) * (nextY - lastY))));
								lastX = nextX;
								lastY = nextY;
							}
							copyAmount = 1;
							lastX = fullpath->getPointX(0);
							lastY = fullpath->getPointY(0);
							float cutpathSize = 0;
							for (i = 1; i < fullpath->getSize(); i++)
							{
								int nextX = fullpath->getPointX(i);
								int nextY = fullpath->getPointY(i);
								cutpathSize += sqrtf((float)(((nextX - lastX) * (nextX - lastX)) + ((nextY - lastY) * (nextY - lastY))));
								copyAmount++;
								lastX = nextX;
								lastY = nextY;
								if (cutpathSize >= (fullpathSize * intData) / 100)
									break;
							}
							//copyAmount = (fullpath->getSize() * intData) / 100;
						}
						if (copyAmount > 0)
						{
							frozenbyte::ai::Path *partialpath = new frozenbyte::ai::Path();
							for (int i = copyAmount - 1; i >= 0; i--)
							{
								partialpath->addPoint(fullpath->getPointX(i), fullpath->getPointY(i));
							}
							VC3 lastPos = VC3(game->gameMap->pathfindToScaledX(partialpath->getPointX(copyAmount - 1)),
								0, game->gameMap->pathfindToScaledX(partialpath->getPointY(copyAmount - 1)));
							//VC3 lastPos = VC3(game->gameMap->pathfindToScaledX(partialpath->getPointX(0)),
							//	0, game->gameMap->pathfindToScaledX(partialpath->getPointY(0)));
							unit->setPath(partialpath);
							unit->setPathIndex(unit->getPathIndex() + 1);
							// (...path object is now contained within the unit, 
							// unit will handle it's proper deletion)
							unit->setWaypoint(unit->getPosition());
							unit->setFinalDestination(VC3(lastPos.x, 
								game->gameMap->getScaledHeightAt(lastPos.x, lastPos.z), 
								lastPos.z));
							unit->setLastPathfindSuccess(true);
						} else {
							unit->setPath(NULL);
							unit->setFinalDestination(unit->getPosition());
							unit->setWaypoint(unit->getPosition());
							unit->setLastPathfindSuccess(false);
						}
						if (fullpath != NULL)
						{
							delete fullpath;
						}
					} 			 
					gsd->waitDestination = true;
					gsd->waitCounter = 50; // 1 sec? or 0.1 sec? or 100/67 sec?
					*pause = true;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to moveToPositionCut for null unit.");
			}
			break;

		case GS_CMD_MOVETOPOSITIONCUTNOWAIT:
			if (unit != NULL)
			{
				if (intData > 100 || intData <= 0)
				{
					sp->error("UnitScripting::process - moveToPositionCutNoWait paremeter out of range.");
				} else {
					UnitActor *ua = getUnitActorForUnit(unit);
					if (ua != NULL)
					{
						// this could be optimized!
						// (now we copy part of the full path to another path)
						frozenbyte::ai::Path *fullpath = ua->solvePath(unit, unit->getPosition(), gsd->position);
						int copyAmount = 0;
						if (fullpath != NULL)
						{
							int lastX = fullpath->getPointX(0);
							int lastY = fullpath->getPointY(0);
							float fullpathSize = 0;
							int i;
							for (i = 1; i < fullpath->getSize(); i++)
							{
								int nextX = fullpath->getPointX(i);
								int nextY = fullpath->getPointY(i);
								fullpathSize += sqrtf((float)(((nextX - lastX) * (nextX - lastX)) + ((nextY - lastY) * (nextY - lastY))));
								lastX = nextX;
								lastY = nextY;
							}
							copyAmount = 1;
							lastX = fullpath->getPointX(0);
							lastY = fullpath->getPointY(0);
							float cutpathSize = 0;
							for (i = 1; i < fullpath->getSize(); i++)
							{
								int nextX = fullpath->getPointX(i);
								int nextY = fullpath->getPointY(i);
								cutpathSize += sqrtf((float)(((nextX - lastX) * (nextX - lastX)) + ((nextY - lastY) * (nextY - lastY))));
								copyAmount++;
								lastX = nextX;
								lastY = nextY;
								if (cutpathSize >= (fullpathSize * intData) / 100)
									break;
							}
							//copyAmount = (fullpath->getSize() * intData) / 100;
						}
						if (copyAmount > 0)
						{
							frozenbyte::ai::Path *partialpath = new frozenbyte::ai::Path();
							for (int i = copyAmount - 1; i >= 0; i--)
							{
								partialpath->addPoint(fullpath->getPointX(i), fullpath->getPointY(i));
							}
							VC3 lastPos = VC3(game->gameMap->pathfindToScaledX(partialpath->getPointX(copyAmount - 1)),
								0, game->gameMap->pathfindToScaledY(partialpath->getPointY(copyAmount - 1)));
							//VC3 lastPos = VC3(game->gameMap->pathfindToScaledX(partialpath->getPointX(0)),
							//	0, game->gameMap->pathfindToScaledX(partialpath->getPointY(0)));
							unit->setPath(partialpath);
							unit->setPathIndex(unit->getPathIndex() + 1);
							// (...path object is now contained within the unit, 
							// unit will handle it's proper deletion)
							unit->setWaypoint(unit->getPosition());
							unit->setFinalDestination(VC3(lastPos.x, 
								game->gameMap->getScaledHeightAt(lastPos.x, lastPos.z), 
								lastPos.z));
							unit->setLastPathfindSuccess(true);
						} else {
							unit->setPath(NULL);
							unit->setFinalDestination(unit->getPosition());
							unit->setWaypoint(unit->getPosition());
							unit->setLastPathfindSuccess(false);
						}
						if (fullpath != NULL)
						{
							delete fullpath;
						}
					} 			 
					//gsd->waitDestination = true;
					//gsd->waitCounter = 100; // 1 sec? or 0.1 sec? or 100/67 sec?
					//*pause = true;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to moveToPositionCutNoWait for null unit.");
			}
			break;

		case GS_CMD_SOLVEPATHNUMBERBYORDER:
			if (unit != NULL)
			{
				if (intData >= 0)
				{
					int seeking = intData;
					if (seeking == 0)
						seeking = *lastValue;
					int allocedPaths = unit->scriptPaths.getAllocatedStoredPaths();
					int atOrder = 0;
					*lastValue = 0;
					for (int i = 1; i < allocedPaths; i++)
					{
						if (unit->scriptPaths.isStoredPathUsed(i)
							&& unit->scriptPaths.isStoredPathStart(i))
						{
							atOrder++;
							if (atOrder == seeking) 
							{
								*lastValue = i;
								break;
							}
						}
					}
					if (*lastValue == 0)
					{
						*lastValue = 1;
						sp->debug("UnitScripting::process - solvePathNumberByOrder failed to find path.");
					}
				} else {
					if (intData == -1)
					{
						int allocedPaths = unit->scriptPaths.getAllocatedStoredPaths();
						*lastValue = 0;
						for (int i = allocedPaths - 1; i >= 1; i--)
						{
							if (unit->scriptPaths.isStoredPathUsed(i)
								&& unit->scriptPaths.isStoredPathEnd(i))
							{
								*lastValue = i + 1;
								break;
							}
						}
						if (*lastValue == allocedPaths)
						{
							*lastValue = 1;
							sp->debug("UnitScripting::process - solvePathNumberByOrder failed to find free path.");
						} 					 
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to solvePathNumberByOrder for null unit.");
			}
			break;

		case GS_CMD_SOLVEPATHNUMBERBYNAME:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					int allocedPaths = unit->scriptPaths.getAllocatedStoredPaths();
					*lastValue = 0;
					for (int i = 1; i < allocedPaths; i++)
					{
						if (unit->scriptPaths.isStoredPathUsed(i)
							&& unit->scriptPaths.isStoredPathStart(i))
						{
							if (unit->scriptPaths.getStoredPathName(i) != NULL
								&& strcmp(stringData, unit->scriptPaths.getStoredPathName(i)) == 0)
							{
								*lastValue = i;
								break;
							}
						}
					}
					// rather no spam...
					//if (*lastValue == 0)
					//{
						//sp->debug("UnitScripting::process - solvePathNumberByName failed to find path.");
					//}
				} else {
					*lastValue = 0;
					sp->error("UnitScripting::process - solvePathNumberByName parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to solvePathNumberByName for null unit.");
			}
			break;

		case GS_CMD_HEALTHPERCENTAGE:
			if (unit != NULL)
			{
				*lastValue = (unit->getHP() * 100) / unit->getMaxHP();
				/*
				if (unit->getRootPart() != NULL
					&& unit->getRootPart()->getType()->getMaxDamage() > 0)
				{
					*lastValue = 100 - (100 * unit->getRootPart()->getDamage() 
						/ unit->getRootPart()->getType()->getMaxDamage());
				} else {
					*lastValue = 0;
				}
				*/
			} else {
				sp->warning("UnitScripting::process - Attempt to healthPercentage for null unit.");
			}
			break;


		case GS_CMD_setHealthPercentageToValue:
			if (unit != NULL)
			{
				unit->setHP((*lastValue) * unit->getMaxHP() / 100);
				/*
				if (unit->getRootPart() != NULL
					&& unit->getRootPart()->getType()->getMaxDamage() > 0)
				{
					*lastValue = 100 - (100 * unit->getRootPart()->getDamage() 
						/ unit->getRootPart()->getType()->getMaxDamage());
				} else {
					*lastValue = 0;
				}
				*/
			} else {
				sp->warning("UnitScripting::process - Attempt to setHealthPercentage for null unit.");
			}
			break;

		case GS_CMD_resetUnitNormalState:
			if (unit != NULL)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
			} else {
				sp->warning("UnitScripting::process - Attempt to resetUnitNormalState for null unit.");
			}
			break;

		case GS_CMD_GETUNITHP:
			if (unit != NULL)
			{
				*lastValue = unit->getHP();
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitHP for null unit.");
			}
			break;

		case GS_CMD_SETUNITHPTOVALUE:
			if (unit != NULL)
			{
				unit->setHP(*lastValue);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitHPToValue for null unit.");
			}
			break;

		case GS_CMD_HEATPERCENTAGE:
			if (unit != NULL)
			{
				if (unit->getMaxHeat() > 0)
					*lastValue = (100 * unit->getHeat() / unit->getMaxHeat());
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to heatPercentage for null unit.");
			}
			break;

		case GS_CMD_AMMOPERCENTAGE:
			if (unit != NULL)
			{
				*lastValue = -1;
				for (int weap = 0; weap < UNIT_MAX_WEAPONS; weap++)
				{
					if (unit->getWeaponType(weap) != NULL)
					{
						if (unit->getWeaponMaxAmmoAmount(weap) > 0)
						{
							int ammoperc = (100 * unit->getWeaponAmmoAmount(weap) 
								/ unit->getWeaponMaxAmmoAmount(weap));
							if (ammoperc > *lastValue)
								*lastValue = ammoperc;
						} 						 
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to heatPercentage for null unit.");
			}
			break;

		case GS_CMD_ISHOSTILE:
			if (unit != NULL)
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - isHostile parameter expected.");
					break;
				}
				Unit *u2 = NULL;
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					u2 = gsd->spottedUnit;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						u2 = gsd->alertUnit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					u2 = gsd->alertUnit;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					u2 = gsd->shooter;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					u2 = gsd->noisy;
				}
				if (u2 != NULL)
				{
					if (game->isHostile(unit->getOwner(), u2->getOwner()))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->warning("UnitScripting::process - isHostile parameter invalid.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isHostile for null unit.");
			}
			break;

		case GS_CMD_OTHERUNITPOSITION:
			if (unit != NULL)
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - otherUnitPosition parameter expected.");
					break;
				}
				Unit *u2 = NULL;
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					u2 = gsd->spottedUnit;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						u2 = gsd->alertUnit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					u2 = gsd->alertUnit;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					u2 = gsd->shooter;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					u2 = gsd->noisy;
				}
				if (u2 != NULL)
				{
					gsd->position = u2->getPosition();
				} else {
					sp->warning("UnitScripting::process - otherUnitPosition parameter invalid or null unit.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to otherUnitPosition for null unit.");
			}
			break;

		case GS_CMD_DESTINATIONPOSITION:
			if (unit != NULL)
			{
				gsd->position = unit->getFinalDestination();
			} else {
				sp->warning("UnitScripting::process - Attempt to destinationPosition for null unit.");
			}
			break;

		case GS_CMD_FINDCLOSESTPATH:
			if (unit != NULL)
			{
				int allocedPaths = unit->scriptPaths.getAllocatedStoredPaths();
				*lastValue = 0;
				float closest = -1;
				for (int i = 1; i < allocedPaths; i++)
				{
					if (unit->scriptPaths.isStoredPathUsed(i)
						&& !unit->scriptPaths.isStoredPathEnd(i))
					{
						VC3 pos = unit->scriptPaths.getStoredPathStartPosition(i);
						pos -= unit->getPosition();
						float diff = pos.GetSquareLength();
						if (closest < 0 || diff < closest)
						{
							closest = diff;
							*lastValue = i;
						}
					}
				}
				if (*lastValue == 0)
				{
					*lastValue = 1;
					sp->debug("UnitScripting::process - findClosestPath failed to find closest path.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to findClosestPath for null unit.");
			}
			break;

		case GS_CMD_ISUNITDESTROYED:
			if (unit != NULL)
			{
				if (unit->isDestroyed())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitDestroyed for null unit.");
			}
			break;

		case GS_CMD_ISMODE:
			if (unit != NULL)
			{
				*lastValue = 0;
				if (strcmp(stringData, "aggressive") == 0)
				{
					if (unit->getMode() == Unit::UNIT_MODE_AGGRESSIVE)
						*lastValue = 1;
					break;
				}
				if (strcmp(stringData, "defensive") == 0)
				{
					if (unit->getMode() == Unit::UNIT_MODE_DEFENSIVE)
						*lastValue = 1;
					break;
				}
				if (strcmp(stringData, "holdfire") == 0)
				{
					if (unit->getMode() == Unit::UNIT_MODE_HOLD_FIRE)
						*lastValue = 1;
					break;
				}
				if (strcmp(stringData, "keeptarget") == 0)
				{
					if (unit->getMode() == Unit::UNIT_MODE_KEEP_TARGET)
						*lastValue = 1;
					break;
				}
				sp->warning("UnitScripting::process - isMode parameter invalid.");
			} else {
				sp->warning("UnitScripting::process - Attempt to isMode for null unit.");
			}
			break;

		case GS_CMD_STOPMOVEMENT:
			if (unit != NULL)
			{
				unit->setPath(NULL);
				unit->setFinalDestination(unit->getPosition());
				unit->setWaypoint(unit->getPosition());
			} else {
				sp->warning("UnitScripting::process - Attempt to stopMovement for null unit.");
			}
			break;

		case GS_CMD_SETUNITVARIABLE:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					// variable number or name...
					if (stringData[0] >= '0' && stringData[0] <= '9')
						unit->variables.setVariable(str2int(stringData), *lastValue);
					else
						unit->variables.setVariable(stringData, *lastValue);
				} else {
					sp->warning("UnitScripting::process - setUnitVariable parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitVariable for null unit.");
			}
			break;

		case GS_CMD_GETUNITVARIABLE:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					// variable number or name...
					if (stringData[0] >= '0' && stringData[0] <= '9')
						*lastValue = unit->variables.getVariable(str2int(stringData));
					else
						*lastValue = unit->variables.getVariable(stringData);
				} else {
					sp->warning("UnitScripting::process - getUnitVariable parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitVariable for null unit.");
			}
			break;

		case GS_CMD_FINDCOVER:
			if (unit != NULL)
			{
				// WARNING: unsafe cast!
				//UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();

				// TODO: still very incomplete...

				VC3 upos = unit->getPosition();
				VC3 epos = upos;
				if (unit->targeting.hasTarget())
				{
					Unit *targu = unit->targeting.getTargetUnit();
					if (targu != NULL) 
						epos = targu->getPosition();
					else
						if (unit->getSeeUnit() != NULL)
							epos = unit->getSeeUnit()->getPosition();
				}
				int upathPosX = game->gameMap->scaledToPathfindX(upos.x);
				int upathPosY = game->gameMap->scaledToPathfindY(upos.z);
				int epathPosX = game->gameMap->scaledToPathfindX(epos.x);
				int epathPosY = game->gameMap->scaledToPathfindY(epos.z);
				int dist = game->gameMap->getCoverMap()->getDistanceToNearestCover(upathPosX, upathPosY);
				if (dist < 100) // not meters, pathfind blocks
				{
					//float distScaled = (float)dist * game->gameMap->getScaledSizeY() / (float)game->gameMap->getPathfindSizeY();

					int destx;
					int desty;
					CoverFinder::findCover(game->gameMap->getCoverMap(), 
						upathPosX, upathPosY, &destx, &desty, epathPosX, epathPosY);

					gsd->position.x = game->gameMap->pathfindToScaledX(destx);
					gsd->position.z = game->gameMap->pathfindToScaledY(desty);
					gsd->position.y = game->gameMap->getScaledHeightAt(gsd->position.x, gsd->position.z);

					*lastValue = 1; 			
				} else {
					*lastValue = 0; 
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to findCover for null unit.");
			}
			break;

		case GS_CMD_LASTTARGETPOSITION:
			if (unit != NULL)
			{
				if (unit->targeting.hasLastTargetPosition())
				{
					gsd->position = unit->targeting.getLastTargetPosition();
					*lastValue = 1;
				} else {
					gsd->position = unit->getPosition();
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to lastTargetPosition for null unit.");
			}
			break;

		case GS_CMD_SETUNIT:
			//if (unit != NULL)
			//{
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - setUnit parameter expected.");
					break;
				}
				Unit *u2 = NULL;
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					u2 = gsd->spottedUnit;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						u2 = gsd->alertUnit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					u2 = gsd->alertUnit;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					u2 = gsd->shooter;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					u2 = gsd->noisy;
				}
				if (strcmp(stringData, "target") == 0)
				{
					if (gsd->unit != NULL
						&& gsd->unit->targeting.hasTarget())
					{
						u2 = gsd->unit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "null") == 0)
				{
					gsd->unit = NULL;
					*lastValue = 1;
					break;
				}
				if (u2 != NULL)
				{
					*lastValue = 1;
					gsd->unit = u2;
				} else {
					*lastValue = 0;
					sp->warning("UnitScripting::process - setUnit parameter invalid or null unit.");
				}
			}
			//} else {
			//	sp->warning("UnitScripting::process - Attempt to setUnit for null unit.");
			//}
			break;

		case GS_CMD_ISUNITAVAILABLE:
			//if (unit != NULL)
			//{
			{
				if (stringData == NULL)
				{
					sp->warning("UnitScripting::process - isUnitAvailable parameter expected.");
					break;
				}
				Unit *u2 = NULL;
				if (strcmp(stringData, "spottedTarget") == 0)
				{
					u2 = gsd->spottedUnit;
				}
				if (strcmp(stringData, "alertTarget") == 0)
				{
					if (gsd->alertUnit != NULL
						&& gsd->alertUnit->targeting.hasTarget())
					{
						u2 = gsd->alertUnit->targeting.getTargetUnit();
					}
				}
				if (strcmp(stringData, "alerter") == 0)
				{
					u2 = gsd->alertUnit;
				}
				if (strcmp(stringData, "shooter") == 0)
				{
					u2 = gsd->shooter;
				}
				if (strcmp(stringData, "noisy") == 0)
				{
					u2 = gsd->noisy;
				}
				if (strcmp(stringData, "current") == 0)
				{
					u2 = gsd->unit;
				}
				if (u2 != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			}
			//} else {
			//	sp->warning("UnitScripting::process - Attempt to setUnit for null unit.");
			//}
			break;

		case GS_CMD_RESTOREUNIT:
			//if (gsd->originalUnit == NULL)
			//{
			//	sp->debug("UnitScripting::process - Restoring null unit.");
			//}
			gsd->unit = gsd->originalUnit;
			break;

		case GS_CMD_SETCLOSESTFRIENDLYUNIT:
			gsd->unit = findClosestFriendlyUnit(game, gsd->position, gsd->player, unit);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETCLOSESTHOSTILEUNIT:
			gsd->unit = findClosestHostileUnit(game, gsd->position, gsd->player, unit);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETCLOSESTOWNEDUNIT:
			gsd->unit = findClosestOwnedUnit(game, gsd->position, gsd->player, unit);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_UNITOWNERPLAYER:
			if (unit != NULL)
			{
				gsd->player = unit->getOwner();
			} else {
				sp->warning("UnitScripting::process - Attempt to unitOwnerPlayer for null unit.");
			}
			
			break;

		case GS_CMD_ISUNITCHARACTERNAME:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					Character *c = unit->getCharacter();
					if (c != NULL)
					{
						char *name = c->getName();
						if (name != NULL)
						{
							if (strcmp(stringData, name) == 0)
								*lastValue = 1;
							else
								*lastValue = 0;
						} else {
							*lastValue = 0;
						}
					} else {
						*lastValue = 0;
					}
				} else {
					sp->warning("UnitScripting::process - isUnitCharacterName parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitCharacterName for null unit.");
			}
			break;

		case GS_CMD_SETNEXTOWNEDUNIT:
			gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, unit);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETFIRSTOWNEDUNIT:
			gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, NULL);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETFIRSTOWNEDUNITOFTYPE:
			if (stringData == NULL)
			{
				sp->error("UnitScripting::process - setFirstOwnedUnitOfType parameter missing (unit type expected).");
			} else {
				UnitType *ut = getUnitTypeByName(stringData);
				if (ut == NULL)
				{
					sp->error("UnitScripting::process - setFirstOwnedUnitOfType, reference to unknown unit type.");
				} else {
					gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, NULL);
					while (gsd->unit != NULL)
					{
						if (gsd->unit->getUnitType() == ut)
							break;
						gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, gsd->unit);
					}
					if (gsd->unit != NULL)
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
				}
			}
			break;

		case GS_CMD_setOwnedUnitOfType:
		case GS_CMD_setOwnedUnitOfTypeIncludingInActiveUnits:
			if (stringData != NULL)
			{
				UnitType *ut = getUnitTypeByName(stringData);
				if (ut == NULL)
				{
					sp->error("UnitScripting::process - setOwnedUnitOfType / setOwnedUnitOfTypeIncludingInActiveUnits, reference to unknown unit type.");
					break;
				}
				
				bool only_active = true;
				if(command == GS_CMD_setOwnedUnitOfTypeIncludingInActiveUnits)
				{
					only_active = false;
				}

				int i = 0;
				gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, NULL, only_active);
				while( gsd->unit != NULL )
				{
					if(gsd->unit->getUnitType() == ut)
					{
						// return the n'th one
						i++;
						if(i == *lastValue) break;
					}
					gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, gsd->unit, only_active);
				}
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			}	else {
				sp->error("UnitScripting::process - setOwnedUnitOfType / setOwnedUnitOfTypeIncludingInActiveUnits parameter missing (unit type expected).");
			}
			break;

		case GS_CMD_SETRANDOMOWNEDUNIT:
			gsd->unit = randomOwnedUnit(game, gsd->position, gsd->player);
			if (gsd->unit != NULL)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETUNITBYCHARACTERNAME:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - setUnitByCharacterName parameter missing.");
			} else {
				gsd->unit = findUnitByCharacterName(game, stringData);
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
				break;
			}

		case GS_CMD_ADDCHARACTER:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					Character *ch = new Character(stringData);
					if (ch == NULL)
					{
						sp->error("UnitScripting::process - addCharacter, failed to create new character.");
					}
					gsd->unit->setCharacter(ch);
				} else {
					sp->error("UnitScripting::process - Attempt to addCharacter for null unit.");
				}
			} else {
				sp->error("UnitScripting::process - addCharacter parameter missing.");
			}
			break;

		case GS_CMD_ADDUNIT:
			if (stringData == NULL)
			{
				sp->error("UnitScripting::process - addUnit parameter missing (unit type expected).");
			} else {
				UnitType *ut = getUnitTypeByName(stringData);
				if (ut == NULL)
				{
					sp->error("UnitScripting::process - addUnit, reference to unknown unit type.");
				} else {
					Unit *reuse = NULL;

					// TODO: don't do this while initializing a mission, this should be absolutely useless waste of 
					// time at that point, as there should be no destroyed units to re-use.

					if (ut->doesAllowRespawn())
					{
						if (!ut->doesRemoveDestroyed())
						{
							sp->warning("UnitScripting::process - addUnit, unit type has \"allowrespawn\" on, but does not have \"removedestroyed\" on (useless combination).");
							sp->debug(stringData);
						}

						reuse = UnitSpawner::findReusableUnit(game, ut, gsd->player);

						if (reuse != NULL)
						{
							UnitSpawner::reuseUnit(game, reuse);
						} else {
							//sp->debug("UnitScripting::process - addUnit, allowrespawn is on, but did not find a suitable unit to re-use. creating a new one.");
						}
					}

					if (reuse != NULL)
					{
						gsd->unit = reuse;
					} else {
						gsd->unit = ut->getNewUnitInstance(gsd->player);
						if (gsd->unit == NULL)
						{
							sp->error("UnitScripting::process - addUnit, internal error while creating unit.");
						} else {
							game->units->addUnit(gsd->unit);
							gsd->unit->setActive(false);
						}
					}
				}
			}
			break;

		case GS_CMD_ADDUNITROOTPART:
			if (!PARTTYPE_ID_STRING_VALID(stringData))
			{
				if (stringData == NULL)
					sp->error("UnitScripting::process - addUnitRootPart, expected part type id.");
				else
					sp->error("UnitScripting::process - addUnitRootPart, illegal part type id.");
			} else {
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("UnitScripting::process - addUnitRootPart, reference to unloaded part type.");
				} else {
					if (unit == NULL)
					{
						sp->error("UnitScripting::process - addUnitRootPart, attempted to add root part to null unit.");
					} else {
						if (unit->getRootPart() != NULL)
						{
							sp->debug("UnitScripting::process - addUnitRootPart, detaching old root part and sub parts.");
							game->detachParts(unit, unit->getRootPart());
						}
						gsd->part = pt->getNewPartInstance();
						gsd->part->setOwner(gsd->player);
						unit->setRootPart(gsd->part);
						//partInUnit = true;
						if (unit->isActive())
						{
							game->deleteVisualOfParts(unit, unit->getRootPart());
							game->createVisualForParts(unit, unit->getRootPart());
						}
					}
				}
			}
			break;

		case GS_CMD_ADDSUBPART:
			if (!PARTTYPE_ID_STRING_VALID(stringData))
			{
				if (stringData == NULL)
					sp->error("UnitScripting::process - addSubPart, expected part type id.");
				else
					sp->error("UnitScripting::process - addSubPart, illegal part type id.");
			} else {
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("UnitScripting::process - addSubPart, reference to unloaded part type.");
				} else {
					// TODO: check that the part is actually attached to a unit!
					//if (!partInUnit || part == NULL)
					if (gsd->part == NULL)
					{
						sp->error("UnitScripting::process - addSubPart, attempted to add sub-part to null parent part.");
					} 
					else if(unit == NULL)
					{
						sp->error("UnitScripting::process - addSubPart, attempted to add sub-part to null unit.");
					}
					else 
					{
						Part *parentPart = gsd->part;
						int slotamount = parentPart->getType()->getSlotAmount();
						int slot;
						for (slot = 0; slot < slotamount; slot++)
						{
							if (parentPart->getSubPart(slot) == NULL 
								&& pt->isInherited(parentPart->getType()->getSlotType(slot)))
							break;
						}
						if (slot < slotamount)
						{
							gsd->part = pt->getNewPartInstance();
							gsd->part->setOwner(gsd->player);
							parentPart->setSubPart(slot, gsd->part);
							if (unit->isActive())
							{
								if (unit->isMuzzleflashVisible())
									unit->setMuzzleflashVisualEffect(NULL, 0);

								// fix: stop player from disappearing for one frame
								game->recreateVisualOfParts(unit, unit->getRootPart());
								//game->deleteVisualOfParts(unit, unit->getRootPart());
								//game->createVisualForParts(unit, unit->getRootPart());

								if (unit->getFlashlight() != NULL)
								{
									unit->getFlashlight()->resetOrigin(unit->getVisualObject());
								}
								/*
								if (unit->getSpotlight() != NULL)
								{
									if (unit->getVisualObject() != NULL)
									{
										unit->getSpotlight()->setOriginModel(unit->getVisualObject()->getStormModel());
									}
								}
								*/

								// renew the lost animation :D
								if (unit->getWalkDelay() < 2)
									unit->setWalkDelay(2);
								if (unit->getAnimationSet() != NULL)
								{
									unit->setAnimation(0); // ANIM_NONE
									if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
										unit->getAnimationSet()->animate(unit, ANIM_STAND);
								}

								// if this is a weapon, redo unit weaponry
								if (pt->isInherited(
									getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
								{ 
									// WARNING: unsafe cast!
									WeaponObject *wo = (WeaponObject *)gsd->part;
									int ammos = wo->getAmmoAmount();

									// TODO: should not SET ammo amount, should ADD ammo amount!

									wo->setAmmoAmount(0);
									unit->uninitWeapons();
									wo->setAmmoAmount(ammos);
									unit->initWeapons();

									// WARNING: unsafe cast!
									if (((Weapon *)wo->getType())->isThrowable())
									{
										int newweap = unit->getWeaponByWeaponType(pt->getPartTypeId());
										if (newweap != -1)
										{
											unit->setSelectedSecondaryWeapon(newweap);
										}
									}

									for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
									{
										// NOTE: oh ffs. the previously added weapons lose the clip ammo amount
										// when weapons are re-initialized - thus, must reload ALL weapon here!
										//if (unit->getWeaponType(w) == pt)
										//{
											unit->reloadWeaponAmmoClip(w, true);
										//}

										if (w == unit->getSelectedWeapon())
											unit->setWeaponActive(w, true);
										else
											unit->setWeaponActive(w, false);
									}
								}
							}

							// HACK!!!
							// WARNING!!!
							// FIXME!!!
							if (pt == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("DoorDirR")))
							{
								unit->setUnitMirrorSide(true);
							}

							// done by setSubPart
							//part->setParent(parentPart);
						} else {
							sp->error("UnitScripting::process - addSubPart, added sub-part does not fit parent part.");
						}
					}
				}
			}
			break;

		case GS_CMD_TOPARENTPART:
			if (gsd->part == NULL)
			{
				sp->error("UnitScripting::process - toParentPart, attempted to select parent for null part.");
			} else {
				// TODO: check that part attached to an unit
				//if (!partInUnit)
				//{
				//	error("Attempted to select parent for storage part.", lineNumber);
				//} else {
					gsd->part = gsd->part->getParent();
				//}
			}
			break;

		case GS_CMD_TOROOTPART:
			if (unit == NULL)
			{
				sp->error("UnitScripting::process - toRootPart, attempted to select root part for null unit.");
			} else {
				gsd->part = unit->getRootPart();
			}
			break;

		case GS_CMD_SETSELECTEDWEAPON:
			if (unit != NULL)
			{
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - setSelectedWeapon, expected part type id.");
					else
						sp->error("UnitScripting::process - setSelectedWeapon, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - setSelectedWeapon, reference to unloaded part type.");
					} else {
						assert(unit->isActive());
						int newweap = unit->getWeaponByWeaponType(pt->getPartTypeId());
						if (newweap != -1)
						{
							unit->setSelectedWeapon(newweap);
							if (unit->isActive())
							{
								for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
								{
									if (w == unit->getSelectedWeapon())
										unit->setWeaponActive(w, true);
									else
										unit->setWeaponActive(w, false);
								}
							}
						} else {
							sp->warning("UnitScripting:process - setSelectedWeapon, unit did not have such weapon");
						}
					}
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setSelectedWeapon for null unit.");
			}
			break;

		case GS_CMD_SETSELECTEDSECONDARYWEAPON:
			if (unit != NULL)
			{
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - setSelectedSecondaryWeapon, expected part type id.");
					else
						sp->error("UnitScripting::process - setSelectedSecondaryWeapon, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - setSelectedSecondaryWeapon, reference to unloaded part type.");
					} else {
						assert(unit->isActive());
						int newweap = unit->getWeaponByWeaponType(pt->getPartTypeId());
						if (newweap != -1)
						{
							unit->setSelectedSecondaryWeapon(newweap);
							if (unit->isActive())
							{
								for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
								{
									if (w == unit->getSelectedSecondaryWeapon())
										unit->setWeaponActive(w, true);
									else
										unit->setWeaponActive(w, false);
								}
							}
						} else {
							sp->warning("UnitScripting:process - setSelectedSecondaryWeapon, unit did not have such weapon");
						}
					}
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setSelectedSecondaryWeapon for null unit.");
			}
			break;

		case GS_CMD_SEEKSUBPART:
			if (!PARTTYPE_ID_STRING_VALID(stringData))
			{
				if (stringData == NULL)
					sp->error("UnitScripting::process - seekSubPart, expected part type id.");
				else
					sp->error("UnitScripting::process - seekSubPart, illegal part type id.");
			} else {
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("UnitScripting::process - seekSubPart, reference to unloaded part type.");
				} else {
					// TODO: check that the part is actually attached to a unit!
					//if (!partInUnit || part == NULL)
					if (gsd->part == NULL)
					{
						sp->error("UnitScripting::process - seekSubPart, attempted to seek sub-part to null parent part.");
					} else {
						Part *parentPart = gsd->part;
						int slotamount = parentPart->getType()->getSlotAmount();
						int slot;
						for (slot = 0; slot < slotamount; slot++)
						{
							if (parentPart->getSubPart(slot) != NULL 
								&& (parentPart->getSubPart(slot)->getType() == pt
								|| parentPart->getSubPart(slot)->getType()->isInherited(pt)))
							{
								break;
							}
						}
						if (slot < slotamount)
						{
							gsd->part = parentPart->getSubPart(slot);
							*lastValue = 1;
						} else {
							gsd->part = NULL;
							*lastValue = 0;
						}
					}
				}
			}
			break;

		case GS_CMD_ISPARTAVAILABLE:
			if (gsd->part != NULL)
			{
				*lastValue = 1; 
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETPARTTYPEDATA:
			if (gsd->part != NULL)
			{
				if (stringData != NULL)
				{
					char *tmpbuf = new char[strlen(stringData) + 1];
					strcpy(tmpbuf, stringData);
					char *sep = strstr(tmpbuf, "=");
					if (sep != NULL)
					{
						*sep = '\0';
						bool success = gsd->part->getType()->setData(tmpbuf, &sep[1]);
						if (!success)
						{
							sp->error("UnitScripting::process - setPartTypeData failed.");
						}
					} else {
						sp->error("UnitScripting::process - setPartTypeData parameter bad (part type data key=value pair expected).");
					}
					delete [] tmpbuf;
				} else {
					sp->error("UnitScripting::process - setPartTypeData parameter missing.");
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setPartTypeData for null part.");
			}
			break;

		case GS_CMD_SETUNITANGLE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				rot.y = (float)intData;
				if (rot.y < 0) rot.y += 360;
				if (rot.y >= 360) rot.y -= 360;
				unit->setRotation(rot.x, rot.y, rot.z);
			} else {
				sp->error("UnitScripting::process - Attempt to setUnitAngle for null unit.");
			}
			break;

		case GS_CMD_GETUNITANGLE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				*lastValue = (int)rot.y;
			} else {
				sp->error("UnitScripting::process - Attempt to getUnitAngle for null unit.");
			}
			break;

		case GS_CMD_SPAWNUNIT:
			if (unit != NULL)
			{
				if (game->inCombat)
				{
					if (unit->isActive())
					{
						sp->error("UnitScripting::process - spawnUnit for unit that is already active.");
					} else {
						UnitSpawner::spawnUnit(game, unit);
					}
				} else {
					sp->error("UnitScripting::process - spawnUnit not allowed in menu mode.");
				}
			} else {
				sp->error("UnitScripting::process - Attempt to spawnUnit for null unit.");
			}
			break;

		case GS_CMD_SETUNITSCRIPT:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (stringData[0] == '\0'
						|| strcmp(stringData, "null") == 0)
						unit->setScript(NULL);
					else
						unit->setScript(stringData);
				} else {
					sp->error("UnitScripting::process - Attempt to setUnitScript for null unit.");
				}
			} else {
				unit->setScript(NULL);
				//sp->error("UnitScripting::process - setUnitScript parameter missing.");
			}
			break;

		case GS_CMD_SETUNITIDSTRING:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (stringData[0] == '\0'
						|| strcmp(stringData, "null") == 0)
					{
						unit->setIdString(NULL);
					} else {
						const char *s = stringData;
						if (stringData[0] == '$'
							&& stringData[1] == '\0')
						{
							s = gsd->stringValue;
						}
						if (s != NULL)
						{
							Unit *prevu = game->units->getUnitByIdString(s);
							if (prevu != NULL)
							{
								sp->warning("UnitScripting::process - setUnitIdString, Another unit with given id-string already exists.");
							}
							unit->setIdString(s);
						} else {
							sp->error("UnitScripting::process - setUnitIdString, null string value.");
						}
					}
				} else {
					sp->error("UnitScripting::process - Attempt to setUnitIdString for null unit.");
				}
			} else {
				if (gsd->unit != NULL)
				{
					unit->setIdString(NULL);
				} else {
					sp->error("UnitScripting::process - Attempt to setUnitIdString for null unit.");
				}
				sp->warning("UnitScripting::process - setUnitIdString parameter missing.");
			}
			break;

		case GS_CMD_SETUNITSPAWNCOORDINATES:
			if (unit != NULL)
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					VC3 spawn = VC3(
						x, game->gameMap->getScaledHeightAt(x,y), y);
					unit->setSpawnCoordinates(spawn);
				} else {
					sp->error("UnitScripting::process - Missing or bad setUnitSpawnCoordinates parameter.");
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setUnitSpawnCoordinates for null unit.");
			}
			break;

		case GS_CMD_SETUNITSPAWNCOORDINATESTOPOSITION:
			if (unit != NULL)
			{
				unit->setSpawnCoordinates(gsd->position);
			} else {
				sp->error("UnitScripting::process - Attempt to setUnitSpawnCoordinatesToPosition for null unit.");
			}
			break;

		case GS_CMD_ISSPEED:
			if (stringData != NULL)
			{
				*lastValue = 0;
				bool paramOk = false;
				if (strcmp(stringData, "slow") == 0)
				{
					paramOk = true;
					if (unit->getSpeed() == Unit::UNIT_SPEED_SLOW)
						*lastValue = 1;
				}
				else if (strcmp(stringData, "fast") == 0)
				{
					paramOk = true;
					if (unit->getSpeed() == Unit::UNIT_SPEED_FAST)
						*lastValue = 1;
				}
				else if (strcmp(stringData, "crawl") == 0)
				{
					paramOk = true;
					if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
						*lastValue = 1;
				}
				else if (strcmp(stringData, "sprint") == 0)
				{
					paramOk = true;
					if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
						*lastValue = 1;
				}
				else if (strcmp(stringData, "jump") == 0)
				{
					paramOk = true;
					if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
						*lastValue = 1;
				}
				if (!paramOk)
				{
					sp->error("UnitScripting::process - isSpeed parameter bad.");
				}
			} else {
				sp->error("UnitScripting::process - isSpeed parameter missing.");
			}
			break;

		case GS_CMD_ISUNITINSIDEBUILDING:
			if (unit != NULL)
			{
				VC3 pos = unit->getPosition();
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					if (game->gameMap->getAreaMap()->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitInsideBuilding for null unit.");
			}
			break;

		case GS_CMD_ISUNITTARGETINSIDEBUILDING:
			if (unit != NULL)
			{
				if (unit->targeting.hasTarget() 
					&& unit->targeting.getTargetUnit() != NULL)
				{
					VC3 pos = unit->targeting.getTargetUnit()->getPosition();
					int x = game->gameMap->scaledToPathfindX(pos.x);
					int y = game->gameMap->scaledToPathfindY(pos.z);
					if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
					{
						if (game->gameMap->getAreaMap()->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
							*lastValue = 1;
						else
							*lastValue = 0;
					} else {
						*lastValue = 0;
					}
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitTargetInsideBuilding for null unit.");
			}
			break;

		case GS_CMD_ISUNITHIDDEN:
			*lastValue = 0;
			/*
			if (unit != NULL)
			{
				VC3 pos = unit->getPosition();
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				// hiddeness 50% or more
				if (game->gameMap->getHideMap()->getHiddenessAt(x, y) >= HideMap::maxHiddeness / 2)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitHidden for null unit.");
			}
			*/
			break;

		case GS_CMD_ISUNITVERYHIDDEN:
			*lastValue = 0;
			/*
			if (unit != NULL)
			{
				VC3 pos = unit->getPosition();
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				// hiddeness 75% or more
				if (game->gameMap->getHideMap()->getHiddenessAt(x, y) >= 3 * HideMap::maxHiddeness / 4)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitVeryHidden for null unit.");
			}
			*/
			break;

		case GS_CMD_ISUNITCOVERED:
			if (unit != NULL)
			{
				VC3 pos = unit->getPosition();
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				if (unit->targeting.hasTarget()
					&& unit->targeting.getTargetUnit() != NULL)
				{
					// see if covered from target direction
					VC3 targPos = unit->targeting.getTargetUnit()->getPosition();
					int targX = game->gameMap->scaledToPathfindX(targPos.x);
					int targY = game->gameMap->scaledToPathfindY(targPos.z);
					if (CoverFinder::isCoveredFrom(
						game->gameMap->getCoverMap(), x, y, targX, targY))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					// just see if covered from any direction
					if (game->gameMap->getCoverMap()->isCoveredFromAny(
						CoverMap::COVER_FROM_ALL_MASK, x, y))
						*lastValue = 1;
					else
						*lastValue = 0;
				}
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitCovered for null unit.");
			}
			break;

		case GS_CMD_ISUNITINVEGETATION:
			*lastValue = 0;
			/*
			if (unit != NULL)
			{
				VC3 pos = unit->getPosition();
				game->gameMap->keepWellInScaledBoundaries(&pos.x, &pos.z);
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				// hiddeness 50% or more and vegetation type
				if (game->gameMap->getHideMap()->getHiddenessAt(x, y) >= HideMap::maxHiddeness / 2
					&& game->gameMap->getHideMap()->getHiddenessTypeAt(x, y) == HideMap::HIDDENESS_TYPE_VEGETATION)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->error("UnitScripting::process - Attempt to isUnitInVegetation for null unit.");
			}
			*/
			break;

		case GS_CMD_UNITSPAWNPOSITION:
			if (unit != NULL)
			{
				VC3 spawnCoords = unit->getSpawnCoordinates();
				gsd->position = spawnCoords;
				//VC3 unitPos = VC3(game->gameMap->configToScaledX((int)spawnCoords.x), 0, 
				//	game->gameMap->configToScaledY((int)spawnCoords.z));
				//gsd->position = unitPos;
			} else {
				sp->warning("UnitScripting::process - Attempt to unitSpawnPosition for null unit.");
			}
			break;

		case GS_CMD_GIVEUNITSIGHTBONUS:
			if (unit != NULL)
			{
				unit->setSightBonus(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to giveUnitSightBonus for null unit.");
			}
			break;

		case GS_CMD_ISTARGETUNCONSCIOUS:
			if (unit != NULL)
			{
				*lastValue = 0;
				if (unit->targeting.hasTarget()
					&& unit->targeting.getTargetUnit() != NULL
					&& unit->targeting.getTargetUnit()->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					*lastValue = 1;
			} else {
				sp->warning("UnitScripting::process - Attempt to isTargetUnconscious for null unit.");
			}
			break;

		case GS_CMD_ISUNITTYPE:
			if (unit != NULL)
			{
				if (strcmp(unit->getUnitType()->getName(), stringData) == 0)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitType for null unit.");
			}
			break;

		case GS_CMD_POSITIONDISTANCETONEARESTCOVER:
			if (unit != NULL)
			{
				VC3 upos = unit->getPosition();
				VC3 epos = upos;
				if (unit->targeting.hasTarget())
				{
					Unit *targu = unit->targeting.getTargetUnit();
					if (targu != NULL) 
						epos = targu->getPosition();
					else
						if (unit->getSeeUnit() != NULL)
							epos = unit->getSeeUnit()->getPosition();
				}
				int upathPosX = game->gameMap->scaledToPathfindX(upos.x);
				int upathPosY = game->gameMap->scaledToPathfindY(upos.z);
				int dist = game->gameMap->getCoverMap()->getDistanceToNearestCover(upathPosX, upathPosY);
				if (dist > 120) // not meters, pathfind blocks
					dist = 120;
				*lastValue = (int)((float)dist * (game->gameMap->getScaledSizeX() / (float)game->gameMap->getPathfindSizeX()));
			} else {
				sp->warning("UnitScripting::process - Attempt to positionDistanceToNearestCover for null unit.");
			}
			break;

		case GS_CMD_ISUNITUNCONSCIOUS:
			if (unit != NULL)
			{
				if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitUnconscious for null unit.");
			}
			break;

		case GS_CMD_WARPUNITTOPOSITION:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);

				VC3 groundPos = gsd->position;

				if (!ua->warpToPosition(unit, groundPos))
				{
					sp->warning("UnitScripting::process - warpUnitToPosition did not find suitable place for unit.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to warpUnitToPosition for null unit.");
			} 					
			break;

		case GS_CMD_CANUNITWARPTOPOSITION:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);

				VC3 groundPos = gsd->position;

				if (ua->canWarpToPosition(unit, groundPos))
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to canUnitWarpToPosition for null unit.");
			} 					
			break;

		case GS_CMD_WARPUNITTOUNIT:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					UnitActor *ua = getUnitActorForUnit(unit);

					Unit *otherUnit = game->units->getUnitByIdString(stringData);
					if (otherUnit != NULL)
					{
						VC3 groundPos = otherUnit->getPosition();

						if (!ua->warpToPosition(unit, groundPos))
						{
							sp->warning("UnitScripting::process - warpUnitToUnit did not find suitable place for unit.");
						}
					} else {
						sp->warning("UnitScripting::process - warpUnitToUnit, no unit found with given id string.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to warpUnitToUnit for null unit.");
				} 					
			} else {
				sp->error("UnitScripting::process - warpUnitToUnit parameter missing, another unit id string expected.");
			}
			break;

		case GS_CMD_setUnitRotationToFaceUnit:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					UnitActor *ua = getUnitActorForUnit(unit);

					Unit *otherUnit = game->units->getUnitByIdString(stringData);
					if (otherUnit != NULL)
					{
						VC3 unitPos = unit->getPosition();
						VC3 otherPos = otherUnit->getPosition();

						float angle = util::PositionDirectionCalculator::calculateDirection(otherPos, unitPos);
						// HACK: some magic treshold number here. (to avoid always changing the blocked area)
						if (fabs(unit->getRotation().y - angle) > 0.5f)
						{
							bool redoObst = unit->obstacleExists;
							// need to update blocked area in case of line block.
							if (redoObst && unit->getUnitType()->isLineBlock())
							{
								ua->removeUnitObstacle(unit);
							}

							unit->setRotation(0, angle, 0);

							// need to update blocked area in case of line block.
							if (redoObst && unit->getUnitType()->isLineBlock())
							{
								ua->addUnitObstacle(unit);
							}
						}

					} else {
						sp->warning("UnitScripting::process - setUnitRotationToFace, no unit found with given id string.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to setUnitRotationToFace for null unit.");
				} 					
			} else {
				sp->error("UnitScripting::process - setUnitRotationToFace parameter missing, another unit id string expected.");
			}
			break;

		case GS_CMD_WARPUNITTOHEIGHT:
			if (unit != NULL)
			{
				VC3 groundPos = unit->getPosition();
				if (game->gameMap->isWellInScaledBoundaries(groundPos.x, groundPos.z))
					groundPos.y = game->gameMap->getScaledHeightAt(groundPos.x, groundPos.z) + (float)intData;
				unit->setPosition(groundPos);
				if (intData == 0)
					unit->setOnGround(true);
				else
					unit->setOnGround(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to warpUnitToHeight for null unit.");
			} 					
			break;

		case GS_CMD_WARPUNITTOFLOATHEIGHT:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				VC3 groundPos = unit->getPosition();
				if (game->gameMap->isWellInScaledBoundaries(groundPos.x, groundPos.z))
					groundPos.y = game->gameMap->getScaledHeightAt(groundPos.x, groundPos.z) + floatData;
				unit->setPosition(groundPos);
				if (floatData == 0.0f)
					unit->setOnGround(true);
				else
					unit->setOnGround(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to warpUnitToHeight for null unit.");
			} 					
			break;

		case GS_CMD_SETUNITROTATIONYTOVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				float yAngle = (float)(*lastValue % 360);
				unit->setRotation(rot.x, yAngle, rot.z);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitRotationYToValue for null unit.");
			} 					
			break;

		case GS_CMD_SETUNITROTATIONXTOVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				unit->setRotation((float)*lastValue, rot.y, rot.z);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitRotationXToValue for null unit.");
			} 					
			break;

		case GS_CMD_SETUNITROTATIONZTOVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				unit->setRotation(rot.x, rot.y, (float)*lastValue);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitRotationZToValue for null unit.");
			} 					
			break;

		case GS_CMD_SLIDEUNITROTATIONYTOWARDVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				float rotSpeed = util::AngleRotationCalculator::getRotationForAngles(rot.y, (float)*lastValue, 1.0f);
				rot.y += rotSpeed * 0.1f;
				unit->setRotation(rot.x, rot.y, rot.z);
			} else {
				sp->warning("UnitScripting::process - Attempt to slideUnitRotationYTowardValue for null unit.");
			} 					
			break;

		case GS_CMD_SLIDEUNITROTATIONXTOWARDVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				if (rot.x < 0) rot.x += 360;
				float rotSpeed = util::AngleRotationCalculator::getRotationForAngles(rot.x, (float)*lastValue, 1.0f);
				if (rot.x >= 180) rot.x -= 360;
				rot.x += rotSpeed * 0.05f;
				unit->setRotation(rot.x, rot.y, rot.z);
			} else {
				sp->warning("UnitScripting::process - Attempt to slideUnitRotationXTowardValue for null unit.");
			} 					
			break;

		case GS_CMD_SLIDEUNITROTATIONZTOWARDVALUE:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();
				if (rot.z < 0) rot.z += 360;
				float rotSpeed = util::AngleRotationCalculator::getRotationForAngles(rot.z, (float)*lastValue, 1.0f);
				if (rot.z >= 180) rot.z -= 360;
				rot.z += rotSpeed * 0.05f;
				unit->setRotation(rot.x, rot.y, rot.z);
			} else {
				sp->warning("UnitScripting::process - Attempt to slideUnitRotationZTowardValue for null unit.");
			} 					
			break;

		case GS_CMD_STOPUNITVELOCITY:
			if (unit != NULL)
			{
				unit->setVelocity(VC3(0,0,0));
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitVelocity for null unit.");
			} 					
			break;

		case GS_CMD_MAKEUNITVISIBLE:
			if (unit != NULL)
			{
				unit->visibility.setToBeSeenByPlayer(0, true);
				unit->visibility.setSeenByFirstPerson(true);
				if (unit->getVisualObject() != NULL)
					unit->getVisualObject()->setVisible(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitVisible for null unit.");
			} 								
			break;

		case GS_CMD_ADDHEALTH:
			if (unit != NULL)
			{
				if (intData != 0)
				{
					unit->setHP(unit->getHP() + intData);
					game->gameUI->setUnitDamagedFlag(unit->getOwner());
				} else {
					sp->warning("UnitScripting::process - addHealth with zero value.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to addHealth for null unit.");
			} 					
			break;

		case GS_CMD_ADDARMOR:
			if (unit != NULL)
			{
				if (intData != 0)
				{
					unit->setArmorAmount(unit->getArmorAmount() + intData);
				} else {
					sp->warning("UnitScripting::process - addArmor with zero value.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to addArmor for null unit.");
			} 					
			break;

		case GS_CMD_GETARMOR:
			if (unit != NULL)
			{
				*lastValue = unit->getArmorAmount();
			} else {
				sp->warning("UnitScripting::process - Attempt to getArmor for null unit.");
			} 					
			break;

		case GS_CMD_SETARMORCLASS:
			if (unit != NULL)
			{
				unit->setArmorClass(intData);
			} else {
				sp->warning("UnitScripting::process - Attempt to setArmorClass for null unit.");
			} 					
			break;

		case GS_CMD_GETARMORCLASS:
			if (unit != NULL)
			{
				*lastValue = unit->getArmorClass();
			} else {
				sp->warning("UnitScripting::process - Attempt to getArmorClass for null unit.");
			} 					
			break;

		case GS_CMD_SETARMORATLEASTTO:
			if (unit != NULL)
			{
				*lastValue = 0;
				if (intData != 0)
				{
					if (unit->getArmorAmount() < intData)
					{
						unit->setArmorAmount(intData);
						*lastValue = 1;
					}
				} else {
					sp->warning("UnitScripting::process - setArmorAtLeastTo with zero value.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setArmorAtLeastTo for null unit.");
			} 					
			break;

		case GS_CMD_SETARMORCLASSATLEASTTO:
			if (unit != NULL)
			{
				*lastValue = 0;
				if (intData != 0)
				{
					if (unit->getArmorClass() < intData)
					{
						unit->setArmorClass(intData);
						*lastValue = 1;
					}
				} else {
					sp->warning("UnitScripting::process - setArmorClassAtLeastTo with zero value.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setArmorClassAtLeastTo for null unit.");
			} 					
			break;

		case GS_CMD_SETHEALTH:
			if (unit != NULL)
			{
				//assert(unit->getOwner() != NO_UNIT_OWNER);
				unit->setHP(intData);
				game->gameUI->setUnitDamagedFlag(unit->getOwner());
				// make unit unconscious
				if(intData < 0)
				{
					unit->setMoveState(Unit::UNIT_MOVE_STATE_UNCONSCIOUS);
					unit->setMoveStateCounter(0);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setHealth for null unit.");
			} 								
			break;

		case GS_CMD_ADDAMMOTOPART:
			if (unit != NULL)
			{
				if (gsd->part != NULL)
				{
					if (intData != 0)
					{
						// TODO: should allow loading of "Weap" type parts...?
						PartType *ammopt = getPartTypeById(
							PARTTYPE_ID_STRING_TO_INT("Ammo"));
						
						if (gsd->part->getType()->isInherited(ammopt))
						{ 						
							// WARNING: unsafe cast (checked by the above if clause)
							AmmoPack *ap = (AmmoPack *)gsd->part->getType();
							if (unit->addWeaponAmmo(ap, intData))
							{
								*lastValue = 1;
							} else {
								*lastValue = 0;
							}
							/*
							// WARNING: unsafe cast (checked by the above if clause)
							AmmoPackObject *ap = (AmmoPackObject *)gsd->part;
							// FIXME!!! won't work, inited ammo don't get transferred
							// out of the part correctly..
							unit->uninitWeapons();
							int newamount = ap->getAmount() + intData;
							if (newamount < 0) newamount = 0;
							if (newamount > ap->getMaxAmount()) newamount = ap->getMaxAmount();
							ap->setAmount(newamount);
							unit->initWeapons();
							game->gameUI->setUnitsChangedFlag(unit->getOwner());
							*/
						} else {
							sp->warning("UnitScripting::process - Attempt to addAmmoToPart to non-ammo part.");
						}
					} else {
						sp->warning("UnitScripting::process - addAmmoToPart with zero value.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to addAmmoToPart for null part.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to addAmmoToPart for null unit.");
			} 								
			break;

		case GS_CMD_ISPARTTYPE:
			if (!PARTTYPE_ID_STRING_VALID(stringData))
			{
				if (stringData == NULL)
					sp->error("UnitScripting::process - isPartType, expected part type id.");
				else
					sp->error("UnitScripting::process - isPartType, illegal part type id.");
			} else {
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("UnitScripting::process - isPartType, reference to unloaded part type.");
				} else {
					if (gsd->part == NULL)
					{
						sp->error("UnitScripting::process - isPartType, attempted to check type for null part.");
					} else {
						if (gsd->part->getType() == pt)
							*lastValue = 1;
						else
							*lastValue = 0;
					}
				}
			}
			break;

		case GS_CMD_MOVEUNITLEFT:
			if (unit != NULL)
			{
				moveUnitToDirection(unit, MOVE_DIR_LEFT, (float)intData / 100.0f);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitLeft for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITRIGHT:
			if (unit != NULL)
			{
				moveUnitToDirection(unit, MOVE_DIR_RIGHT, (float)intData / 100.0f);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitRight for null unit.");
			}
			break;

		case GS_CMD_FORCEUNITANIMATION:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					int num = unit->getAnimationSet()->getAnimNumberByName(stringData);
					if (num != -1)
					{
						unit->setForcedAnimation(num);
					} else {
						sp->warning("GameScripting::process - forceUnitAnimation anim by given name not found.");
					}
				} else {
					sp->warning("GameScripting::process - forceUnitAnimation parameter missing.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to forceUnitAnimation for null unit.");
			}
			break;
								
		case GS_CMD_ENDFORCEDUNITANIMATION:
			if (unit != NULL)
			{
				unit->setForcedAnimation(0); // ANIM_NONE
			} else {
				sp->warning("GameScripting::process - Attempt to endForcedUnitAnimation for null unit.");
			}
			break;

		case GS_CMD_DISABLEUNITAI:
			if (unit != NULL)
			{
				// WARNING: unsafe cast!
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				ai->setEnabled(false);
			} else { 
				sp->warning("GameScripting::process - Attempt to disableUnitAI for null unit.");
			}
			break;
						
		case GS_CMD_ENABLEUNITAI:
			if (unit != NULL)
			{
				// WARNING: unsafe cast!
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				ai->setEnabled(true);
			} else {
				sp->warning("GameScripting::process - Attempt to enableUnitAI for null unit.");
			}
			break;					 
		
		case GS_CMD_CLEARTARGET: 
			if (unit != NULL)
			{
				unit->targeting.clearTarget();
			} else {
				sp->warning("GameScripting::process - Attempt to clearTarget for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITFORWARD:
			if (unit != NULL)
			{
				moveUnitToDirection(unit, MOVE_DIR_FORWARD, (float)intData / 100.0f);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitForward for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITBACKWARD:
			if (unit != NULL)
			{
				moveUnitToDirection(unit, MOVE_DIR_BACKWARD, (float)intData / 100.0f);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitBackward for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITLEFTFLOAT:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				moveUnitToDirection(unit, MOVE_DIR_LEFT, floatData);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitLeft for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITRIGHTFLOAT:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				moveUnitToDirection(unit, MOVE_DIR_RIGHT, floatData);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitRight for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITFORWARDFLOAT:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				moveUnitToDirection(unit, MOVE_DIR_FORWARD, floatData);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitForward for null unit.");
			}
			break;

		case GS_CMD_MOVEUNITBACKWARDFLOAT:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				moveUnitToDirection(unit, MOVE_DIR_BACKWARD, floatData);
			} else {
				sp->warning("GameScripting::process - Attempt to moveUnitBackward for null unit.");
			}
			break;

		case GS_CMD_GIVEUNITITEM:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					UnitInventory::giveUnitItem(game, unit, stringData);
				} else {
					sp->error("GameScripting::process - giveUnitItem parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to giveUnitItem for null unit.");
			}
			break;

		case GS_CMD_GETUNITITEMCOUNT:
			*lastValue = 0;
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					*lastValue = UnitInventory::getUnitItemCount(game, unit, stringData);
				} else {
					sp->error("GameScripting::process - getUnitItemCount parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to getUnitItemCount for null unit.");
			}
			break;

		case GS_CMD_REMOVEUNITITEM:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					UnitInventory::removeUnitItem(game, unit, stringData);
				} else {
					sp->error("GameScripting::process - removeUnitItem parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to removeUnitItem for null unit.");
			}
			break;

		case GS_CMD_SETUNITITEMCOUNTTOVALUE:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					int value = *lastValue;
					if (value < 0)
					{
						value = 0;
						sp->warning("GameScripting::process - setUnitItemCountToValue value out of range.");
					}
					UnitInventory::setUnitItemCount(game, unit, stringData, value);
				} else {
					sp->error("GameScripting::process - setUnitItemCountToValue parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to setUnitItemCountToValue for null unit.");
			}
			break;

		case GS_CMD_SETSELECTEDUNITITEM:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					UnitInventory::selectUnitItem(game, unit, stringData);
				} else {
					sp->error("GameScripting::process - setSelectedUnitItem parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to setSelectedUnitItem for null unit.");
			}
			break;

		case GS_CMD_CLEARSELECTEDUNITITEM:
			if (unit != NULL)
			{
				UnitInventory::deselectUnitItem(game, unit);
			} else {
				sp->warning("GameScripting::process - Attempt to setSelectedUnitItem for null unit.");
			}
			break;

		case GS_CMD_ISSELECTEDUNITITEM:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					if (UnitInventory::isSelectedUnitItem(game, unit, stringData))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->error("GameScripting::process - isSelectedUnitItem parameter missing, item type name expected.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to isSelectedUnitItem for null unit.");
			}
			break;

		case GS_CMD_DESTROYUNIT:
			if (unit != NULL)
			{
				unit->setHP(unit->getUnitType()->getMinHP());
				unit->setDestroyed(true);
				int anim = ANIM_NONE;
				if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				{
					anim = ANIM_DIE_PRONE;
				} else {
					if ((game->gameRandom->nextInt() & 1) == 0)
						anim = ANIM_DIE_BACK;
					else
						anim = ANIM_DIE_FRONT;
				}
				if (unit->getAnimationSet() != NULL)
				{
					if (unit->getAnimationSet()->isAnimationInSet(anim))
					{
						// FIXME: does not set unit->setFallenOnBack if died back.
						unit->getAnimationSet()->animate(unit, anim);
					} else {
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_DIE))
						{
							unit->getAnimationSet()->animate(unit, ANIM_DIE);
						} else {
							unit->getAnimationSet()->animate(unit, ANIM_NONE);
						}
					}
				}
			} else {
				sp->warning("GameScripting::process - Attempt to destroyUnit for null unit.");
			}
			break;

		case GS_CMD_FINDCLOSESTUNITOFTYPE:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - findClosestUnitOfType parameter missing.");
			} else {
				gsd->unit = findClosestUnitOfType(game, gsd->position, stringData);
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_GETUNITID:
			if (unit != NULL)
			{
				*lastValue = game->units->getIdForUnit(gsd->unit);
			} else {
				sp->warning("GameScripting::process - Attempt to getUnitId for null unit.");
			}
			break;

		case GS_CMD_SETUNITBYID:
			if (*lastValue >= UNITID_LOWEST_POSSIBLE_VALUE
				&& *lastValue <= UNITID_HIGHEST_POSSIBLE_VALUE)
			{
				gsd->unit = game->units->getUnitById(*lastValue);
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				*lastValue = 0;
				sp->warning("GameScripting::process - setUnitById, value is not a valid id number.");
			}
			break;

		case GS_CMD_GETUNITIDSTRING:
			if (unit != NULL)
			{
				gsd->setStringValue(unit->getIdString());
			} else {
				sp->warning("GameScripting::process - Attempt to getUnitIdString for null unit.");
			}
			break;

		case GS_CMD_SETUNITBYIDSTRING:
			if (stringData != NULL)
			{
				// TODO: use the actual id string!!!
				if (str2int(stringData) > 0)
				{
					// seek by id number if this is a number...
					// TODO: maybe should not do this!
					gsd->unit = game->units->getUnitById(str2int(stringData));
					Logger::getInstance()->warning("GameScripting::process - setUnitByIdString, integer parameter given, treating it as id number (deprecated).");
				} else {
					const char *s = stringData;
					if (stringData[0] == '$'
						&& stringData[1] == '\0')
					{
						s = gsd->stringValue;
					}
					if (s != NULL)
					{
						gsd->unit = game->units->getUnitByIdString(s);
					} else {
						Logger::getInstance()->warning("GameScripting::process - setUnitByIdString, null string value.");
					}
				}

				if (gsd->unit != NULL)
				{
					if (!gsd->unit->isActive())
						gsd->unit = NULL;
				}

				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
					sp->warning("GameScripting::process - setUnitByIdString, No unit found with given id.");
				}
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - setUnitByIdString, parameter missing (unit id string expected).");
			}
			break;

		case GS_CMD_doesUnitExistWithIdString:
			if (stringData != NULL)
			{
				Unit *tmp = NULL;

				// TODO: use the actual id string!!!
				if (str2int(stringData) > 0)
				{
					// seek by id number if this is a number...
					// TODO: maybe should not do this!
					tmp = game->units->getUnitById(str2int(stringData));
				} else {
					tmp = game->units->getUnitByIdString(stringData);
				}

				if (tmp != NULL)
				{
					if (!tmp->isActive())
						tmp = NULL;
				}

				if (tmp != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
					//sp->warning("GameScripting::process - doesUnitExistWithIdString, No unit found with given id.");
				}
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - doesUnitExistWithIdString, parameter missing (unit id string expected).");
			}
			break;

		case GS_CMD_SETTARGETBYIDSTRING:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					Unit *tmp = NULL;
					// TODO: use the actual id string!!!
					if (str2int(stringData) > 0)
					{
						// seek by id number if this is a number...
						// TODO: maybe should not do this!
						tmp = game->units->getUnitById(str2int(stringData));
					} else {
						tmp = game->units->getUnitByIdString(stringData);
					}

					if (tmp != NULL)
					{
						if (gsd->unit != tmp)
						{
							gsd->unit->targeting.setTarget(tmp);
							*lastValue = 1;
						} else {
							*lastValue = 0;
							sp->error("GameScripting::process - setTargetByIdString, Unit with given id is the same as current unit (cannot target self).");
						} 
					} else {
						*lastValue = 0;
						sp->warning("GameScripting::process - setTargetByIdString, No unit found with given id.");
					}
				} else {
					sp->error("GameScripting::process - Attempt to setTargetByIdString for null unit.");
				}
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - setTargetByIdString, parameter missing (unit id string expected).");
			}
			break;

		case GS_CMD_SETUNITBYIDSTRINGVALUE:
			if (gsd->stringValue != NULL)
			{
				// TODO: use the actual id string!!!
				if (str2int(gsd->stringValue) > 0)
				{
					// seek by id number if this is a number...
					// TODO: maybe should not do this!
					gsd->unit = game->units->getUnitById(str2int(gsd->stringValue));
				} else {
					gsd->unit = game->units->getUnitByIdString(gsd->stringValue);
				}

				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
					sp->warning("GameScripting::process - setUnitByIdStringValue, No unit found with given id.");
					Logger::getInstance()->debug(gsd->stringValue);
				}
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - setUnitByIdStringValue, null string value encountered.");
			}
			break;

		case GS_CMD_SETTARGETBYIDVALUE:
			if (unit != NULL)
			{
				if (*lastValue > 1)
				{
					Unit *tmp = game->units->getUnitById(*lastValue);
					if (gsd->unit != tmp)
					{
						gsd->unit->targeting.setTarget(tmp);
						*lastValue = 1;
					} else {
						*lastValue = 0;
						sp->error("GameScripting::process - setTargetByIdValue, Unit with given id is the same as current unit (cannot target self).");
					} 
				} else {
					sp->error("GameScripting::process - Attempt to setTargetByIdValue for non-unit-id value.");
					*lastValue = 0;
				}
			} else {
				sp->warning("GameScripting::process - Attempt to setTargetByIdValue for null unit.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_MAKEUNITEXECUTE:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);
				ua->doExecute(unit);
			} else {
				sp->warning("GameScripting::process - Attempt to makeUnitExecute for null unit.");
			}
			break;

		case GS_CMD_GETPATHPOSITION:
			if (unit != NULL)
			{
				gsd->position = unit->scriptPaths.getStoredPathEndPosition(unit->scriptPaths.getStoredPathNumber());
			} else {
				sp->warning("UnitScripting::process - Attempt to getPathPosition for null unit.");
			}
			break;

		case GS_CMD_WASLASTPATHFINDSUCCESS:
			if (unit != NULL)
			{
				if (unit->wasLastPathfindSuccess())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to wasLastPathfindSuccess for null unit.");
			}
			break;

		case GS_CMD_SETFLASHLIGHTENERGY:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->setFlashlightEnergy(*lastValue);
				} 			
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightEnergy for null unit.");
			}
			break;

		case GS_CMD_setFlashlightOperable:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->setFlashlightOperable(true);
					game->gameUI->getCombatWindow( gsd->player )->showFlashlight();
				} 			
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightOperable for null unit.");
			}
			break;

		case GS_CMD_setFlashlightInoperable:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->setFlashlightOperable(false);
					game->gameUI->getCombatWindow( gsd->player )->hideFlashlight();
				} 			
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightInoperable for null unit.");
			}
			break;

		case GS_CMD_GETFLASHLIGHTENERGY:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					*lastValue = unit->getFlashlight()->getFlashlightEnergy();
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightEnergy for null unit.");
			}
			break;

		case GS_CMD_SETFLASHLIGHTON:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->setFlashlightOn(true);
				} 			
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightOn for null unit.");
			}
			break;

		case GS_CMD_SETFLASHLIGHTOFF:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->setFlashlightOn(false);
				} 			
			} else {
				sp->warning("UnitScripting::process - Attempt to setFlashlightOff for null unit.");
			}
			break;

		case GS_CMD_ISFLASHLIGHTON:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					if (unit->getFlashlight()->isFlashlightOn())
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
					sp->warning("UnitScripting::process - isFlashlightOn called for unit that has no flashlight.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isFlashlightOn for null unit.");
			}
			break;

		case GS_CMD_HASFLASHLIGHT:
			if (unit != NULL)
			{
				if (unit->getFlashlight() != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to hasFlashlight for null unit.");
			}
			break;

		case GS_CMD_ABSOLUTEINTERPOLATEPOSITIONCLOCKWISE:
		case GS_CMD_ABSOLUTEINTERPOLATEPOSITIONANTICLOCKWISE:
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				tmp.Normalize();
				tmp *= (float)intData;

				float tmpX = tmp.x; // (preserve the x value for calculations)
				float angle = (3.1415926f / 2.0f);
				if (command == GS_CMD_ABSOLUTEINTERPOLATEPOSITIONANTICLOCKWISE)
				{
					angle = -angle;
				}
				tmp.x = tmp.x * cosf(angle) + tmp.z * sinf(angle);
				tmp.z = tmp.z * cosf(angle) - tmpX * sinf(angle);

				gsd->position = unit->getPosition() + tmp;
			} else {
				if (command == GS_CMD_ABSOLUTEINTERPOLATEPOSITIONANTICLOCKWISE)
				{
					sp->warning("UnitScripting::process - Attempt to absoluteInterpolatePositionAnticlockwise for null unit.");
				} else {
					sp->warning("UnitScripting::process - Attempt to absoluteInterpolatePositionClockwise for null unit.");
				}
			}
			break;

		case GS_CMD_INTERPOLATEPOSITIONCLOCKWISE:
		case GS_CMD_INTERPOLATEPOSITIONANTICLOCKWISE:
			if (unit != NULL)
			{
				VC3 tmp = gsd->position - unit->getPosition();
				float dist = tmp.GetLength();
				dist *= ((float)intData / 100.0f);
				tmp.Normalize();

				float tmpX = tmp.x; // (preserve the x value for calculations)
				float angle = (3.1415926f / 2.0f);
				if (command == GS_CMD_INTERPOLATEPOSITIONANTICLOCKWISE)
				{
					angle = -angle;
				}
				tmp.x = tmp.x * cosf(angle) + tmp.z * sinf(angle);
				tmp.z = tmp.z * cosf(angle) - tmpX * sinf(angle);

				gsd->position = unit->getPosition() + tmp * dist;
			} else {
				if (command == GS_CMD_INTERPOLATEPOSITIONANTICLOCKWISE)
				{
					sp->warning("UnitScripting::process - Attempt to interpolatePositionAnticlockwise for null unit.");
				} else {
					sp->warning("UnitScripting::process - Attempt to interpolatePositionClockwise for null unit.");
				}
			}
			break;

		case GS_CMD_DEFORMPATHFORDARKNESS:
			if (game->inCombat)
			{
				if (unit != NULL)
				{
					if (!unit->isAtPathEnd())
					{
						float height = 1.5f;
						if (unit->getUnitType()->getAimHeightStanding() < 0.5f)
							height = 0.5f;
						util::PathDeformer::deformForDarkness(game->getGameScene()->getPathFinder(), unit->getPath(), unit->getPathIndex(), game->gameUI->getTerrain()->GetTerrain(), game->gameMap, height);
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to deformPathForDarkness for null unit.");
				}
			}
			break;

		case GS_CMD_WAITFORPATHCOMPLETE:
			if (unit != NULL)
			{
				gsd->waitDestination = true;
				gsd->waitCounter = 50; // 1 sec? or 0.1 sec? or 100/67 sec?
				*pause = true;
			} else {
				sp->warning("UnitScripting::process - Attempt to waitForPathComplete for null unit.");
			}
			break;

		case GS_CMD_ISUNITIMMORTAL:
			if (unit != NULL)
			{
				if (unit->isImmortal())
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitImmortal for null unit.");
			}
			break;

		case GS_CMD_MAKEUNITIMMORTAL:
		case GS_CMD_makeUnitImmortalWithHitScript:
			if (unit != NULL)
			{
				if(command == GS_CMD_makeUnitImmortalWithHitScript)
				{
					unit->setImmortal(true, true);
				}
				else
				{
					unit->setImmortal(true);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitImmortal / makeUnitImmortalWithHitScript for null unit.");
			}
			break;

		case GS_CMD_MAKEUNITMORTAL:
			if (unit != NULL)
			{
				unit->setImmortal(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitMortal for null unit.");
			}
			break;

		case GS_CMD_MAKEJUMP:
		case GS_CMD_MAKEJUMPTOWARDPOSITION:
			if (unit != NULL)
			{
				if (intData > 0)
				{
					if (unit->getJumpCounter() > 0)
					{
						assert("UnitScripting::process - makeJump/makeJumpTowardPosition command when already jumping.");
						sp->warning("UnitScripting::process - makeJump/makeJumpTowardPosition command when already jumping.");
					}
					UnitActor *ua = getUnitActorForUnit(unit);

					bool forward = false;
					bool backward = false;
					bool left = false;
					bool right = false;

					if (command == GS_CMD_MAKEJUMPTOWARDPOSITION)
					{
						VC3 rot = unit->getRotation();
						float angle = util::PositionDirectionCalculator::calculateDirection(unit->getPosition(), gsd->position);
						float relativeAngle = util::AngleRotationCalculator::getFactoredRotationForAngles(rot.y, angle, 0.0f);
						if (fabs(relativeAngle - 0.0f) <= 45.0f/2.0f)
						{
							forward = true;
						}
						else if (fabs(relativeAngle - 45.0f) <= 45.0f/2.0f)
						{
							forward = true;
							right = true;
						}
						else if (fabs(relativeAngle - -45.0f) <= 45.0f/2.0f)
						{
							forward = true;
							left = true;
						}
						else if (fabs(relativeAngle - 90.0f) <= 45.0f/2.0f)
						{
							right = true;
						}
						else if (fabs(relativeAngle - -90.0f) <= 45.0f/2.0f)
						{
							left = true;
						}
						else if (fabs(relativeAngle - (90.0f+45.0f)) <= 45.0f/2.0f)
						{
							backward = true;
							right = true;
						}
						else if (fabs(relativeAngle - -(90.0f+45.0f)) <= 45.0f/2.0f)
						{
							backward = true;
							left = true;
						} else {
							backward = true;
						}
					} else {
						forward = true;
					}

					ua->doSpecialMove(unit, forward, backward, left, right, intData / GAME_TICK_MSEC);

					gsd->waitCounter = intData / GAME_TICK_MSEC + 1; 
					*pause = true;
				} else {
					sp->warning("UnitScripting::process - Attempt to makeJump/makeJumpTowardPosition with zero length parameter.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to makeJump/makeJumpTowardPosition for null unit.");
			}
			break;

		case GS_CMD_ISLINEOFJUMPFORWARD:
			if (unit != NULL)
			{
				if (intData > 0)
				{
					VC3 pos = unit->getPosition();
					VC3 rot = unit->getRotation();
					pos.x += -sinf(UNIT_ANGLE_TO_RAD(rot.y)) * intData;
					pos.z += -cosf(UNIT_ANGLE_TO_RAD(rot.y)) * intData;
					if (LineOfJumpChecker::hasLineOfJump(unit, pos, game, pos, 0))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->warning("UnitScripting::process - Attempt to isLineOfJumpForward with zero length parameter.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isLineOfJumpForward for null unit.");
			}
			break;

		case GS_CMD_ISLINEOFJUMPTOWARDPOSITION:
			if (unit != NULL)
			{
				//if (intData > 0)
				//{
				if (LineOfJumpChecker::hasLineOfJump(unit, gsd->position, game, gsd->position, 0))
					*lastValue = 1;
				else
					*lastValue = 0;
				//} else {
				//	sp->warning("UnitScripting::process - Attempt to isLineOfJumpForward with zero length parameter.");
				//}
			} else {
				sp->warning("UnitScripting::process - Attempt to isLineOfJumpForward for null unit.");
			}
			break;

		case GS_CMD_ISLINEOFJUMPFORWARDATTARGET:
			if (unit != NULL)
			{
				Unit *targUnit = unit->targeting.getTargetUnit();
				if (targUnit != NULL)
				{
					VC3 targPos = targUnit->getPosition();

					VC3 distVec = targPos - unit->getPosition();
					float dist = distVec.GetLength();

					VC3 pos = unit->getPosition();
					VC3 rot = unit->getRotation();
					pos.x += -sinf(UNIT_ANGLE_TO_RAD(rot.y)) * dist;
					pos.z += -cosf(UNIT_ANGLE_TO_RAD(rot.y)) * dist;

					if (LineOfJumpChecker::hasLineOfJump(unit, pos, game, targPos, LINEOFJUMP_TARG_AREA))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isLineOfJumpForward for null unit.");
			}
			break;

		case GS_CMD_ISUNITFACINGTARGET:
			if (unit != NULL)
			{
				Unit *targUnit = unit->targeting.getTargetUnit();
				if (targUnit != NULL)
				{
					VC3 targPos = targUnit->getPosition();
					VC3 rot = unit->getRotation();

					float targAngle = util::PositionDirectionCalculator::calculateDirection(unit->getPosition(), targPos);
					float rotAngle = util::AngleRotationCalculator::getFactoredRotationForAngles(rot.y, targAngle, 0.0f);

					if (fabs(rotAngle) < (float)intData)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitFacingTarget for null unit.");
			}
			break;

		case GS_CMD_ISTARGETFACINGUNIT:
			if (unit != NULL)
			{
				Unit *targUnit = unit->targeting.getTargetUnit();
				if (targUnit != NULL)
				{
					VC3 targPos = targUnit->getPosition();
					VC3 targRot = targUnit->getRotation();

					float targAngle = util::PositionDirectionCalculator::calculateDirection(targPos, unit->getPosition());
					float rotAngle = util::AngleRotationCalculator::getFactoredRotationForAngles(targRot.y, targAngle, 0.0f);

					if (fabs(rotAngle) < (float)intData)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitFacingTarget for null unit.");
			}
			break;

		case GS_CMD_ISUNITFACINGPOSITION:
			if (unit != NULL)
			{
				VC3 rot = unit->getRotation();

				float targAngle = util::PositionDirectionCalculator::calculateDirection(unit->getPosition(), gsd->position);
				float rotAngle = util::AngleRotationCalculator::getFactoredRotationForAngles(rot.y, targAngle, 0.0f);

				if (fabs(rotAngle) < (float)intData)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitFacingPosition for null unit.");
			}
			break;

		case GS_CMD_SKIPMAINSCRIPTWAIT:
			if (unit != NULL)
			{
				// WARNING: unsafe cast!
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				ai->skipMainScriptWait();
			} else {
				sp->warning("UnitScripting::process - Attempt to skipMainScriptWait for null unit.");
			}
			break;

		case GS_CMD_DOESUNITOBSTACLEOVERLAP:
			if (unit != NULL)
			{
				if (unit->obstacleOverlaps)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to doesUnitObstacleOverlap for null unit.");
			}
			break;

		case GS_CMD_FINDDOORCOUNTERPART:
			if (unit != NULL)
			{
				LinkedList *ulist = game->units->getOwnedUnits(unit->getOwner());
				LinkedListIterator iter(ulist);
				bool foundit = false;
				while (iter.iterateAvailable())
				{ 		
					Unit *other = (Unit *)iter.iterateNext();
					// TODO: some treshold to the position...?
					if (other != unit 
						&& other->getSpawnCoordinates().x == unit->getSpawnCoordinates().x
						&& other->getSpawnCoordinates().z == unit->getSpawnCoordinates().z
						&& other->getUnitType()->hasDoorExecute())
					{
						gsd->unit = other;
						foundit = true;
						break;
					}
				}
				if (foundit)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to findDoorCounterpart for null unit.");
			}
			break;

		case GS_CMD_CLEARUNITOBSTACLEOVERLAPFLAG:
			if (unit != NULL)
			{
				unit->obstacleOverlaps = false;
			} else {
				sp->warning("UnitScripting::process - Attempt to clearUnitObstacleOverlapFlag for null unit.");
			}
			break;

		case GS_CMD_SETUNITFIRSTPERSONCONTROL:
			if (unit != NULL)
			{
				if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
				{
					for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
					{
						if (game->gameUI->getFirstPerson(c) == unit)
						{
							game->gameUI->setFirstPerson(game->singlePlayerNumber, NULL, c);							
						}
					}
					game->gameUI->setFirstPerson(game->singlePlayerNumber, unit, *lastValue);
				} else {
					sp->warning("UnitScripting::process - setUnitFirstPersonControl last value (controller number) out of range.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitFirstPersonControl for null unit.");
			}
			break;

		case GS_CMD_setFirstPersonControlledUnit:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				gsd->unit = game->gameUI->getFirstPerson(*lastValue);
			} else {
				sp->warning("UnitScripting::process - setFirstPersonControledUnit last value (controller number) out of range.");
			}
			break;

		case GS_CMD_FINDCLOSESTUNITWITHVARIABLESET:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - findClosestUnitWithVariableSet parameter missing.");
			} else {
				gsd->unit = findClosestUnitWithVariableSet(game, gsd->position, stringData, true);
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_FINDCLOSESTUNITWITHVARIABLENOTSET:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - findClosestUnitWithVariableNotSet parameter missing.");
			} else {
				gsd->unit = findClosestUnitWithVariableSet(game, gsd->position, stringData, false);
				if (gsd->unit != NULL)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_FINDCLOSESTUNITOFTYPEWITHSTRINGVALUEVARIABLESET:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - findClosestUnitOfTypeWithStringValueVariableSet parameter missing.");
			} else {
				if (gsd->stringValue == NULL)
				{
					sp->warning("UnitScripting::process - findClosestUnitOfTypeWithStringValueVariableSet called with null string value (unitvariable name expected).");
				} else {
					gsd->unit = findClosestUnitOfTypeWithVariableSet(game, gsd->position, stringData, gsd->stringValue, true);
					if (gsd->unit != NULL)
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
				}
			}
			break;

		case GS_CMD_FINDCLOSESTUNITOFTYPEWITHSTRINGVALUEVARIABLENOTSET:
			if (stringData == NULL)
			{
				sp->warning("UnitScripting::process - findClosestUnitOfTypeWithStringValueVariableNotSet parameter missing.");
			} else {
				if (gsd->stringValue == NULL)
				{
					sp->warning("UnitScripting::process - findClosestUnitOfTypeWithStringValueVariableNotSet called with null string value (unitvariable name expected).");
				} else {
					gsd->unit = findClosestUnitOfTypeWithVariableSet(game, gsd->position, stringData, gsd->stringValue, false);
					if (gsd->unit != NULL)
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
				}
			}
			break;

		case GS_CMD_SELECTUNITWEAPON:
		case GS_CMD_SELECTUNITWEAPONBYVALUE:
			if (unit != NULL)
			{
				int num = intData;
				if (command == GS_CMD_SELECTUNITWEAPONBYVALUE)
					num = *lastValue;

				int weapAmount = UNIT_MAX_WEAPONS;
				int w;
				for (w = 0; w < UNIT_MAX_WEAPONS; w++)
				{
					if (unit->getWeaponType(w) == NULL)
					{
						weapAmount = w;
						break;
					}
				}
				if (num >= -1 && num < weapAmount)
				{
					int useWeap = num;
					if (unit->getSelectedWeapon() != useWeap)
					{
						unit->setSelectedWeapon(useWeap);
						for (w = 0; w < UNIT_MAX_WEAPONS; w++)
						{
							if (w == useWeap)
							{
								unit->setWeaponActive(w, true);
							} else {
								unit->setWeaponActive(w, false);
							}
						}
					}
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setUnitWeapon/setUnitWeaponByValue for null unit.");
			}
			break;

		case GS_CMD_GETSELECTEDUNITWEAPON:
			if (unit != NULL)
			{
				*lastValue = unit->getSelectedWeapon();
			} else {
				sp->error("UnitScripting::process - Attempt to getSelectedUnitWeapon for null unit.");
			}
			break;

		case GS_CMD_isSelectedUnitWeaponOfType:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (s[0] == '$' && s[1] == '\0')
				{
					s = gsd->stringValue;
				}
				if (s != NULL)
				{
					if (PARTTYPE_ID_STRING_VALID(s))
					{
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(s));
						if (pt != NULL)
						{
							// TODO: should actually check that the given part type is a weapon ("Weap")...
							if (unit != NULL)
							{
								int selWeap = unit->getSelectedWeapon();
								if (selWeap != -1)
								{
									Weapon *weapType = unit->getWeaponType(selWeap);									
									if (weapType != NULL && (weapType->getPartTypeId() == pt->getPartTypeId() 
										|| weapType->isInherited(pt)))
									{
										*lastValue = 1;
									} else {
										*lastValue = 0;
									}
								} else {
									*lastValue = 0;
									//sp->warning("UnitScripting::process - isSelectedUnitWeaponOfType, unit has no weapon selected.");
								}
							} else {
								sp->error("UnitScripting::process - Attempt to isSelectedUnitWeaponOfType for null unit.");
							}
						} else {
							sp->error("UnitScripting::process - isSelectedUnitWeaponOfType, no part type loaded with given id.");
							LOG_DEBUG(s);
						}
					} else {
						sp->error("UnitScripting::process - isSelectedUnitWeaponOfType, invalid part type parameter.");
					}
				} else {
					sp->error("UnitScripting::process - isSelectedUnitWeaponOfType, null string value.");
				}
			} else {
				sp->error("UnitScripting::process - isSelectedUnitWeaponOfType parameter missing.");
			}
			break;

		case GS_CMD_ISUNITDOOREXECUTETYPE:
			if (unit != NULL)
			{
				if (unit->getUnitType()->hasDoorExecute())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitDoorExecuteType for null unit.");
			}
			break;

		case GS_CMD_SKIPUNITWEAPONRELOADDELAY:
			if (unit != NULL)
			{
				if (unit->getSelectedWeapon() != -1)
				{
					if (unit->getFireReloadDelay(unit->getSelectedWeapon()) > 1)
					{
						int newDelay = unit->getFireReloadDelay(unit->getSelectedWeapon());
						newDelay -= intData / GAME_TICK_MSEC;
						if (newDelay < 1) newDelay = 1;
						unit->setFireReloadDelay(unit->getSelectedWeapon(), newDelay);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to skipUnitWeaponReloadDelay for null unit.");
			}
			break;

		case GS_CMD_GETUNITWEAPONRELOADDELAY:
			if (unit != NULL)
			{
				if (unit->getSelectedWeapon() != -1)
				{
					*lastValue = unit->getFireReloadDelay(unit->getSelectedWeapon()) * GAME_TICK_MSEC;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitWeaponReloadDelay for null unit.");
			}
			break;

		case GS_CMD_SETUNITWEAPONRELOADDELAYTOVALUE:
			if (unit != NULL)
			{
				if (unit->getSelectedWeapon() != -1)
				{
					int val = (*lastValue) / GAME_TICK_MSEC;
					if (val > 0)
					{
						unit->setFireReloadDelay(unit->getSelectedWeapon(), val);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitWeaponReloadDelayToValue for null unit.");
			}
			break;

		case GS_CMD_SETUNITWALKSTOPTIMETOVALUE:
			if (unit != NULL)
			{
				int val = (*lastValue) / GAME_TICK_MSEC;
				unit->setWalkDelay(val);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitWalkStopTimeToValue for null unit.");
			}
			break;

		case GS_CMD_GETUNITWALKSTOPTIME:
			if (unit != NULL)
			{
				*lastValue = unit->getWalkDelay() * GAME_TICK_MSEC;
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitWalkStopTime for null unit.");
			}
			break;

		case GS_CMD_setUnitJumpNotAllowedTimeToValue:
			if (unit != NULL)
			{
				int val = (*lastValue) / GAME_TICK_MSEC;
				unit->setJumpNotAllowedTime(val);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitJumpNotAllowedTimeToValue for null unit.");
			}
			break;

		case GS_CMD_setUnitJumpNotAllowedTime:
			if (unit != NULL)
			{
				int val = intData / GAME_TICK_MSEC;
				unit->setJumpNotAllowedTime(val);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitJumpNotAllowedTime for null unit.");
			}
			break;

		case GS_CMD_getUnitJumpNotAllowedTime:
			if (unit != NULL)
			{
				*lastValue = unit->getJumpNotAllowedTime() * GAME_TICK_MSEC;
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitJumpNotAllowedTime for null unit.");
			}
			break;

		case GS_CMD_GETUNITWEAPONFIRETIME:
			if (unit != NULL)
			{
				if (unit->getSelectedWeapon() != -1)
				{
					*lastValue = unit->getWeaponFireTime(unit->getSelectedWeapon()) * GAME_TICK_MSEC;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitWeaponFireTime for null unit.");
			}
			break;

		case GS_CMD_GETUNITWEAPONWAITDELAY:
			if (unit != NULL)
			{
				if (unit->getSelectedWeapon() != -1)
				{
					*lastValue = unit->getFireWaitDelay(unit->getSelectedWeapon()) * GAME_TICK_MSEC;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitWeaponWaitDelay for null unit.");
			}
			break;

		case GS_CMD_LISTENTOEVENT:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					if (strcmp(stringData, "event_fire_complete") == 0)
					{
						// WARNING: unsafe cast
						UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
						ai->addEventListener(UNITLEVELAI_EVENT_MASK_FIRE_COMPLETE);
					}
					else if (strcmp(stringData, "event_jump_start") == 0)
					{
						// WARNING: unsafe cast
						UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
						ai->addEventListener(UNITLEVELAI_EVENT_MASK_JUMP_START);
					}
					else if (strcmp(stringData, "event_jump_end") == 0)
					{
						// WARNING: unsafe cast
						UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
						ai->addEventListener(UNITLEVELAI_EVENT_MASK_JUMP_END);
					} else {
						sp->warning("UnitScripting::process - listenToEvent parameter invalid, no such event name.");
						sp->debug(stringData);
					}
				} else {
					sp->warning("UnitScripting::process - listenToEvent parameter missing, event name expected.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to listenToEvent for null unit.");
			}
			break;

		case GS_CMD_GHOSTPREDICTFUTURE:
			if (unit != NULL)
			{
				for (int i = 0; i < 4; i++)
				{ 					
					Unit *fpGhost = NULL;
					if (game->gameUI->getFirstPerson(0) != NULL)
					{
						fpGhost = game->createGhostsOfFuture(game->gameUI->getFirstPerson(0));
					}
					if (fpGhost != NULL)
					{
						Unit *uGhost = game->createGhostsOfFuture(unit);
						if (uGhost != NULL)
						{
							uGhost->targeting.setTarget(fpGhost);
							fpGhost->targeting.setTarget(uGhost);
						}
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to ghostPredictFuture for null unit.");
			}
			break;

		case GS_CMD_WARPUNITTOPOSITIONFROMSECONDARYUSINGOFFSET:
			if (unit != NULL)
			{
				if (gsd->secondaryPosition.x == 0
					&& gsd->secondaryPosition.y == 0
					&& gsd->secondaryPosition.z == 0)
				{
					sp->warning("UnitScripting::process - warpUnitToPositionFromSecondaryUsingOffset, secondary position is origo, probably an error.");
				}

				VC3 offset = unit->getPosition() - gsd->secondaryPosition;

				UnitActor *ua = getUnitActorForUnit(unit);
				VC3 groundPos = gsd->position + offset;

				if (!ua->warpToPosition(unit, groundPos))
				{
					sp->warning("UnitScripting::process - warpUnitToPositionFromSecondaryUsingOffset did not find suitable place for unit.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to warpUnitToPositionFromSecondaryUsingOffset for null unit.");
			}
			break;

		case GS_CMD_DOIDLE:
			if (unit != NULL)
			{
				// TODO: proper max idle number...
				if (intData >= 1 && intData <= 8)
				{
					unit->setIdleRequest(intData);
				} else {
					sp->warning("UnitScripting::process - doIdle parameter bad.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to doIdle for null unit.");
			}
			break;

		case GS_CMD_NOUNITAIDISABLERANGE:
			if (unit != NULL)
			{
				unit->setUseAIDisableRange(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to noUnitAIDisableRange for null unit.");
			}
			break;

		case GS_CMD_FADEUNITVISIBILITYUSINGTIMEVALUE:
			if (unit != NULL)
			{
				//sp->debug("UnitScripting::process - fadeUnitVisibilityUsingTimeValue");
				// TODO: time value
				int timeValue = *lastValue;
				if (timeValue < GAME_TICK_MSEC)
					sp->warning("UnitScripting::process - fadeUnitVisibilityUsingTimeValue, value given for time invalid (less than game tick).");
				float floatData = intFloat.f;
				unit->fadeVisibility(floatData, timeValue / GAME_TICK_MSEC);
			} else {
				sp->warning("UnitScripting::process - Attempt to fadeUnitVisibilityUsingTimeValue for null unit.");
			}
			break;

		case GS_CMD_SETUNITVISIBILITY:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				unit->setFadeVisibilityImmediately(floatData);
			} else {
				sp->warning("UnitScripting::process - Attempt to fadeUnitVisibilityUsingTimeValue for null unit.");
			}
			break;

		case GS_CMD_FADEUNITLIGHTINGUSINGTIMEVALUE:
			if (unit != NULL)
			{
				// TODO: time value
				int timeValue = *lastValue;
				if (timeValue < GAME_TICK_MSEC)
					sp->warning("UnitScripting::process - fadeUnitLightingUsingTimeValue, value given for time invalid (less than game tick).");
				float floatData = intFloat.f;
				unit->fadeLighting(floatData, timeValue / GAME_TICK_MSEC);
			} else {
				sp->warning("UnitScripting::process - Attempt to fadeUnitLightingUsingTimeValue for null unit.");
			}
			break;

		case GS_CMD_ENABLEUNITCOLLISIONCHECK:
			if (unit != NULL)
			{
				unit->setCollisionCheck(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to enableUnitCollisionCheck for null unit.");
			}
			break;

		case GS_CMD_DISABLEUNITCOLLISIONCHECK:
			if (unit != NULL)
			{
				unit->setCollisionCheck(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to enableUnitCollisionCheck for null unit.");
			}
			break;

		case GS_CMD_SETAREACENTERTOPOSITION:
			if (unit != NULL)
			{
				unit->setAreaCenter(gsd->position);
			} else {
				sp->warning("UnitScripting::process - Attempt to setAreaCenterToPosition for null unit.");
			}
			break;

		case GS_CMD_SETAREARANGETOVALUE:
			if (unit != NULL)
			{
				unit->setAreaRange((float)*lastValue);
			} else {
				sp->warning("UnitScripting::process - Attempt to setAreaRange for null unit.");
			}
			break;

		case GS_CMD_SETAREACLIPTOVALUE:
			if (unit != NULL)
			{
				unit->setAreaClipMask(*lastValue);
			} else {
				sp->warning("UnitScripting::process - Attempt to setAreaClip for null unit.");
			}
			break;

		case GS_CMD_HASAREATRIGGERED:
			if (unit != NULL)
			{
				if (unit->hasAreaTriggered())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to hasAreaTriggered for null unit.");
			}
			break;

		case GS_CMD_AREALISTENTOPLAYERUNITS:
			if (unit != NULL)
			{
				if (gsd->originalUnit != NULL && unit != gsd->originalUnit)
				{
					sp->warning("UnitScripting::process - areaListenToPlayerUnits, unit is not the original script unit (possibly missing restoreUnit somewhere).");
				}
				if (unit->getAreaClipMask() >= 1 && unit->getAreaClipMask() <= 15 && unit->getAreaRange() > 0)
				{
					if (game->units->getOwnedUnitAmount(gsd->player) > 0)
					{
						LinkedList *ulist = game->units->getOwnedUnits(gsd->player);
						LinkedList ulistfixed;
						LinkedListIterator iter(ulist);
						while (iter.iterateAvailable())
						{
							Unit *tmp = (Unit *)iter.iterateNext();
							if (tmp->isActive() && !tmp->isDestroyed())
							{
								ulistfixed.append(tmp);
							}
						}
						game->gameScripting->addAreaTrigger(unit, unit->getAreaCenter(), unit->getAreaRange(), unit->getAreaClipMask(), &ulistfixed);
						while (!ulistfixed.isEmpty())
						{
							ulistfixed.popLast();
						}
					} else {
						sp->warning("UnitScripting::process - areaListenToPlayerUnits, player has no units to track.");
					}
				} else {
					sp->warning("UnitScripting::process - areaListenToPlayerUnits, bad area range/clip parameters.");
					std::string dbgtmp = std::string("range cm = ") + int2str((int)(unit->getAreaRange() * 100.0f));
					dbgtmp += std::string(", clip = ") + int2str(unit->getAreaClipMask());
					Logger::getInstance()->debug(dbgtmp.c_str());
					fb_assert(!"UnitScripting::process - areaListenToPlayerUnits, bad area range/clip parameters.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to areaListenToPlayerUnits for null unit.");
			}
			break;

		case GS_CMD_STARTFOLLOWPLAYER:
			if (unit != NULL)
			{
				unit->setFollowPlayer(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to followPlayer for null unit.");
			}
			break;

		case GS_CMD_STOPFOLLOWPLAYER:
			if (unit != NULL)
			{
				unit->setFollowPlayer(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to setFollowPlayer for null unit.");
			}
			break;

		case GS_CMD_ISUNITONFIRE:
			if (unit != NULL)
			{
				if (unit->isOnFire())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitOnFire for null unit.");
			}
			break;

		case GS_CMD_SETUNITONFIRE:
			if (unit != NULL)
			{
				unit->setOnFireCounter(intData / GAME_TICK_MSEC);
				if (intData < GAME_TICK_MSEC)
				{
					sp->warning("UnitScripting::process - makeUnitOnFire parameter out of range.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitOnFire for null unit.");
			}
			break;

		case GS_CMD_MARKALIVEHOSTILEUNITS:
			UnitSpawner::markAliveHostileUnits(game);
			break;

		case GS_CMD_RESPAWNMARKEDHOSTILEUNITSALIVE:
			UnitSpawner::respawnMarkedHostileUnitsAlive(game);
			break;

		case GS_CMD_DELETEALLUNITPARTS:
			if (unit != NULL)
			{
				// no reason why it should be active?
				//if (unit->isActive())
				{
					if (unit->isMuzzleflashVisible())
						unit->setMuzzleflashVisualEffect(NULL, 0);

					game->deleteVisualOfParts(unit, unit->getRootPart());

					if (unit->getRootPart() != NULL)
					{
						for (int i = 0; i < MAX_PART_CHILDREN; i++)
						{
							if (unit->getRootPart()->getSubPart(i) != NULL)
								game->removeParts(unit, unit->getRootPart()->getSubPart(i));
						}
					}

					// HACK: copy&pasted (and modified?) from addSubPart!

					game->createVisualForParts(unit, unit->getRootPart());

					if (unit->getFlashlight() != NULL)
					{
						unit->getFlashlight()->resetOrigin(unit->getVisualObject());
					}

					// renew the lost animation :D
					if (unit->getWalkDelay() < 2)
						unit->setWalkDelay(2);
					if (unit->getAnimationSet() != NULL)
					{
						unit->setAnimation(0); // ANIM_NONE
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
							unit->getAnimationSet()->animate(unit, ANIM_STAND);
					}

					unit->uninitWeapons();
					unit->deleteCustomizedWeaponTypes();
					unit->initWeapons();

					unit->setSelectedWeapon(-1);
					unit->setSelectedSecondaryWeapon(-1);

					for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
					{
						// NOTE: oh ffs. the previously added weapons lose the clip ammo amount
						// when weapons are re-initialized - thus, must reload ALL weapon here!
						//if (unit->getWeaponType(w) == pt)
						//{
							unit->reloadWeaponAmmoClip(w, true);
						//}

						if (w == unit->getSelectedWeapon())
							unit->setWeaponActive(w, true);
						else
							unit->setWeaponActive(w, false);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to deleteAllUnitParts for null unit.");
			}
			break;

		case GS_CMD_RECREATEVISUALFORUNIT:
			if (unit != NULL)
			{
				if (unit->isActive())
				{
					// HACK: copy & pasted once again

					if (unit->isMuzzleflashVisible())
						unit->setMuzzleflashVisualEffect(NULL, 0);

					game->deleteVisualOfParts(unit, unit->getRootPart(), true);
					game->createVisualForParts(unit, unit->getRootPart(), true);

					if (unit->getFlashlight() != NULL)
					{
						unit->getFlashlight()->resetOrigin(unit->getVisualObject());
					}

					// renew the lost animation :D
					if (unit->getWalkDelay() < 2)
						unit->setWalkDelay(2);
					if (unit->getAnimationSet() != NULL)
					{
						unit->setAnimation(0); // ANIM_NONE
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
							unit->getAnimationSet()->animate(unit, ANIM_STAND);
					}

					if (unit->getVisualObject() != NULL)
					{
						VC3 rot = unit->getRotation();
						VC3 pos = unit->getPosition();
						unit->setPosition(pos);
						unit->setRotation(rot.x, rot.y, rot.z);
						unit->getVisualObject()->forcePositionUpdate();
					}
				}

			} else {
				sp->warning("UnitScripting::process - Attempt to recreateVisualForUnit for null unit.");
			}
			break;

		case GS_CMD_GETAMMOFORUNITWEAPON:
			if (unit != NULL)
			{
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - getAmmoForUnitWeapon, expected part type id.");
					else
						sp->error("UnitScripting::process - getAmmoForUnitWeapon, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - getAmmoForUnitWeapon, reference to unloaded part type.");
					} else {
						if (unit->isActive())
						{
							assert(unit->isActive());
							int wnum = unit->getWeaponByWeaponType(pt->getPartTypeId());
							if (wnum != -1)
							{
								int ammoa = unit->getWeaponAmmoAmount(wnum);
								*lastValue = ammoa;
							} else {
								*lastValue = 0;
								sp->debug("UnitScripting:process - getAmmoForUnitWeapon, unit did not have such weapon");
							}
						} else {
							if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
							{
								// WARNING: unsafe cast!
								Weapon *w = (Weapon *)pt;
								AmmoPack *ap = w->getAmmoType();
								if (ap != NULL)
								{
									Part *p = unit->seekPartOfPartType(ap);
									if (p != NULL)
									{
										// WARNING: unsafe cast!
										AmmoPackObject *apo = (AmmoPackObject *)p;
										*lastValue = apo->getAmount();
									} else {
										sp->debug("UnitScripting:process - getAmmoForUnitWeapon, unit (inactive) did not have such weapon");
										*lastValue = 0;
									}
								} else {
									sp->debug("UnitScripting:process - getAmmoForUnitWeapon, weapon does not use any ammo type.");
									*lastValue = 0;
								}
							} else {
								sp->error("UnitScripting:process - getAmmoForUnitWeapon, part type is of not weapon type.");
							}
						}
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to getAmmoForUnitWeapon for null unit.");
			}
			break;

		case GS_CMD_SETAMMOTOPARTBYVALUE:
			if (unit != NULL)
			{
				if (gsd->part != NULL)
				{
					// TODO: should allow loading of "Weap" type parts...?
					PartType *ammopt = getPartTypeById(
						PARTTYPE_ID_STRING_TO_INT("Ammo"));
					
					if (gsd->part->getType()->isInherited(ammopt))
					{ 					
						int val = *lastValue;
						if (val < 0)
						{
							val = 0;
							sp->warning("UnitScripting::process - Attempt to setAmmoToPartByValue with negative value.");
						}
						if (unit->isActive())
						{
							// WARNING: unsafe cast (checked by the above if clause)
							AmmoPack *ap = (AmmoPack *)gsd->part->getType();
							if (unit->setWeaponAmmo(ap, val))
							{
								*lastValue = 1;
							} else {
								*lastValue = 0;
							}
						} else {
							// WARNING: unsafe cast (checked by the above if clause)
							AmmoPackObject *apo = (AmmoPackObject *)gsd->part;
							apo->setAmount(val);
						}
					} else {
						sp->warning("UnitScripting::process - Attempt to setAmmoToPartByValue to non-ammo part.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to setAmmoToPartByValue for null part.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setAmmoToPartByValue for null unit.");
			} 								
			break;

		case GS_CMD_STOPALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						u->setPath(NULL);
						u->setFinalDestination(u->getPosition());
						u->setWaypoint(u->getPosition());
					}
				}
			}
			break;

		case GS_CMD_CLEARTARGETFORALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						u->targeting.clearTarget();
					}
				}
			}
			break;

		case GS_CMD_HIDEALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						if (u->getVisualObject() != NULL)
						{
							u->getVisualObject()->setVisible(false);
						}
					}
				}
			}
			break;

		case GS_CMD_HIDEALLUNITS:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed())
					{
						if (u->getVisualObject() != NULL)
						{
							u->getVisualObject()->setVisible(false);
						}
					}
				}
			}
			break;

		case GS_CMD_enableTouchBullet:
			if (unit != NULL)
			{
				Unit *u = unit;
				if (u->isActive() && !u->isDestroyed())
				{
					u->setTouchProjectileEnabled(true);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to enableTouchBullet for null unit.");
			}
			break;

		case GS_CMD_disableTouchBullet:
			if (unit != NULL)
			{
				Unit *u = unit;
				if (u->isActive() && !u->isDestroyed())
				{
					u->setTouchProjectileEnabled(false);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to disableTouchBullet for null unit.");
			}
			break;


		case GS_CMD_SHOWUNIT:
			if (unit != NULL)
			{
				Unit *u = unit;
#ifdef PROJECT_CLAW_PROTO
				if (u->isActive())
#else
				if (u->isActive() && !u->isDestroyed())
#endif
				{
					if (u->getVisualObject() != NULL)
					{
						u->getVisualObject()->setVisible(true);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to showUnit for null unit.");
			}
			break;

		case GS_CMD_HIDEUNIT:
			if (unit != NULL)
			{
				Unit *u = unit;
#ifdef PROJECT_CLAW_PROTO
				if (u->isActive())
#else
				if (u->isActive() && !u->isDestroyed())
#endif
				{
					if (u->getVisualObject() != NULL)
					{
						u->getVisualObject()->setVisible(false);
						u->setPointerVisualEffect(NULL);
						u->setPointerHitVisualEffect(NULL);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to hideUnit for null unit.");
			}
			break;

		case GS_CMD_DISABLEUNITAITEMPORARILYFORALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						UnitLevelAI *ai = (UnitLevelAI *)u->getAI();
						if (ai != NULL)
						{
							ai->setTempDisabled(true);
						}
					}
				}
			}
			break;

		case GS_CMD_ENABLEUNITAITEMPORARILYFORALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						UnitLevelAI *ai = (UnitLevelAI *)u->getAI();
						if (ai != NULL)
						{
							ai->setTempDisabled(false);
						}
					}
				}
			}
			break;

		case GS_CMD_SHOWALLHOSTILES:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed()
						&& game->isHostile(gsd->player, u->getOwner()))
					{
						if (u->getVisualObject() != NULL)
						{
							u->getVisualObject()->setVisible(true);
						}
					}
				}
			}
			break;

		case GS_CMD_SHOWALLUNITS:
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed())
					{
						if (u->getVisualObject() != NULL)
						{
							u->getVisualObject()->setVisible(true);
						}
					}
				}
			}
			break;

		case GS_CMD_GETUNITEXECUTETIP:
			if (unit != NULL)
			{
				gsd->setStringValue(unit->getExecuteTipText());
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitExecuteTip for null unit.");
			}
			break;

		case GS_CMD_SETUNITEXECUTETIPTOSTRINGVALUE:
			if (unit != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					unit->setExecuteTipText(gsd->stringValue);
				} else {
					sp->warning("UnitScripting::process - Attempt to setUnitExecuteTipToStringValue for null string value (use clearUnitExecuteTip instead).");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitExecuteTipToStringValue for null unit.");
			}
			break;

		case GS_CMD_CLEARUNITEXECUTETIP:
			if (unit != NULL)
			{
				unit->setExecuteTipText(NULL);
			} else {
				sp->warning("UnitScripting::process - Attempt to clearUnitExecuteTip for null unit.");
			}
			break;

		case GS_CMD_ISUNITEXECUTABLEBYPLAYER:
			{
				*lastValue = 0;
				// HACK: player1
				Unit *pl1 = game->units->getUnitByIdString("player1");
				if (pl1 != NULL && pl1 != unit)
				{
					UnitActor *ua = getUnitActorForUnit(pl1);
					if (ua->canDoUnitExecute(pl1, unit))
					{
						*lastValue = 1;
					}
				}
			}
			break;

		case GS_CMD_HASUNITDIEDBYPOISON:
			if (unit != NULL)
			{
				if (unit->isDestroyed() && unit->hasDiedByPoison())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->warning("UnitScripting::process - Attempt to hasUnitDiedByPoison for null unit.");
			}
			break;

		case GS_CMD_COPYEXTPATHFROMORIGINALUNIT:
			// THE HORRIBLE COPY PASTE HACK!!!
			// TODO: tidy up
			if (unit != NULL)
			{
				if (gsd->originalUnit != NULL)
				{
					if (gsd->originalUnit != unit)
					{
						if (stringData != NULL)
						{
							int foundPath = -1;
							int allocedOrigPaths = gsd->originalUnit->scriptPaths.getAllocatedStoredPaths();
							{
								for (int i = 1; i < allocedOrigPaths; i++)
								{
									if (gsd->originalUnit->scriptPaths.isStoredPathUsed(i)
										&& gsd->originalUnit->scriptPaths.isStoredPathStart(i))
									{
										if (gsd->originalUnit->scriptPaths.getStoredPathName(i) != NULL
											&& strcmp(stringData, gsd->originalUnit->scriptPaths.getStoredPathName(i)) == 0)
										{
											foundPath = i;
											break;
										}
									}
								}
							}
							int addFromPath = 1;
							//bool alreadyHasExt = false;
							if (foundPath != -1)
							{
								// BUGBUG?!? Shouldn't it be
								// int allocedUnitPaths = unit->scriptPaths.getAllocatedStoredPaths(); ??
								int allocedUnitPaths = unit->scriptPaths.getAllocatedStoredPaths();
								// NOTE: currently cannot add more than 1 ext_path
								// FIXME: erase old ext_path so that additional ones could be added.
								for (int i = 1; i < allocedUnitPaths; i++)
								{
									if (unit->scriptPaths.isStoredPathUsed(i))
									{
										addFromPath = i + 1;
									}
									if (unit->scriptPaths.isStoredPathUsed(i)
										&& unit->scriptPaths.isStoredPathStart(i))
									{
										if (unit->scriptPaths.getStoredPathName(i) != NULL
											&& strcmp("ext_path", unit->scriptPaths.getStoredPathName(i)) == 0)
										{
											//alreadyHasExt = true;
											addFromPath = i;
											break;
										}
									}
								}
							}
							//if (foundPath != -1 && !alreadyHasExt)
							if (foundPath != -1)
							{
								int endPathNum = -1;
								{
									// find out where the end piece for this path is...
									for (int i = 1; i < allocedOrigPaths; i++)
									{
										if (gsd->originalUnit->scriptPaths.isStoredPathUsed(i)
											&& gsd->originalUnit->scriptPaths.isStoredPathEnd(i))
										{
											endPathNum = i;
											break;
										}
									}
								}
								if (endPathNum != -1)
								{
									gsd->lastStoredPath = addFromPath;

									// copied: "pathName"
									gsd->setStorePathName("ext_path");

									for (int i = foundPath; i < endPathNum + 1; i++)
									{
										// copy to current unit's path...

										VC3 oldPos = gsd->originalUnit->scriptPaths.getStoredPathStartPosition(i);
										VC3 pos = gsd->originalUnit->scriptPaths.getStoredPathEndPosition(i);

										if (i == foundPath)
										{
											// copied: "pathStart"
											unit->scriptPaths.setStoredPathStart(0, pos, unit, gsd->storePathName);
										}
										else if (i == endPathNum)
										{
											// copied: "pathEnd"
											unit->scriptPaths.setStoredPathEnd(0, pos, unit);
										} else {
											// copied: "pathTo"
											UnitActor *ua = getUnitActorForUnit(unit);
											if (ua != NULL)
											{
												frozenbyte::ai::Path *path = ua->solvePath(unit, oldPos, pos);
												// notice: gsd->position may have been changed by getPath
												// if it was blocked.
												unit->setStoredPath(0, path, oldPos, pos);
												if (path == NULL)
												{
													sp->debug("UnitScripting::process - copyExtPathFromOriginalUnit failed to find path.");
												}
											}
										}

										// copied: "storeNextPath"
										gsd->lastStoredPath++;
										unit->scriptPaths.moveStoredPath(0, gsd->lastStoredPath);
									}								
								} else {
									sp->error("UnitScripting::process - copyExtPathFromOriginalUnit failed to find end point for given path (internal error).");
								}
							} else {
								//if (alreadyHasExt)
								//{
								//	sp->warning("UnitScripting::process - copyExtPathFromOriginalUnit cannot be done as unit already has ext_path (todo, support for ext_path recreate).");
								//} else {
									sp->warning("UnitScripting::process - copyExtPathFromOriginalUnit failed to find path with given name.");
								//}
							}
						} else {
							sp->error("UnitScripting::process - copyExtPathFromOriginalUnit parameter missing, path name expected.");
						}
					} else {
						// NOTE: this might still work? copying unit's another path to name "ext_path"..
						sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit when unit is the same as original unit.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit for null original unit.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit for null unit.");
			}
			break;

		case GS_CMD_CREATEUNITEFFECTLAYERUSINGTIMEVALUE:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					if (*lastValue >= GAME_TICK_MSEC)
					{
						assert(unit->isActive());

						if (unit->isActive())
						{
							int effDuration = *lastValue / GAME_TICK_MSEC;
							bool effTypeOk = true;
							if (strcmp(stringData, "electric") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_ELECTRIC, effDuration);
							else if (strcmp(stringData, "burning") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_BURNING, effDuration);
							else if (strcmp(stringData, "slime") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_SLIME, effDuration);
							else if (strcmp(stringData, "cloak") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_CLOAK, effDuration);
							else if (strcmp(stringData, "cloakhit") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_CLOAKHIT, effDuration);
							else if (strcmp(stringData, "protectiveskin") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_PROTECTIVESKIN, effDuration);
							else if (strcmp(stringData, "red_cloak") == 0)
								unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_CLOAKRED, effDuration);
							else 
								effTypeOk = false;

							if (effTypeOk)
							{
								game->deleteVisualOfParts(unit, unit->getRootPart(), true);
								game->createVisualForParts(unit, unit->getRootPart(), true);
							} else {
								sp->warning("UnitScripting::process - createUnitEffectLayerUsingTimeValue parameter invalid, effect layer type expected.");
							}
						}
					} else {
						sp->warning("UnitScripting::process - Attempt to createUnitEffectLayerUsingTimeValue with time value less than one game tick.");
					}
				} else {
					sp->warning("UnitScripting::process - createUnitEffectLayerUsingTimeValue parameter missing, effect layer type expected.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to createUnitEffectLayerUsingTimeValue for null unit.");
			}
			break;

		case GS_CMD_DELETEUNITEFFECTLAYER:
			if (unit != NULL)
			{
				if (unit->getUnitEffectLayerDuration() > 1)
				{
					// NOTE: cloakhit will normally change to cloak effect once it's done,
					// that is not desirable in this case, so first change to cloak effect.
					if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_CLOAKHIT)
					{
						// NOTE: must first set duration to 1 to allow overriding cloakhit
						// (so the following line is necessary first)
						unit->setUnitEffectLayerDuration(1);
						unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_CLOAK, 1);
					} else {
						unit->setUnitEffectLayerDuration(1);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to clearUnitEffectLayer for null unit.");
			}
			break;

		case GS_CMD_SETUNITFORCEDLIGHTRENDERVISIBILITY:
			if (unit != NULL)
			{
				assert(unit->isActive());
				if (unit->isActive())
				{
					float floatData = intFloat.f;

					unit->setLightVisibilityFactor(floatData, true);

					if(unit->getVisualObject())
						unit->getVisualObject()->setLightVisibilityFactor(floatData, true);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitForcedLightRenderVisibility for null unit.");
			}
			break;

		case GS_CMD_ENDUNITFORCEDLIGHTRENDERVISIBILITY:
			if (unit != NULL)
			{
				assert(unit->isActive());
				if (unit->isActive())
				{
					unit->setLightVisibilityFactor(0.0f, false);

					if(unit->getVisualObject())
						unit->getVisualObject()->setLightVisibilityFactor(0.f, false);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to endUnitForcedLightRenderVisibility for null unit.");
			}
			break;

		case GS_CMD_CLEARUNITHITSCRIPTDELAY:
			if (unit != NULL)
			{
				if (unit->getHitScriptDelay() > 0)
				{
					unit->setHitScriptDelay(0);
					unit->setHitByUnit(NULL, NULL);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to clearUnitHitScriptDelay for null unit.");
			}
			break;

		case GS_CMD_SETUNITRUSHINGON:
			if (unit != NULL)
			{
				unit->setRushing(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitRushingOn for null unit.");
			}
			break;

		case GS_CMD_SETUNITRUSHINGOFF:
			if (unit != NULL)
			{
				unit->setRushing(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitRushingOff for null unit.");
			}
			break;

		case GS_CMD_RESTOREUNITRUSHING:
			if (unit != NULL)
			{
				unit->setRushing(unit->getUnitType()->isRusher());
			} else {
				sp->warning("UnitScripting::process - Attempt to restoreUnitRushing for null unit.");
			}
			break;

		case GS_CMD_setUnitHighlightStyle:
			if (unit != NULL)
			{
				unit->setHighlightStyle( intData );
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitHighlightStyle for null unit.");
			}
			break;

		case GS_CMD_setUnitHighlightText:
			if (unit != NULL)
			{
				if( stringData != NULL )
					unit->setHighlightText( stringData );
				else 
					unit->setHighlightText( "" );
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitHighlightText for null unit.");
			}
			break;

		case GS_CMD_moveOriginalUnitToUnitHelper:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					if (gsd->originalUnit != NULL)
					{
						if (gsd->originalUnit != unit)
						{
							if (unit->getVisualObject() != NULL
								&& unit->getVisualObject()->getStormModel() != NULL)
							{
								IStorm3D_Model *model = unit->getVisualObject()->getStormModel();
								VC3 tmp = VC3(0,0,0);
								bool helperFound = util::getHelperPosition(model, stringData, tmp);
								VC3 rot = unit->getRotation();
								if (!helperFound)
								{
									tmp = unit->getPosition();
									sp->warning("UnitScripting::process - moveOriginalUnitToUnitHelper, helper with given name not found.");
								} else {
									// HACK: fix, if unit was out of screen, the helper position
									// is not up-to-date (it will be 0,0,0?), use the unit
									// position instead...
									// WARNING: relying on Storm3D to initialize the helper
									// positions to 0,0,0 until updated.
									// FIXME: storm does not initialize to 0,0,0 - this bugs???
									if (tmp.x == 0 && tmp.y == 0 && tmp.z == 0)
									{
										tmp = unit->getPosition();
									}
								}
								gsd->originalUnit->setPosition(tmp);
								float newAngle = rot.y + unit->getLastBoneAimDirection();
								if (newAngle >= 360.0f) newAngle -= 360.0f;
								if (newAngle < 0.0f) newAngle += 360.0f;
								gsd->originalUnit->setRotation(rot.x, newAngle, rot.z);
							} else {
								sp->warning("UnitScripting::process - Attempt to moveOriginalUnitToUnitHelper for unit with no visual object or model.");
							}
						} else {
							sp->warning("UnitScripting::process - Attempt to moveOriginalUnitToUnitHelper for same unit and original unit.");
						}
					} else {
						sp->warning("UnitScripting::process - Attempt to moveOriginalUnitToUnitHelper for null original unit.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to moveOriginalUnitToUnitHelper for null unit.");
				}
			} else {
				sp->warning("UnitScripting::process - moveOriginalUnitToUnitHelper parameter missing, helper name expected.");
			}
			break;

		case GS_CMD_resetUnitMaterialEffects:
			if (unit != NULL)
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->resetMaterialEffects();
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to resetUnitMaterialEffects for null unit.");
			}
			break;

		case GS_CMD_setUnitHaloBrightnessToValue:
			if (unit != NULL)
			{
				if (unit->getSecondarySpotlight() != NULL)
				{
					float val = float(*lastValue) / 100.0f;
					//COL col = COL(val, val, val);
					unit->getSecondarySpotlight()->setFakelightBrightness(val);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitHaloBrightness for null unit.");
			}
			break;

		case GS_CMD_setUnitObjectSelfIllumination:
			if (unit != NULL)
			{
				if(unit->getVisualObject() && unit->getVisualObject()->getStormModel())
				{
					IStorm3D_Model *model = unit->getVisualObject()->getStormModel();
					boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
					for(; !object_iterator->IsEnd(); object_iterator->Next())
					{
						IStorm3D_Model_Object *o = object_iterator->GetCurrent();
						if(!o)
							continue;
						IStorm3D_Mesh *mesh = o->GetMesh();
						if(!mesh)
							continue;
						IStorm3D_Material *material = mesh->GetMaterial();
						if(!material)
							continue;

						float f = intFloat.f;
						material->SetSelfIllumination(COL(f,f,f));
						material->SetGlowFactor(f);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitObjectSelfIllumination for null unit.");
			}
			break;

		case GS_CMD_setUnitWeaponInoperable:
		case GS_CMD_setUnitWeaponOperable:
			if (unit != NULL)
			{
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - setUnitWeaponInoperable/Operable, expected part type id.");
					else
						sp->error("UnitScripting::process - setUnitWeaponInoperable/Operable, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - setUnitWeaponInoperable/Operable, reference to unloaded part type.");
					} else {
						assert(unit->isActive());
						int newweap = unit->getWeaponByWeaponType(pt->getPartTypeId());
						if (newweap != -1)
						{
							if (command == GS_CMD_setUnitWeaponOperable)
							{
								unit->setWeaponOperable(newweap, true);
							} else {
								unit->setWeaponOperable(newweap, false);
							}
#ifdef PROJECT_SURVIVOR
							// update weapon window
							WeaponWindow *win;						
							win = (WeaponWindow *)game->gameUI->getCombatWindow( 0 )->getSubWindow( "SurvivorMarineWeaponWindow" );
							if(win)	win->forceUpdate();
							win = (WeaponWindow *)game->gameUI->getCombatWindow( 0 )->getSubWindow( "SurvivorNapalmWeaponWindow" );
							if(win)	win->forceUpdate();
							win = (WeaponWindow *)game->gameUI->getCombatWindow( 0 )->getSubWindow( "SurvivorSniperWeaponWindow" );
							if(win)	win->forceUpdate();
#endif
						} else {
							sp->warning("UnitScripting:process - setUnitWeaponInoperable/Operable, unit did not have such weapon");
						}
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitWeaponInoperable/Operable for null unit.");
			}
			break;

		case GS_CMD_makeUnitUnspottable:
			if (unit != NULL)
			{
				unit->setSpottable(false);
				// TODO: should loop thru all units and clear targets for ones that 
				// have targeted this unit?
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitUnspottable for null unit.");
			}
			break;

		case GS_CMD_makeUnitSpottable:
			if (unit != NULL)
			{
				unit->setSpottable(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitSpottable for null unit.");
			}
			break;

		case GS_CMD_setGlowFactor:
			if (unit != NULL)
			{
				if(unit->getVisualObject() && unit->getVisualObject()->getStormModel())
				{
					IStorm3D_Model *model = unit->getVisualObject()->getStormModel();
					boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
					for(; !object_iterator->IsEnd(); object_iterator->Next())
					{
						IStorm3D_Model_Object *o = object_iterator->GetCurrent();
						if(!o)
							continue;
						IStorm3D_Mesh *mesh = o->GetMesh();
						if(!mesh)
							continue;
						IStorm3D_Material *material = mesh->GetMaterial();
						if(!material)
							continue;

						float f = intFloat.f;
						material->SetGlowFactor(f);
					}
				}

			} else {
				sp->warning("UnitScripting::process - Attempt to setGlowFactor for null unit.");
			}
			break;

		case GS_CMD_isUnitHitByBullet:
			if (unit != NULL)
			{
				if (unit != gsd->originalUnit)
				{
					sp->error("UnitScripting::process - isUnitHitByBullet can only be called inside hit script for original hit script unit.");
				}
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - isUnitHitByBullet, expected part type id.");
					else
						sp->error("UnitScripting::process - isUnitHitByBullet, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - isUnitHitByBullet, reference to unloaded part type.");
					} else {
						PartType *bulletpt = getPartTypeById(
							PARTTYPE_ID_STRING_TO_INT("Bull"));

						if (bulletpt != NULL && pt->isInherited(bulletpt))
						{ 						
							Bullet *b = (Bullet *)pt;

							if (b == gs_hitscript_hit_bullet_type)
							{
								*lastValue = 1;
							} else {
								// new: accept inherited bullet types too.
								if (gs_hitscript_hit_bullet_type != NULL
									&& b != NULL
									&& gs_hitscript_hit_bullet_type->isInherited(b))
								{
									*lastValue = 1;
								} else {
									*lastValue = 0;
								}
							}

						} else {
							sp->warning("UnitScripting::process - Attempt to isUnitHitByBullet to non-bullet part.");
						}
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitHitByBullet for null unit.");
			} 								
			break;

		case GS_CMD_isUnitHitByElectricBullet:
			if (unit != NULL)
			{
				if (unit != gsd->originalUnit)
				{
					sp->error("UnitScripting::process - isUnitHitByElectricBullet can only be called inside hit script for original hit script unit.");
				}
				if (gs_hitscript_hit_bullet_type != NULL)
				{
					if (gs_hitscript_hit_bullet_type->getHitDamage(DAMAGE_TYPE_ELECTRIC) > 0)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->warning("UnitScripting::process - Attempt to isUnitHitByElectricBullet when null hit bullet type info.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitHitByElectricBullet for null unit.");
			} 								
			break;

		case GS_CMD_isUnitElectrified:
			if (unit != NULL)
			{
				if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_ELECTRIFIED)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitHitByBullet for null unit.");
			} 								
			break;

		case GS_CMD_rotateUnitAroundUnitByValue:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					UnitActor *ua = getUnitActorForUnit(unit);

					Unit *otherUnit = game->units->getUnitByIdString(stringData);
					if (otherUnit != NULL)
					{
						if (*lastValue != 0)
						{
							VC3 unitPos = unit->getPosition();
							VC3 otherPos = otherUnit->getPosition();

							//float angle = util::PositionDirectionCalculator::calculateDirection(otherPos, unitPos);

							ua->removeUnitObstacle(unit);

							VC3 diffvec = unitPos - otherPos;

							float angle = float(*lastValue) / 100.0f * 3.1415f / 180.0f;

							float diffvecX = diffvec.x;
							diffvec.x = diffvec.x * cosf(angle) + diffvec.z * sinf(angle);
							diffvec.z = diffvec.z * cosf(angle) - diffvecX * sinf(angle);

							VC3 newPos = otherPos + diffvec;

							unit->setPosition(newPos);

							ua->addUnitObstacle(unit);
						}

					} else {
						sp->warning("UnitScripting::process - rotateUnitAroundUnitByValue, no unit found with given id string.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to rotateUnitAroundUnitByValue for null unit.");
				} 					
			} else {
				sp->error("UnitScripting::process - rotateUnitAroundUnitByValue parameter missing, another unit id string expected.");
			}
			break;

		case GS_CMD_setUnitTypeData:
			if (unit != NULL)
			{
				if (stringData != NULL)
				{
					char *tmpbuf = new char[strlen(stringData) + 1];
					strcpy(tmpbuf, stringData);
					char *sep = strstr(tmpbuf, "=");
					if (sep != NULL)
					{
						*sep = '\0';

						bool redoObstacles = true;
						// HACK: if changing the unit type's block radius, gotta redo all obstacles
						char *posstr = strstr(tmpbuf, "blockradius");
						if (posstr != NULL)
						{
							redoObstacles = true;
						}

						LinkedList removedObstaclesList;

						if (redoObstacles)
						{
							LinkedList *ulist = game->units->getAllUnits();
							LinkedListIterator iter(ulist);
							while (iter.iterateAvailable())
							{
								Unit *u = (Unit *)iter.iterateNext();
								if (u->getUnitType() == unit->getUnitType())
								{
									if (u->obstacleExists)
									{
										UnitActor *ua = getUnitActorForUnit(u);
										ua->removeUnitObstacle(u);
										removedObstaclesList.append(u);
									}
								}
							}
						}
						bool success = unit->getUnitType()->setData(tmpbuf, &sep[1]);
						if (!success)
						{
							sp->error("UnitScripting::process - setUnitTypeData failed.");
						}

						if (redoObstacles)
						{
							while (!removedObstaclesList.isEmpty())
							{
								Unit *u = (Unit *)removedObstaclesList.popLast();
								UnitActor *ua = getUnitActorForUnit(u);
								ua->addUnitObstacle(u);
							}
						}

					}
					delete [] tmpbuf;
				} else {
					sp->warning("UnitScripting::process - setUnitTypeData parameter missing.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitTypeData for null unit.");
			}
			break;

		case GS_CMD_openUnitHealthBar:
			if( unit != NULL )
			{
				if( game && game->gameUI && game->gameUI->getCombatWindow( 0 ) )
				{
					game->gameUI->getCombatWindow( 0 )->openUnitHealthBar( unit );
				}
				else
				{
					sp->warning("UnitScripting::process - openUnitHealthBar failed to open health bar.");
				}
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to openUnitHealthBar for null unit.");
			}
			break;

			
		case GS_CMD_closeUnitHealthBar:
			if( game && game->gameUI && game->gameUI->getCombatWindow( 0 ) )
			{
				game->gameUI->getCombatWindow( 0 )->closeUnitHealthBar();
			}
			else
			{
				sp->warning("UnitScripting::process - closeUnitHealthBar failed to close health bar.");
			}
			break;

		case GS_CMD_updateUnitSidewaysFlag:
			if(unit != NULL)
			{
				// should call recreateUnitVisual after this.
				unit->setSideways(unit->getUnitType()->isSideways());
			} else {
				sp->warning("UnitScripting::process - Attempt to updateUnitSidewaysFlag for null unit.");
			}
			break;

		case GS_CMD_setUnitSidewaysFlagToValue:
			if(unit != NULL)
			{
				// should call recreateUnitVisual after this.
				if (*lastValue != 0)
					unit->setSideways(true);
				else
					unit->setSideways(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitSidewaysFlagToValue for null unit.");
			}
			break;

		case GS_CMD_updateUnitSideGravity:
			if(unit != NULL)
			{
				// should call recreateUnitVisual after this.
				unit->setSideGravityX(unit->getUnitType()->getSideGravityX());
				unit->setSideGravityZ(unit->getUnitType()->getSideGravityZ());
			} else {
				sp->warning("UnitScripting::process - Attempt to updateUnitSideGravity for null unit.");
			}
			break;

		case GS_CMD_setUnitSideGravityXToValue:
			if(unit != NULL)
			{
				unit->setSideGravityX((float(*lastValue) / 100.0f));
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitSideGravityXToValue for null unit.");
			}
			break;

		case GS_CMD_setUnitSideGravityZToValue:
			if(unit != NULL)
			{
				unit->setSideGravityZ((float(*lastValue) / 100.0f));
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitSideGravityZToValue for null unit.");
			}
			break;

		case GS_CMD_setUnitSideVelocityMaxToValue:
			if(unit != NULL)
			{
				unit->setSideVelocityMax((float(*lastValue) / 100.0f));
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitSideVelocityMaxToValue for null unit.");
			}
			break;

		case GS_CMD_disableUnitPhysicsObjectFeedback:
			if(unit != NULL)
			{
				unit->setPhysicsObjectFeedbackEnabled(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to disableUnitPhysicsObjectFeedback for null unit.");
			}
			break;

		case GS_CMD_enableUnitPhysicsObjectFeedback:
			if(unit != NULL)
			{
				unit->setPhysicsObjectFeedbackEnabled(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to enableUnitPhysicsObjectFeedback for null unit.");
			}
			break;

		case GS_CMD_isUnitPhysicsObjectDifference:
			if (unit != NULL)
			{
				float floatData = intFloat.f;
				if (unit->getPhysicsObjectDifference() >= floatData)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitPhysicsObjectDifference for null unit.");
			}
			break;

		case GS_CMD_lockUnitToPhysicsObject:
			if(unit != NULL)
			{
				unit->setPhysicsObjectLock(true);
			} else {
				sp->warning("UnitScripting::process - Attempt to lockUnitToPhysicsObject for null unit.");
			}
			break;

		case GS_CMD_unlockUnitFromPhysicsObject:
			if(unit != NULL)
			{
				unit->setPhysicsObjectLock(false);
			} else {
				sp->warning("UnitScripting::process - Attempt to unlockUnitFromPhysicsObject for null unit.");
			}
			break;

		case GS_CMD_isUnitLockedToPhysicsObject:
			if(unit != NULL)
			{
				if (unit->isPhysicsObjectLock())
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitLockedToPhysicsObject for null unit.");
			}
			break;

		case GS_CMD_setUnitVisualizationOffset:
			{
				float floatData = intFloat.f;
				Unit::setVisualizationOffset( floatData );
			}
			break;

		case GS_CMD_copyExtPathToOriginalUnit:
			// THE HORRIBLE COPY PASTE HACK!!!
			// COPY-PASTED FROM THE HORRIBLE COPY PAST HACK OF GS_CMD_COPYEXTPATHFROMORIGINALUNIT
			{
			
				Unit* unit1 = gsd->originalUnit;
				Unit* unit2 = unit;
				if (unit1 != NULL)
				{
					if (unit2 != NULL)
					{
						if (unit2 != unit1)
						{
							if (stringData != NULL)
							{
								int foundPath = -1;
								int allocedOrigPaths = unit2->scriptPaths.getAllocatedStoredPaths();
								{
									for (int i = 1; i < allocedOrigPaths; i++)
									{
										if (unit2->scriptPaths.isStoredPathUsed(i)
											&& unit2->scriptPaths.isStoredPathStart(i))
										{
											if (unit2->scriptPaths.getStoredPathName(i) != NULL
												&& strcmp(stringData, unit2->scriptPaths.getStoredPathName(i)) == 0)
											{
												foundPath = i;
												break;
											}
										}
									}
								}
								int addFromPath = 1;
								//bool alreadyHasExt = false;
								if (foundPath != -1)
								{
									int allocedUnitPaths = unit1->scriptPaths.getAllocatedStoredPaths();
									// NOTE: currently cannot add more than 1 ext_path
									// FIXME: erase old ext_path so that additional ones could be added.
									for (int i = 1; i < allocedUnitPaths; i++)
									{
										if (unit1->scriptPaths.isStoredPathUsed(i))
										{
											addFromPath = i + 1;
										}
										if (unit1->scriptPaths.isStoredPathUsed(i)
											&& unit1->scriptPaths.isStoredPathStart(i))
										{
											if (unit1->scriptPaths.getStoredPathName(i) != NULL
												&& strcmp("ext_path", unit1->scriptPaths.getStoredPathName(i)) == 0)
											{
												//alreadyHasExt = true;
												addFromPath = i;
												break;
											}
										}
									}
								}
								//if (foundPath != -1 && !alreadyHasExt)
								if (foundPath != -1)
								{
									int endPathNum = -1;
									{
										// find out where the end piece for this path is...
										for (int i = 1; i < allocedOrigPaths; i++)
										{
											if (unit2->scriptPaths.isStoredPathUsed(i)
												&& unit2->scriptPaths.isStoredPathEnd(i))
											{
												endPathNum = i;
												break;
											}
										}
									}
									if (endPathNum != -1)
									{
										gsd->lastStoredPath = addFromPath;

										// copied: "pathName"
										gsd->setStorePathName("ext_path");

										for (int i = foundPath; i < endPathNum + 1; i++)
										{
											// copy to current unit's path...

											VC3 oldPos = unit2->scriptPaths.getStoredPathStartPosition(i);
											VC3 pos = unit2->scriptPaths.getStoredPathEndPosition(i);

											if (i == foundPath)
											{
												// copied: "pathStart"
												unit1->scriptPaths.setStoredPathStart(0, pos, unit1, gsd->storePathName);
											}
											else if (i == endPathNum)
											{
												// copied: "pathEnd"
												unit1->scriptPaths.setStoredPathEnd(0, pos, unit1);
											} else {
												// copied: "pathTo"
												UnitActor *ua = getUnitActorForUnit(unit1);
												if (ua != NULL)
												{
													frozenbyte::ai::Path *path = ua->solvePath(unit1, oldPos, pos);
													// notice: gsd->position may have been changed by getPath
													// if it was blocked.
													unit1->setStoredPath(0, path, oldPos, pos);
													if (path == NULL)
													{
														sp->debug("UnitScripting::process - copyExtPathFromOriginalUnit failed to find path.");
													}
												}
											}

											// copied: "storeNextPath"
											gsd->lastStoredPath++;
											unit1->scriptPaths.moveStoredPath(0, gsd->lastStoredPath);
										}								
									} else {
										sp->error("UnitScripting::process - copyExtPathFromOriginalUnit failed to find end point for given path (internal error).");
									}
								} else {
									//if (alreadyHasExt)
									//{
									//	sp->warning("UnitScripting::process - copyExtPathFromOriginalUnit cannot be done as unit already has ext_path (todo, support for ext_path recreate).");
									//} else {
										sp->warning("UnitScripting::process - copyExtPathFromOriginalUnit failed to find path with given name.");
									//}
								}
							} else {
								sp->error("UnitScripting::process - copyExtPathFromOriginalUnit parameter missing, path name expected.");
							}
						} else {
							// NOTE: this might still work? copying unit's another path to name "ext_path"..
							sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit when unit is the same as original unit.");
						}
					} else {
						sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit for null original unit.");
					}
				} else {
					sp->warning("UnitScripting::process - Attempt to copyExtPathFromOriginalUnit for null unit.");
				}
			}
			break;

		case GS_CMD_risingMessage:
			{
				if( game && game->gameUI && game->gameUI->getCombatWindow( game->singlePlayerNumber ) && game->gameUI->getCombatWindow( 0 )->getSubWindow( "TargetDisplayWindow" ) )
				{
					ICombatSubWindow* temp = game->gameUI->getCombatWindow( 0 )->getSubWindow( "TargetDisplayWindow" );
					if( temp )
					{
						TargetDisplayWindowUpdator* updator = (TargetDisplayWindowUpdator*)temp;
						updator->risingMessage( unit, stringData, ui::risingMessageStyle );
					}

				}

			}
			break;

		case GS_CMD_activateUnitByIdString:
			if (stringData != NULL)
			{
				Unit *u = game->units->getUnitByIdString(stringData);

				if (u != NULL)
				{
					if (!u->isActive())
					{
						UnitSpawner::spawnUnit(game, u);						
					} else {
						sp->warning("GameScripting::process - activateUnitByIdString, unit is already active.");
					}
				} else {
					sp->warning("GameScripting::process - activateUnitByIdString, No unit found with given id.");
				}
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - activateUnitByIdString, parameter missing (unit id string expected).");
			}
			break;

		case GS_CMD_deactivateUnit:
			if (gsd->unit != NULL)
			{
				if (gsd->unit->isActive())
				{
					for (int fp = 0; fp < MAX_PLAYERS_PER_CLIENT; fp++)
					{
						if (game->gameUI->getFirstPerson(fp) == gsd->unit)
						{
							game->gameUI->setFirstPerson(game->singlePlayerNumber, NULL, fp);
						}
					}

					UnitActor *ua = getUnitActorForUnit(gsd->unit);
					ua->removeUnitObstacle(gsd->unit);

					UnitSpawner::spawner_dont_delete_unit = true;
					UnitSpawner::retireUnit(game, gsd->unit);
					UnitSpawner::spawner_dont_delete_unit = false;
				} else {
					sp->warning("GameScripting::process - deactivateUnit, unit is already inactive.");
				}
			} else {
				sp->warning("GameScripting::process - Attempt to deactivateUnit for null unit.");
			}
			break;

		case GS_CMD_activateUnit:
			if( gsd->unit != NULL )
			{
				Unit *u = gsd->unit;

				if (u != NULL)
				{
					if (!u->isActive())
					{
						UnitSpawner::spawnUnit(game, u);						
					} else {
						sp->warning("GameScripting::process - activateUnit, unit is already active.");
					}
				} else {
					sp->warning("GameScripting::process - activateUnit, No unit to activate.");
				}
			}
			break;

		case GS_CMD_setFirstOwnedUnitOfTypeIncludingInActiveUnits:
			if (stringData == NULL)
			{
				sp->error("UnitScripting::process - setFirstOwnedUnitOfTypeIncludingInActiveUnits parameter missing (unit type expected).");
			} else {
				UnitType *ut = getUnitTypeByName(stringData);
				if (ut == NULL)
				{
					sp->error("UnitScripting::process - setFirstOwnedUnitOfTypeIncludingInActiveUnits, reference to unknown unit type.");
				} else {
					gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, NULL, false );
					while (gsd->unit != NULL)
					{
						if (gsd->unit->getUnitType() == ut)
							break;
						gsd->unit = nextOwnedUnit(game, gsd->position, gsd->player, gsd->unit, false );
					}
					if (gsd->unit != NULL)
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
						sp->error("UnitScripting::process - setFirstOwnedUnitOfTypeIncludingInActiveUnits, could not find unit type.");
					}
				}
			}
			break;

		case GS_CMD_setUnitJumpCounterTime:
			if (unit != NULL)
			{
				int val = intData / GAME_TICK_MSEC;
				unit->setJumpCounter(val);
				sp->warning("UnitScripting::process - setUnitJumpCounterTime, should not be used (inteded to use setUnitJumpNotAllowedTime?).");
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitJumpCounterTime for null unit.");
			}
			break;

		case GS_CMD_setPlayerWeaponry:
			if( unit != NULL )
			{
				std::string temp_string( stringData?stringData:"" );
				std::vector< std::string > splitString = util::StringSplit( ",", temp_string );

				if( splitString.size() >= 2 )
				{
					int ui_number = boost::lexical_cast< int >( splitString[ 0 ] );

					const char* partTypeIdString = splitString[ 1 ].c_str();
					if (PARTTYPE_ID_STRING_VALID(partTypeIdString))
					{
						int weapon_id = PARTTYPE_ID_STRING_TO_INT(partTypeIdString);
						game::PlayerWeaponry::setPlayerWeaponry( unit, ui_number, weapon_id );
					}
					else
					{
						sp->warning("UnitScripting::process - Attempt to setPlayerWeaponry with invalid weapon id.");
					}

				}
				else
				{
					sp->warning("UnitScripting::process - Attempt to setPlayerWeaponry with invalid string parameter.");
				}
				
				/*
				
				if( stringData != NULL )
				{
					
					if (PARTTYPE_ID_STRING_VALID(partTypeIdString))
					{
						int weapon_id = PARTTYPE_ID_STRING_TO_INT(partTypeIdString);
						game::PlayerWeaponry::setPlayerWeaponry( unit, ui_number, weapon_id );
					}
					else
					{
					}
				}
				else
				{
					sp->warning("UnitScripting::process - setPlayerWeaponry requires a string parameter.");
				}*/
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setPlayerWeaponry for null unit.");
			}
			break;

		case GS_CMD_deleteUnitPhysicsObject:
			if (unit != NULL)
			{
				if (unit->getGamePhysicsObject() != NULL)
				{
					if (game->getGamePhysics() != NULL)
					{
						UnitPhysicsUpdater::deletePhysics(unit, game->getGamePhysics());
					}
				} else {
					sp->warning("UnitScripting::process - deleteUnitPhysicsObject, unit does not have a physics object.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to deleteUnitPhysicsObject for null unit.");
			}
			break;

		case GS_CMD_createUnitPhysicsObject:
			if (unit != NULL)
			{
				if (unit->getGamePhysicsObject() == NULL)
				{
					if (game->getGamePhysics() != NULL)
					{
						UnitPhysicsUpdater::createPhysics(unit, game->getGamePhysics(), game);
					}
				} else {
					sp->warning("UnitScripting::process - deleteUnitPhysicsObject, unit already has a physics object.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to createUnitPhysicsObject for null unit.");
			}
			break;

		case GS_CMD_setUnitSide:
			if(unit != NULL)
			{
				if(game->units != NULL)
				{
					game->units->switchUnitSide(unit, intData);
				} else {
					sp->warning("UnitScripting::process - setUnitSide, UnitList missing.");
				}
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setUnitSide for null unit.");
			}
			break;

		case GS_CMD_getUnitType:
			if (unit != NULL)
			{
				gsd->setStringValue(unit->getUnitType()->getName());
			} else {
				sp->warning("GameScripting::process - Attempt to getUnitType for null unit.");
			}
			break;

		case GS_CMD_doesWeaponExistInPlayerWeaponry:
			if( unit != NULL )
			{
				if(stringData != NULL)
				{
					if (PARTTYPE_ID_STRING_VALID(stringData))
					{
						int weapon_id = PARTTYPE_ID_STRING_TO_INT(stringData);
						if(game::PlayerWeaponry::getUINumberByWeaponId( unit, weapon_id ) >= 0)
						{
							*lastValue = 1;
						}
						else
						{
							*lastValue = 0;
						}
					}
					else
					{
						sp->warning("UnitScripting::process - Attempt to doesWeaponExistInPlayerWeaponry with invalid weapon id.");
					}
				}
				else
				{
					sp->warning("UnitScripting::process - Attempt to doesWeaponExistInPlayerWeaponry with invalid string parameter.");
				}
				
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to doesWeaponExistInPlayerWeaponry for null unit.");
			}
			break;

		case GS_CMD_getUnitMaxHP:
			if( unit != NULL )
			{
				*lastValue = unit->getMaxHP();
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to getUnitMaxHP for null unit.");
			}
			break;

		case GS_CMD_setUnitMaxHP:
			if( unit != NULL )
			{
				unit->setMaxHP(*lastValue);
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setUnitMaxHP for null unit.");
			}
			break;

		case GS_CMD_resetUnitMaxHP:
			if( unit != NULL )
			{
				if (unit->getCharacter() != NULL)
				{
					unit->setMaxHP(unit->getUnitType()->getHP() * unit->getCharacter()->getSkillAmount(CHAR_SKILL_STAMINA) / 100);
				} else {
					unit->setMaxHP(unit->getUnitType()->getHP());
				}
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to resetUnitMaxHP for null unit.");
			}
			break;

		case GS_CMD_getHealth:
			if( unit != NULL )
			{
				*lastValue = unit->getHP();
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to getHealth for null unit.");
			}
			break;

		case GS_CMD_setUnitPoisonResistance:
			if( unit != NULL )
			{
				float floatData = intFloat.f;
				unit->setPoisonResistance(floatData);
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setUnitPoisonResistance for null unit.");
			}
			break;

		case GS_CMD_setUnitHPGainMode:
			if( unit != NULL )
			{
				if(strcmp(stringData, "disabled") == 0)
				{
					unit->disableHPGain();
				}
				else
				{
					// parse parameters
					float limit = 0;
					int amount = 0;
					int delay = 0;
					int startdelay = 0;
					float damagefactor = 0;

					char *pos;

					// VS doesn't handle this as const
					pos = strstr((char*)stringData, "limit");
					if(pos == NULL || sscanf(pos, "limit=%f", &limit) != 1)
					{
						sp->error("UnitScripting::process - setUnitHPGainMode expects float parameter 'limit'.");
						break;
					}

					pos = strstr((char*)stringData, "amount");
					if(pos == NULL || sscanf(pos, "amount=%i", &amount) != 1)
					{
						sp->error("UnitScripting::process - setUnitHPGainMode expects integer parameter 'amount'.");
						break;
					}

					pos = strstr((char*)stringData, "delay");
					if(pos == NULL || sscanf(pos, "delay=%i", &delay) != 1)
					{
						sp->error("UnitScripting::process - setUnitHPGainMode expects integer parameter 'delay'.");
						break;
					}

					pos = strstr((char*)stringData, "startdelay");
					if(pos == NULL || sscanf(pos, "startdelay=%i", &startdelay) != 1)
					{
						sp->error("UnitScripting::process - setUnitHPGainMode expects integer parameter 'startdelay'.");
						break;
					}

					pos = strstr((char*)stringData, "damagefactor");
					if(pos == NULL || sscanf(pos, "damagefactor=%f", &damagefactor) != 1)
					{
						sp->error("UnitScripting::process - setUnitHPGainMode expects float parameter 'damagefactor'.");
						break;
					}

					unit->enableHPGain(limit, amount, delay * GAME_TICKS_PER_SECOND, startdelay * GAME_TICKS_PER_SECOND, damagefactor);
				}
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setUnitHPGainMode for null unit.");
			}
			break;

		case GS_CMD_setUnitCriticalHitPercent:
			if( unit != NULL )
			{
				float floatData = intFloat.f;
				unit->setCriticalHitPercent(floatData);
			}
			else
			{
				sp->warning("UnitScripting::process - Attempt to setUnitCriticalHitPercent for null unit.");
			}
			break;

		case GS_CMD_makeUnitSkyModel:
			if(unit != NULL)
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->makeSkyModel();

					//game->gameUI->addSkyModelUnit(unit);
				} else {
					sp->warning("UnitScripting::process - Attempt to makeUnitSkyModel for non-spawned unit.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to makeUnitSkyModel for null unit.");
			}
			break;

		case GS_CMD_warpUnitToAbsolutePosition:
			if (unit != NULL)
			{
				UnitActor *ua = getUnitActorForUnit(unit);

				if (ua != NULL)
				{
					ua->removeUnitObstacle(unit);

					unit->setPosition(gsd->position);

					ua->addUnitObstacle(unit);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to warpUnitToAbsolutePosition for null unit.");
			}
			break;

		case GS_CMD_setCustomWeaponTypeDataForType:
			if (stringData != NULL)
			{
				char *tmpbuf = (char *)alloca(strlen(stringData) + 1);
				strcpy(tmpbuf, stringData);
				PartType *pt = NULL;

				// find parttype before : separator
				{
					char *sep = strstr(tmpbuf, ":");
					if (sep == NULL)
					{
						sp->error("UnitScripting::process - setCustomWeaponTypeDataForType parameter bad (\"parttype:datakey=value\" expected).");
						break;
					}
					*sep = 0;
					char *partType = tmpbuf;
					if (!PARTTYPE_ID_STRING_VALID(partType))
					{
						sp->error("UnitScripting::process - setCustomWeaponTypeDataForType invalid part type.");
						break;
					}
					pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(partType));
					tmpbuf = sep + 1;
				}

				Weapon *customized_weapon = unit->getCustomizedWeaponType((Weapon*)pt);
				if(customized_weapon == NULL)
				{
					customized_weapon = unit->createCustomizedWeaponType((Weapon*)pt);
				}
				pt = customized_weapon;

				// find key=value
				{
					char *sep = strstr(tmpbuf, "=");
					if (sep == NULL)
					{
						sp->error("UnitScripting::process - setCustomWeaponTypeDataForType parameter bad (\"parttype:datakey=value\" expected).");
						break;
					}
					*sep = 0;
					bool success = pt->setData(tmpbuf, &sep[1]);
					if (!success)
					{
						sp->error("UnitScripting::process - setCustomWeaponTypeDataForType failed.");
					}
				}
			} else {
				sp->error("UnitScripting::process - setCustomWeaponTypeDataForType parameter missing.");
			}
			break;

		case GS_CMD_changeUnitRootPartModel:
			if (unit != NULL)
			{
				if (unit->getRootPart())
				{
					Part *part = unit->getRootPart();
					const char *partName = "Torso";

					if (unit->getVisualObject() == NULL)
					{
						sp->error("UnitScripting::process - Attempt to changeUnitRootPartModel for unit without visual object.");
						break;
					}

					unit->getVisualObject()->removeObject(partName);

					ui::VisualObject *oldv = part->getVisualObject();
					part->setVisualObject(NULL);
					if (oldv != NULL) delete oldv;

					VisualObjectModel *vom = game->visualObjectModelStorage->getVisualObjectModel(stringData);
					if(vom == NULL)
					{
						sp->error("UnitScripting::process - Attempt to changeUnitRootPartModel with invalid model name.");
						break;
					}
					VisualObject *vo = vom->getNewObjectInstance();
					if(vo == NULL)
					{
						sp->error("UnitScripting::process - Attempt to changeUnitRootPartModel with invalid model name.");
						break;
					}
					part->setVisualObject(vo);
					// combine part model to unit model
					char *helperName = NULL;
					unit->getVisualObject()->combine(part->getVisualObject(), partName, helperName);
					unit->setTorsoModelOverride(stringData);
				}
				else
				{
					sp->error("UnitScripting::process - Attempt to changeUnitRootPartModel for unit without root part.");
				}

			} else {
				sp->warning("UnitScripting::process - Attempt to changeUnitRootPartModel for null unit.");
			}
			break;

		case GS_CMD_setUnitTimeFactor:
			{
				if(unit != NULL)
				{
					float floatData = intFloat.f;
					unit->setCustomTimeFactor(floatData);
				}
				else
				{
					sp->warning("UnitScripting::process - Attempt to setUnitTimeFactor for null unit.");
				}
			}
			break;


		case GS_CMD_setCustomWeaponTypeData:
			if (gsd->part != NULL && unit != NULL)
			{
				if (stringData != NULL)
				{
					int w = unit->getWeaponByWeaponType(gsd->part->getType()->getPartTypeId());
					if(w == -1)
					{
						sp->error("UnitScripting::process - setCustomWeaponTypeData part is not a weapon of unit.");
						break;
					}
					if(unit->getCustomizedWeaponType((Weapon*)gsd->part->getType()) == NULL)
					{
						Weapon *w_old = (Weapon*)gsd->part->getType();
						Weapon *w_new = unit->createCustomizedWeaponType(w_old);

						// move attachment links to new weapon
						for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
						{
							Weapon *w = unit->getWeaponType(i);
							if(w != NULL && w->getAttachedWeaponType() == w_old)
							{
								// customize it first...
								if(unit->getCustomizedWeaponType(w) == NULL)
								{
									w = unit->createCustomizedWeaponType(w);
								}
								w->setAttachedWeaponType(w_new);
							}
						}
					}
					Weapon *pt = unit->getWeaponType(w);

					char *tmpbuf = new char[strlen(stringData) + 1];
					strcpy(tmpbuf, stringData);
					char *sep = strstr(tmpbuf, "=");
					if (sep != NULL)
					{
						*sep = '\0';
						bool success = pt->setData(tmpbuf, &sep[1]);
						if (!success)
						{
							sp->error("UnitScripting::process - setCustomWeaponTypeData failed.");
						}
					} else {
						sp->error("UnitScripting::process - setCustomWeaponTypeData parameter bad (part type data key=value pair expected).");
					}
					delete [] tmpbuf;
				} else {
					sp->error("UnitScripting::process - setCustomWeaponTypeData parameter missing.");
				}
			} else {
				sp->error("UnitScripting::process - Attempt to setCustomWeaponTypeData for null unit/part.");
			}
			break;

		case GS_CMD_setUnitWeaponInvisible:
		case GS_CMD_setUnitWeaponVisible:
			if (unit != NULL)
			{
				if (!PARTTYPE_ID_STRING_VALID(stringData))
				{
					if (stringData == NULL)
						sp->error("UnitScripting::process - setUnitWeaponInvisible/Visible, expected part type id.");
					else
						sp->error("UnitScripting::process - setUnitWeaponInvisible/Visible, illegal part type id.");
				} else {
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
					if (pt == NULL) 
					{ 
						sp->error("UnitScripting::process - setUnitWeaponInvisible/Visible, reference to unloaded part type.");
					} else {
						assert(unit->isActive());
						int newweap = unit->getWeaponByWeaponType(pt->getPartTypeId());
						if (newweap != -1)
						{
							if (command == GS_CMD_setUnitWeaponVisible)
							{
								unit->setWeaponVisible(newweap, true);
							} else {
								unit->setWeaponVisible(newweap, false);
							}
						}
					}
				}
			}
			else
			{
				sp->error("UnitScripting::process - setUnitWeaponInvisible/Visible for null unit.");
			}
			break;


		case GS_CMD_unitTimeSinceLastDamaged:
			if(unit != NULL)
			{
				(*lastValue) = (game->gameTimer - unit->getLastTimeDamaged()) / GAME_TICKS_PER_SECOND;
			}
			else
			{
				sp->error("UnitScripting::process - unitTimeSinceLastDamaged for null unit.");
			}
			break;

		case GS_CMD_restoreDefaultSpeed:
			if (unit != NULL)
			{
				// HACK: this needs to be done using the setSpeed script command to handle all the jump, etc.
				// stuff correctly (or these two commands would actually need to be properly refactored)
				const char *defSpeedStr = NULL;
				if (unit->getUnitType()->getDefaultSpeed() == UnitType::DEFAULT_SPEED_FAST)
					defSpeedStr = "fast";
				else if (unit->getUnitType()->getDefaultSpeed() == UnitType::DEFAULT_SPEED_SLOW)
					defSpeedStr = "slow";
				else if (unit->getUnitType()->getDefaultSpeed() == UnitType::DEFAULT_SPEED_CRAWL)
					defSpeedStr = "crawl";
				else if (unit->getUnitType()->getDefaultSpeed() == UnitType::DEFAULT_SPEED_SPRINT)
					defSpeedStr = "sprint";

				if (defSpeedStr != NULL)
					UnitScripting::process(sp, GS_CMD_SETSPEED, intFloat, defSpeedStr, lastValue, gsd, game, pause);
				else
					sp->warning("UnitScripting::process - restoreDefaultSpeed, invalid default speed type (internal error).");
			} else {
				sp->warning("UnitScripting::process - Attempt to restoreDefaultSpeed for null unit.");
			}
			break;

		case GS_CMD_setUnitStunned:
			if (unit != NULL)
			{
				int val = (intData * GAME_TICKS_PER_SECOND) / 1000;
				if(val > 0)
				{
					unit->setMoveState(Unit::UNIT_MOVE_STATE_STUNNED);
					unit->setMoveStateCounter(val);
				}
				else
				{
					unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
					unit->setMoveStateCounter(0);
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitStunned for null unit.");
			}
			break;

		case GS_CMD_getUnitOwner:
			if (unit != NULL)
			{
				(*lastValue) = unit->getOwner();
			} else {
				sp->warning("UnitScripting::process - Attempt to getUnitOwner for null unit.");
			}
			break;

		case GS_CMD_disableUnitAIWithHitScript:
			if (unit != NULL)
			{
				// WARNING: unsafe cast!
				UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
				ai->setEnabled(false, true);
			} else {
				sp->warning("UnitScripting::process - disableUnitAIWithHitScript for null unit.");
			}
			break;

		case GS_CMD_setUnitCulling:
			if(unit != NULL)
			{
				IStorm3D_Model *model = unit->getVisualObject()->getStormModel();
				if(model)
				{
					bool useCulling = true;
					if(!intData)
						useCulling = false;

					game->gameUI->getStormScene()->EnableCulling(model, useCulling);
				}
			}
			break;

		case GS_CMD_setUnitHealthBarFlashingToValue:
			if( game && game->gameUI && game->gameUI->getCombatWindow( 0 ) )
			{
				game->gameUI->getCombatWindow( 0 )->setUnitHealthBarFlashing(*lastValue);
			}
			else
			{
				sp->warning("UnitScripting::process - setUnitHealthBarFlashingToValue failed to find health bar.");
			}
			break;

		case GS_CMD_getAngleToPosition:
			if (unit != NULL)
			{
				float posangle = unit->getAngleTo(gsd->position);
				*lastValue = (int)(posangle);
			}
			break;

		case GS_CMD_setUnitNoClip:
			if (unit != NULL)
			{
				if(intData == 1)
				{
					unit->setCollisionCheck(false);
					unit->setCollisionBlocksOthers(false);
					UnitActor *ua = getUnitActorForUnit(unit);
					ua->removeUnitObstacle(unit);
					if(unit->getGamePhysicsObject())
					{
						UnitPhysicsUpdater::deletePhysics(unit, game->getGamePhysics());
					}
				}
				else if(intData == 0)
				{
					unit->setCollisionCheck(true);
					unit->setCollisionBlocksOthers(true);
					UnitActor *ua = getUnitActorForUnit(unit);
					ua->addUnitObstacle(unit);
					if(unit->getGamePhysicsObject() == NULL && SimpleOptions::getBool(DH_OPT_B_PHYSICS_ENABLED))
					{
						UnitPhysicsUpdater::createPhysics(unit, game->getGamePhysics(), game);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitNoClip for null unit.");
			}
			break;

		case GS_CMD_setUnitNoHit:
			if (unit != NULL)
			{
				if(intData == 1)
				{
					if(unit->getVisualObject())
					{
						unit->getVisualObject()->setForcedNoCollision(true);
					}
				}
				else if(intData == 0)
				{
					if(unit->getVisualObject())
					{
						unit->getVisualObject()->setForcedNoCollision(false);
					}
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to setUnitNoHit for null unit.");
			}
			break;

		case GS_CMD_isUnitHitByPoison:
			if (unit != NULL)
			{
				if (unit != gsd->originalUnit)
				{
					sp->error("UnitScripting::process - isUnitHitByPoison can only be called inside hit script for original hit script unit.");
				}
				if (gs_hitscript_hit_bullet_type != NULL)
				{
					if (gs_hitscript_hit_bullet_type->doesPoisonDamage())
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->warning("UnitScripting::process - Attempt to isUnitHitByPoison when null hit bullet type info.");
				}
			} else {
				sp->warning("UnitScripting::process - Attempt to isUnitHitByPoison for null unit.");
			} 								
			break;

		case GS_CMD_setUnitShielded:
			if (unit != NULL)
			{
				unit->setShielded(intData == 0 ? false : true);
			} else {
				sp->error("UnitScripting::process - Attempt to setUnitShielded for null unit.");
			} 								
			break;

		case GS_CMD_reloadUnitWeapons:
			if (unit != NULL)
			{
				for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
				{
					unit->reloadWeaponAmmoClip(w, true);
				}
			} else {
				sp->error("UnitScripting::process - Attempt to reloadUnitWeapons for null unit.");
			}
			break;

		default:
			sp->error("UnitScripting::process - Unknown command.");
			assert(!"UnitScripting::process - Unknown command.");
		}
	}


	void UnitScripting::moveUnitToDirection(Unit *unit, int direction, 
		float amount)
	{
		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();

		float angle = rotation.y;

		if (direction == MOVE_DIR_LEFT)
			angle += 270;
		else if (direction == MOVE_DIR_RIGHT) 		
			angle += 90;
		else if (direction == MOVE_DIR_BACKWARD)			
			angle += 180;
		while (angle < 0) angle += 360;
		while (angle >= 360) angle -= 360;

		position.x += -amount * sinf(UNIT_ANGLE_TO_RAD(angle));
		position.z += -amount * cosf(UNIT_ANGLE_TO_RAD(angle));

		unit->setPosition(position);
		unit->setWaypoint(position);
		unit->setFinalDestination(position);
	}


	bool UnitScripting::findGroup(Game *game, Unit *unit)
	{
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed() 
				&& u->getOwner() == unit->getOwner())
			{
				// NOTICE: not a good comparision here...
				//if (unit->getUnitType()->isFlying() == u->getUnitType()->isFlying()
				//	&& unit->getUnitType()->isStationary() == u->getUnitType()->isStationary())
				if (unit->getUnitType() == u->getUnitType())
				{
					VC3 pos1 = unit->getPosition();
					VC3 pos2 = u->getPosition();
					float distsq = (pos1 - pos2).GetSquareLength();
					if (distsq < GROUP_FIND_DIST * GROUP_FIND_DIST)
					{
						if (u->getLeader() != NULL)
						{
							if (u->getLeader() == unit)
								unit->setLeader(NULL);
							else
								unit->setLeader(u->getLeader());
						} else {
							unit->setLeader(u);
						}
						return true;
					}
				}
			}
		}
		return false;
	}


	Unit *UnitScripting::nextOwnedUnit(Game *game, const VC3 &position, int player, Unit *fromUnit, bool only_active )
	{
		bool passedFrom = false;
		Unit *first = NULL;
		// notice: getAllUnits if friendly, not owned.
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u == fromUnit)
			{
				passedFrom = true;
			} else {
				if ( ( !only_active || ( u->isActive()
					&& !u->isDestroyed() ) ) )
					// && u->getOwner() == player)
				{
					if (first == NULL)
					{
						first = u;
					}
					if (passedFrom)
					{
						return u;
					}
				}
			}
		}
		if (fromUnit == NULL)
			return first;
		else
			return NULL;
	}


	Unit *UnitScripting::randomOwnedUnit(Game *game, VC3 &position, int player)
	{
		int amount = game->units->getOwnedUnitAmount(player);
		if (amount <= 0) return NULL;
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		int randNum = game->gameRandom->nextInt() % amount;
		int num = 0;
		Unit *beforeUnit = NULL;
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
//				&& u->getOwner() == player)
			{
				if (num >= randNum) 
				{
					return u;
				} else {
					beforeUnit = u;
				}
			}
			num++;
		}
		return beforeUnit;
	}


	Unit *UnitScripting::findClosestHostileUnit(Game *game, VC3 &position, int player, Unit *ignore)
	{
		Unit *closest = NULL;
		float closestRangeSq = 0;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed() 
				&& game->isHostile(player, u->getOwner()))
			{
				VC3 pos2 = u->getPosition();
				float distsq = (position - pos2).GetSquareLength();
				if ((distsq < closestRangeSq || closest == NULL) 
					&& u != ignore)
				{
					closest = u;
					closestRangeSq = distsq;
				}
			}
		}
		return closest;
	}


	Unit *UnitScripting::findClosestFriendlyUnit(Game *game, VC3 &position, int player, Unit *ignore)
	{
		Unit *closest = NULL;
		float closestRangeSq = 0;
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
				if ((distsq < closestRangeSq || closest == NULL) 
					&& u != ignore)
				{
					closest = u;
					closestRangeSq = distsq;
				}
			}
		}
		return closest;
	}


	Unit *UnitScripting::findClosestOwnedUnit(Game *game, VC3 &position, int player, Unit *ignore)
	{
		Unit *closest = NULL;
		float closestRangeSq = 0;
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
				if ((distsq < closestRangeSq || closest == NULL) 
					&& u != ignore)
				{
					closest = u;
					closestRangeSq = distsq;
				}
			}
		}
		return closest;
	}


	Unit *UnitScripting::findUnitByCharacterName(Game *game, const char *charname)
	{
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && u->getCharacter() != NULL)
			{
				if (u->getCharacter()->getName() == NULL)
				{
					assert(0);
					return NULL;
				}
				if (strcmp(charname, u->getCharacter()->getName()) == 0)
				{
					return u;
				}
			}
		}
		return NULL;
	}


	Unit *UnitScripting::findClosestUnitOfType(Game *game, const VC3 &position, const char *unittype)
	{
		float closestDistSq = 999999.0f;
		Unit *closest = NULL;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() 
				&& !u->isDestroyed()
				&& u->getUnitType()->getName() != NULL)
			{
				if (strcmp(u->getUnitType()->getName(), unittype) == 0)
				{
					VC3 distVec = u->getPosition() - position;
					float distSq = distVec.GetSquareLength();
					if (distSq < closestDistSq)
					{
						closestDistSq = distSq;
						closest = u;
					}
				}
			}
		}
		return closest;
	}

	Unit *UnitScripting::findClosestUnitWithVariableSet(Game *game, const VC3 &position, const char *unitvar, bool varIsNonZero)
	{
		float closestDistSq = 999999.0f;
		Unit *closest = NULL;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);

		int varid = UnitVariables::getVariableNumberByName(unitvar);

		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() 
				&& !u->isDestroyed())
			{
				int varVal = u->variables.getVariable(varid);
				if ((varIsNonZero && varVal != 0)
					|| (!varIsNonZero && varVal == 0))
				{
					VC3 distVec = u->getPosition() - position;
					float distSq = distVec.GetSquareLength();
					if (distSq < closestDistSq)
					{
						closestDistSq = distSq;
						closest = u;
					}
				}
			}
		}
		return closest;
	}

	Unit *UnitScripting::findClosestUnitOfTypeWithVariableSet(Game *game, const VC3 &position, const char *unittype, const char *unitvar, bool varIsNonZero)
	{
		float closestDistSq = 999999.0f;
		Unit *closest = NULL;
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);

		int varid = UnitVariables::getVariableNumberByName(unitvar);

		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive()
				&& !u->isDestroyed()
				&& u->getUnitType()->getName() != NULL)
			{
				if (strcmp(u->getUnitType()->getName(), unittype) == 0)
				{
					int varVal = u->variables.getVariable(varid);
					if ((varIsNonZero && varVal != 0)
						|| (!varIsNonZero && varVal == 0))
					{
						VC3 distVec = u->getPosition() - position;
						float distSq = distVec.GetSquareLength();
						if (distSq < closestDistSq)
						{
							closestDistSq = distSq;
							closest = u;
						}
					}
				}
			}
		}
		return closest;
	}

}


