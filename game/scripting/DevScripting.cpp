
#include "precompiled.h"

#include "DevScripting.h"

#include "scripting_macros_start.h"
#include "dev_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>
#include <IStorm3D.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../UnitSelections.h"
#include "../UnitLevelAI.h"
#include "../GameScene.h"
#include "../GameRandom.h"
#include "../Character.h"
#include "../SimpleOptions.h"
#include "../UnitPhysicsUpdater.h"
#include "../options/options_debug.h"
#include "../options/options_cheats.h"
#include "../ui/VisualEffectManager.h"
#include "../../util/assert.h"
#include "../ParticleSpawnerManager.h"
#include "../ui/SelectionVisualizer.h"
#include "GameScripting.h"

#ifndef PROJECT_SHADOWGROUNDS
#include "../ui/MissionSelectionWindow.h"
#endif

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"
#ifdef PHYSICS_ODE
#include "../../physics_ode/physics_ode.h"
#endif
#ifdef PHYSICS_PHYSX
#include "../game/physics/GamePhysics.h"
#include "../../physics/physics_lib.h"
#endif
#include "../game/physics/PhysicsContactDamageManager.h"

#include "../../util/Debug_MemoryManager.h"

#include "../../ui/MenuCollection.h"
#include "igios.h"

using namespace ui;

#ifdef PROJECT_AOV
// (other projects don't have this variable)
extern bool app_soft_restart_requested;
extern bool app_hard_restart_requested;
#endif

namespace ui
{
	extern int visual_effect_allocations;
}

namespace game
{
  extern PhysicsContactDamageManager *gameui_physicsDamageManager;

	void DevScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_ERROR:
			//sp->error("DevScripting::process - Script error:");
			Logger::getInstance()->error("DevScripting::process - Script error:");
			if (stringData != NULL)
			{
				sp->error(stringData);
			} else {
				sp->error("DevScripting::process - Script error (error command has null parameter).");
			}
			break;

		case GS_CMD_RELOADSCRIPTFILE:
			if (stringData != NULL)
			{
				util::ScriptManager::getInstance()->loadScripts(stringData, NULL, true);
			} else {
				sp->error("GameScripting::process - reloadScriptFile parameter missing.");
			}
			break;

		case GS_CMD_reloadParticleEffects:
			if (game->inCombat
				&& game->gameUI->getVisualEffectManager() != NULL)
			{
				game->gameUI->getVisualEffectManager()->loadParticleEffects();
				Logger::getInstance()->info("GameScripting::process - reloadParticleEffects, particle effects reloaded (existing effects may not be affected).");
			} else {
				sp->warning("GameScripting::process - reloadParticleEffects called while in menus or no visual effect manager loaded (call ignored).");
			}
			break;

		case GS_CMD_RELOADSTRINGVALUESCRIPTFILE:
			if (gsd->stringValue != NULL)
			{
				util::ScriptManager::getInstance()->loadScripts(gsd->stringValue, NULL, true);
			} else {
				sp->error("GameScripting::process - reloadStringValueScriptFile for null string.");
			}
			break;

		case GS_CMD_SCRIPTDUMP:
			{
				Logger::getInstance()->debug("Script dump command encountered, output follows...");
				char *dump = sp->getScript()->getDebugDump(sp, false);
				if (dump != NULL)
				{
					Logger::getInstance()->debug(dump);
					delete [] dump;
				} else {
					Logger::getInstance()->debug("Null output."); 				
					assert(0);
				}
			}
			break;

		case GS_CMD_FULLSCRIPTDUMP:
			{
				Logger::getInstance()->debug("Full script dump command encountered, output follows...");
				char *dump = sp->getScript()->getDebugDump(sp, true);
				if (dump != NULL)
				{
					Logger::getInstance()->debug(dump);
					delete [] dump;
				} else {
					Logger::getInstance()->debug("Null output."); 				
					assert(0);
				}
			}
			break;


		case GS_CMD_DEVUNITSCRIPTDUMP:
			{
				Logger::getInstance()->debug("Dev unit script dump command encountered, output follows...");
					// WARNING: unsafe cast!
				util::ScriptProcess *mainsp = ((game::UnitLevelAI *)game->devUnit->getAI())->mainScriptProcess;
				char *dump = mainsp->getScript()->getDebugDump(mainsp, false);
				if (dump != NULL)
				{
					Logger::getInstance()->debug(dump);
					delete [] dump;
				} else {
					Logger::getInstance()->debug("Null output."); 				
					assert(0);
				}
			}
			break;

		case GS_CMD_DEVUNITFULLSCRIPTDUMP:
			{
				if (game->devUnit != NULL)
				{
					Logger::getInstance()->debug("Dev unit full script dump command encountered, output follows...");
					// WARNING: unsafe cast!
					util::ScriptProcess *mainsp = ((game::UnitLevelAI *)game->devUnit->getAI())->mainScriptProcess;
					char *dump = mainsp->getScript()->getDebugDump(mainsp, true);
					if (dump != NULL)
					{
						Logger::getInstance()->debug(dump);
						delete [] dump;
					} else {
						Logger::getInstance()->debug("Null output."); 				
						assert(0);
					}
				}
			}
			break;

		case GS_CMD_DEVMESSAGE:
			if (stringData != NULL)
			{
				if (SimpleOptions::getBool(DH_OPT_B_SCRIPT_DEV_MODE))
				{
					if (game->devUnit == NULL 
						|| gsd->unit == game->devUnit)
					{
						sp->warning(stringData);
					}
				}
			}
			break;

		case GS_CMD_HIDECONSOLE:
			game->gameUI->hideConsole();
			break;

		case GS_CMD_SHOWCONSOLE:
			game->gameUI->showConsole();
			break;

		case GS_CMD_DEVSIDESWAP:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				if (SimpleOptions::getBool(DH_OPT_B_ALLOW_SIDE_SWAP)
					&& game->inCombat)
				{
					sp->debug("DevScripting::process - Swapping sides.");

					game->unitSelections[game->singlePlayerNumber]->selectAllUnits(false);
					
					game->gameUI->closeCombatWindow(game->singlePlayerNumber);
					game->singlePlayerNumber = intData;
					game->gameUI->openCombatWindow(game->singlePlayerNumber);
				}
			} else {
				sp->warning("DevScripting::process - devSideSwap parameter bad.");
			}
			break;

		case GS_CMD_DEVSIDESWAPTOVALUE:
			if (*lastValue >= 0 && *lastValue < ABS_MAX_PLAYERS)
			{
				if (SimpleOptions::getBool(DH_OPT_B_ALLOW_SIDE_SWAP)
					&& game->inCombat)
				{
					sp->debug("DevScripting::process - Swapping sides.");
					
					game->gameUI->closeCombatWindow(game->singlePlayerNumber);
					game->singlePlayerNumber = *lastValue;
					game->gameUI->openCombatWindow(game->singlePlayerNumber);
				}
			} else {
				sp->warning("DevScripting::process - devSideSwapToValue last value out of ranged.");
			}
			break;

		case GS_CMD_DEVUNIT:
			if (gsd->unit == NULL)
			{
				sp->warning("DevScripting::process - devUnit for null unit.");
				game->devUnit = NULL;
			} else {
				game->devUnit = gsd->unit;	
			}
			break;

		case GS_CMD_CLEARDEVUNIT:
			game->devUnit = NULL;
			break;

		case GS_CMD_DUMPSTATUSINFO:
			if (game->getGameScene() != NULL && game->inCombat)
			{
				char *stats = game->getGameScene()->getStorm3D()->GetPrintableStatusInfo();
				Logger::getInstance()->info(stats);
				game->getGameScene()->getStorm3D()->DeletePrintableStatusInfo(stats);
				
				Logger::getInstance()->info("Visual effects running:");
				Logger::getInstance()->info(int2str(ui::visual_effect_allocations));
			} else {
				Logger::getInstance()->warning("DevScripting::process - dumpStatusInfo cannot get info for null scene.");
			}
			break;

		case GS_CMD_dumpEffectsInfo:
			if (game->getGameScene() != NULL && game->inCombat)
			{
				Logger::getInstance()->info("Visual effects running:");
				Logger::getInstance()->info(int2str(ui::visual_effect_allocations));
				Logger::getInstance()->info("Particle spawners total amount:");
				Logger::getInstance()->info(int2str(game->particleSpawnerManager->getParticleSpawnerAmount()));
			} else {
				Logger::getInstance()->warning("DevScripting::process - dumpEffectsInfo cannot get info for null scene.");
			}
			break;

		case GS_CMD_dumpGameSceneGraph:
			{
				char *stats = game->getGameSceneGraphDump();
				Logger::getInstance()->info(stats);
				game->deleteGameSceneGraphDump(stats);
			}
			break;

		case GS_CMD_dumpPhysicsInfo:
			{
				char *stats = UnitPhysicsUpdater::getStatusInfo();
				Logger::getInstance()->info(stats);
				UnitPhysicsUpdater::deleteStatusInfo(stats);
#ifdef PHYSICS_ODE
				stats = PhysicsActorOde::getStatusInfo();
				Logger::getInstance()->info(stats);
				PhysicsActorOde::deleteStatusInfo(stats);
#endif
#ifdef PHYSICS_PHYSX
				if(game->getGamePhysics() && game->getGamePhysics()->getPhysicsLib())
				{
					std::string info = game->getGamePhysics()->getPhysicsLib()->getStatistics();
					Logger::getInstance()->info(info.c_str());
				}
#endif
			}
			break;

		case GS_CMD_RAYTRACEBLAST:
			if (game->getGameScene() != NULL && game->inCombat)
			{
				if (game->gameUI->getFirstPerson(0) != 0)
				{
					if (game->gameUI->getFirstPerson(0)->getVisualObject() != NULL)
					{
						game->gameUI->getFirstPerson(0)->getVisualObject()->setCollidable(false);
					}
					VC3 pos = game->gameUI->getFirstPerson(0)->getPosition();
					pos.y += 1.0f;
					for (float b = -2.0f; b < 1.0f; b += 0.5f)
					{
						for (float a = 0; a < 2*3.1415f; a += 0.25f)
						{
							VC3 dir = VC3(cosf(a), sinf(b / 2.0f), sinf(a));
							dir.Normalize();
							GameCollisionInfo cinfo;
							game->getGameScene()->rayTrace(pos, dir, 20, cinfo, true, false);
						}
					}
					if (game->gameUI->getFirstPerson(0)->getVisualObject() != NULL)
					{
						game->gameUI->getFirstPerson(0)->getVisualObject()->setCollidable(true);
					}
				}
			}
			break;

		case GS_CMD_DEVEXIT:
			sp->error("DevScripting::process - devExit called, making an immediate and unclean exit.");
			exit(0);
			break;

		case GS_CMD_DEVASSERT:
			if (*lastValue == 0)
			{
				sp->warning("DevScripting::process - devAssert.");
				igios_unimplemented();
				// FIXME: this is disabled
				// need to fix in scripts?
				//FB_ASSERT(!"DevScripting::process - devAssert.");
			}
			break;

		case GS_CMD_DEVCRASH:
			{
#ifndef FB_TESTBUILD
				sp->warning("DevScripting::process - devCrash ignored.");
#else
				sp->warning("DevScripting::process - devCrash about to crash.");
				FB_ASSERT(!"DevScripting::process - devCrash.");
				int *crashPtr = NULL;
				int crashValue = *crashPtr;
				*lastValue = crashValue;
#endif
			}
			break;

		case GS_CMD_LISTGLOBALVARIABLES:
			{
				LinkedList *tmp = util::Script::getGlobalVariableList(false);

				while (!tmp->isEmpty())
				{
					const char *varname = (const char *)tmp->popLast();
					int varval = 0;
					util::Script::getGlobalIntVariableValue(varname, &varval);

					char *tmpbuf = new char[strlen(varname) + 32];
					strcpy(tmpbuf, varname);
					strcat(tmpbuf, " (");
					strcat(tmpbuf, int2str(varval));
					strcat(tmpbuf, ")");
					Logger::getInstance()->info(tmpbuf);
				}

				delete tmp;
			}
			break;

		case GS_CMD_reloadObjectDurabilities:
			if (gameui_physicsDamageManager != NULL)
			{
				gameui_physicsDamageManager->reloadConfiguration();
			}
			break;

		case GS_CMD_openScoreWindow:
			{
				if( game && game->gameUI )
				{
					game->gameUI->openScoreWindow( game->singlePlayerNumber );
				}
			}
			break;      
      
		case GS_CMD_openMissionSelectionWindow:
			{
				if( game && game->gameUI )
				{
					game->gameUI->openMissionSelectionWindow();
				}
			}
			break;

		case GS_CMD_dumpScriptInfo:
			Logger::getInstance()->info(util::ScriptManager::getInstance()->getStatusInfo().c_str());
			Logger::getInstance()->info("Custom script processes amount:");
			Logger::getInstance()->info(int2str(game->getCustomScriptProcessAmount()));
			break;

		case GS_CMD_devStopAllCustomScripts:
			game->stopAllCustomScriptProcesses();
			break;


#ifndef PROJECT_SHADOWGROUNDS
		case GS_CMD_missionSelectionWindowAddMissionButton:
			{
				if( game && game->gameUI && game->gameUI->getMissionSelectionWindow() )
				{
					game->gameUI->getMissionSelectionWindow()->AddMissionButton( stringData );
				}
			}
			break;
#endif

		case GS_CMD_openMainMenuWindow:
			{
				if( game && game->gameUI )
				{
					game->gameUI->openMainmenuFromGame( intData );
					// game->gameUI->getCommandWindow( game->singlePlayerNumber )->openMenu( intData );
				}
			}
			break;

		case GS_CMD_devRunSingleCommand:
			if (stringData != NULL)
			{				
				int slen = strlen(stringData);
				char *cmdName = new char[slen + 1];
				char *param = new char[slen + 1];
				int cmdNamePos = 0;
				int paramPos = 0;
				cmdName[0] = '\0';
				param[0] = '\0';
				bool trimming = true;
				bool readingCmd = false;
				bool skippingSep = false;
				bool readingParam = false;
				for (int i = 0; i < slen; i++)
				{
					if (trimming)
					{
						if (stringData[i] != ' ' && stringData[i] != '\t')
						{
							trimming = false;
							readingCmd = true;
						}
					}
					if (readingCmd)
					{
						if (stringData[i] == ' ' || stringData[i] == '\t')
						{
							readingCmd  = false;
							skippingSep = true;
						} else {
							cmdName[cmdNamePos] = stringData[i];
							cmdNamePos++;
						}
					}
					if (skippingSep)
					{
						if (stringData[i] != ' ' && stringData[i] != '\t')
						{
							skippingSep = false;
							readingParam = true;
						}
					}
					if (readingParam)
					{
						param[paramPos] = stringData[i];
						paramPos++;
					}
				}
				cmdName[cmdNamePos] = '\0';
				param[paramPos] = '\0';
				if (cmdName[0] == '\0')
				{
					sp->error("DevScripting::process - devRunSingleCommand parameter invalid.");
				} else {
					int tmpInt = *lastValue;
					int tmpInt2 = sp->getSecondaryValue();
					bool success;
					if (!readingParam)
					{
						success = game->gameScripting->runSingleSimpleStringCommand(cmdName, NULL, &tmpInt, &tmpInt2); 
					} else {
						success = game->gameScripting->runSingleSimpleStringCommand(cmdName, param, &tmpInt, &tmpInt2); 
					}
					if (!success)
					{
						sp->error("DevScripting::process - devRunSingleCommand failed.");
					}
					*lastValue = tmpInt;
					sp->setSecondaryValue(tmpInt2);
				}
				delete [] param;
				delete [] cmdName;
			} else {
				sp->error("DevScripting::process - devRunSingleCommand parameter missing.");
			}
			break;

		case GS_CMD_forceCursorVisibility:
			{
				game->gameUI->forceCursorVisibility(intData == 0 ? false : true);
			}
			break;

		case GS_CMD_dumpMemoryInfo:
			{
#ifdef FROZENBYTE_DEBUG_MEMORY
				frozenbyte::debug::dumpLeakSnapshot();
				frozenbyte::debug::markLeakSnapshot();					
#endif
			}
			break;

		case GS_CMD_devPhysicsConnectToRemoteDebugger:
#if (!defined (FINAL_RELEASE_BUILD)) && defined (PHYSICS_PHYSX)
			if (stringData != NULL)
			{
				game->getGamePhysics()->getPhysicsLib()->connectToRemoteDebugger(stringData, 5425);
			} else {
				sp->error("DevScripting::process - devPhysicsConnectToRemoteDebugger parameter missing.");
			}
#else
			sp->error("DevScripting::process - devPhysicsConnectToRemoteDebugger not in build.");
#endif
			break;

		case GS_CMD_clearSelectionVisualizationForUnifiedHandle:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				SelectionVisualizer::clearSelectionForUnifiedHandle(gsd->unifiedHandle);
			} else {
				sp->error("DevScripting::process - clearSelectionVisualizationForUnifiedHandle, unified handle not valid.");
			}
			break;

		case GS_CMD_setSelectionVisualizationForUnifiedHandle:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				SelectionVisualizer::setSelectionForUnifiedHandle(gsd->unifiedHandle);
			} else {
				sp->error("DevScripting::process - clearSelectionVisualizationForUnifiedHandle, unified handle not valid.");
			}
			break;

		case GS_CMD_reloadChangedScriptFiles:
			{
				int reloaded = util::ScriptManager::getInstance()->reloadChangedScripts();
				if (reloaded > 0)
				{
					std::string tmp = "reloadChangedScriptFiles - Reloaded ";
					tmp += int2str(reloaded);
					tmp += " script(s).";
					Logger::getInstance()->info(tmp.c_str());
				} else {
					Logger::getInstance()->info("reloadChangedScriptFiles - No scripts were reloaded.");
				}
			}
			break;

		case GS_CMD_restartApplicationSoft:
#ifdef PROJECT_AOV
			Logger::getInstance()->info("DevScripting::process - restartApplicationSoft called.");
			game->gameUI->setQuitRequested();
			::app_soft_restart_requested = true;
#else
			Logger::getInstance()->warning("DevScripting::process - restartApplicationSoft not in build.");
#endif
			break;

		case GS_CMD_restartApplicationHard:
#ifdef PROJECT_AOV
			Logger::getInstance()->info("DevScripting::process - restartApplicationHard called.");
			game->gameUI->setQuitRequested();
			::app_hard_restart_requested = true;
#else
			Logger::getInstance()->warning("DevScripting::process - restartApplicationHard not in build.");
#endif
			break;


		default:
			sp->error("DevScripting::process - Unknown command.");
			assert(0);
		}
	}
}


