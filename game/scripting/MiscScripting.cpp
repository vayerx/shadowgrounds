
#include "precompiled.h"

#include "MiscScripting.h"

#include "scripting_macros_start.h"
#include "misc_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>
#include <IStorm3D.h>
#include <istorm3d_mesh.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../ui/GameConsole.h"
#include "../GameStats.h"
#include "../../util/BuildingHandler.h"
#include "../GameMap.h"
#include "../EngineMetaValues.h"
#include "../GameScene.h"
#include "../GameRandom.h"
#include "../GameProfiles.h"
#include "../scaledefs.h"
#include "../createparts.h"
#include "../savegamevars.h"
#include "../Character.h"
#include "../ProjectileList.h"
#include "../Projectile.h"
#include "../UnitVisibilityChecker.h"
#include "../UnitLevelAI.h"
#include "../UnitList.h"
#include "../SimpleOptions.h"
#include "../options/options_players.h"
#include "../options/options_graphics.h"
#include "../../ui/GameController.h"
#include "../../ui/CombatWindow.h"
#include "../../util/LipsyncManager.h"
#include "../../util/StringUtil.h"
#include "../../ui/Animator.h"

#include "../physics/gamephysics.h"
#include "../physics/abstractphysicsobject.h"

#include "../../system/Logger.h"
#include "../../system/Timer.h"
#include "../../convert/int64_to_hex.h"
#include <stdio.h>

#include "../../convert/str2int.h"
#include "../../convert/int64_to_hex.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../util/TextureCache.h"
#include "../../util/GridOcclusionCuller.h"
#include "../DHLocaleManager.h"
#include "../ParticleSpawner.h"
#include "../ParticleSpawnerManager.h"
#include "../AlienSpawner.h"
#include "../unified_handle.h"
#include "../../ui/ErrorWindow.h"
#include "../../ui/ElaborateHintMessageWindow.h"
#include "../../ui/LightManager.h"
#ifdef PROJECT_SURVIVOR
	#include "../../ui/GenericTextWindow.h"
#endif
#include <istorm3d_terrain_renderer.h>
#include "../UnifiedHandleManager.h"
#include "../GameWorldFold.h"
#include "../../editor/UniqueEditorObjectHandle.h"
#include "../../editor/ueoh_to_id_string.h"

#include "../../util/Debug_MemoryManager.h"

#include "GameScripting.h"

// HACK:
//#include "../../chat/SGScriptChatter.h"

#ifdef PROJECT_CLAW_PROTO
#ifndef USE_CLAW_CONTROLLER
#define USE_CLAW_CONTROLLER
#endif
#endif

#ifdef USE_CLAW_CONTROLLER
#include "../ClawController.h"
#endif


int lipsync_start_time;

using namespace ui;

namespace game
{
	extern bool game_in_start_combat;

	static UnifiedHandle gs_lastAddedParticleSpawnerHandle = UNIFIED_HANDLE_NONE;

	void MiscScripting::process(util::ScriptProcess *sp, 
		int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
		GameScriptData *gsd, Game *game, bool *pause)
	{
		switch(command)
		{
		case GS_CMD_WAIT:
			{
				int paramTimeMsec = intData * 1000;
				gsd->waitCounter = ((paramTimeMsec + gsd->waitRemainder) / GAME_TICK_MSEC);
				gsd->waitRemainder = paramTimeMsec - (gsd->waitCounter * GAME_TICK_MSEC);
				if (gsd->waitRemainder < 0) gsd->waitRemainder = 0;
				assert(gsd->waitRemainder < GAME_TICK_MSEC);
				gsd->waitCounter--; 
				if (gsd->waitCounter < 0) gsd->waitCounter = 0;

				if (intData <= 0)
				{
					sp->warning("MiscScripting::process - wait with zero or negative parameter.");
				}
				if (intData >= 15)
				{
					sp->warning("MiscScripting::process - wait with parameter greater or equal than 15 seconds (probably unintended?).");
				}
			}
			*pause = true;
			break;

		case GS_CMD_WAITACCURATE:
			{
				int paramTimeMsec = intData;
				gsd->waitCounter = ((paramTimeMsec + gsd->waitRemainder) / GAME_TICK_MSEC);
				gsd->waitRemainder = paramTimeMsec - (gsd->waitCounter * GAME_TICK_MSEC);
				if (gsd->waitRemainder < 0) gsd->waitRemainder = 0;
				assert(gsd->waitRemainder < GAME_TICK_MSEC);
				gsd->waitCounter--; 
				if (gsd->waitCounter < 0) gsd->waitCounter = 0;
			}

			if (intData < 15)
			{
				sp->warning("MiscScripting::process - waitAccurate with parameter less than 15 milliseconds (probably unintended?).");
			}
			*pause = true;
			break;

		case GS_CMD_WAITTICK:
			gsd->waitCounter = 0;
			*pause = true;
			break;

		case GS_CMD_RANDOM:
			if (intData > 0)
			{
				*lastValue = (game->gameRandom->nextInt() % intData);
			} else {
				sp->warning("MiscScripting::process - random with zero or negative parameter.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_MESSAGE:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				// NEW: messages clear any previous messages!
				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_NORMAL);

				game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), NULL, 
					1, 3000 + strlen(usestr) * 40, GameUI::MESSAGE_TYPE_NORMAL);
			} else {
				sp->error("GameScripting::process - message parameter missing.");
			}
			break;

		case GS_CMD_characterMessageNoFace:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				// NEW: messages clear any previous messages!
				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_NORMAL);

				game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), NULL, 
					1, 240000, GameUI::MESSAGE_TYPE_NORMAL);
			} else {
				sp->error("GameScripting::process - message parameter missing.");
			}
			break;

		case GS_CMD_CENTERMESSAGE:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				// NEW: messages clear any previous messages!
				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_CENTER_BIG);

				game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), NULL, 
					3, 3000 + strlen(usestr) * 40, GameUI::MESSAGE_TYPE_CENTER_BIG);
			} else {
				sp->error("GameScripting::process - centerMessage parameter missing.");
			}
			break;

		case GS_CMD_HINTMESSAGE:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				// NEW: messages clear any previous messages!
				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_HINT);

				game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), NULL, 
					1, 4000 + strlen(usestr) * 40, GameUI::MESSAGE_TYPE_HINT);
			} else {
				sp->error("GameScripting::process - hintMessage parameter missing.");
			}
			break;

		case GS_CMD_getLocaleStringLength:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				const char *tmpstr = getLocaleSubtitleString(usestr);

				if (tmpstr != NULL)
				{
					*lastValue = (int)strlen(tmpstr);
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("GameScripting::process - message parameter missing.");
			}
			break;

		case GS_CMD_CLEARMESSAGE:
			game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_NORMAL);
			break;

		case GS_CMD_CLEARHINTMESSAGE:
			game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_HINT);
			break;

		case GS_CMD_CLEARCENTERMESSAGE:
			game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_CENTER_BIG);
			break;

		case GS_CMD_CLEAREXECUTETIPMESSAGE:
			game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_EXECUTE_TIP);
			break;

		case GS_CMD_EXECUTETIPMESSAGE:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_EXECUTE_TIP);

				game->gameUI->gameMessage(convertLocaleGuiString(usestr), NULL, 
					1, 700, GameUI::MESSAGE_TYPE_EXECUTE_TIP);
			} else {
				sp->error("GameScripting::process - executeTipMessage parameter missing.");
			}
			break;

		case GS_CMD_PRIORITYEXECUTETIPMESSAGE:
			if (stringData != NULL)
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_EXECUTE_TIP);

				game->gameUI->gameMessage(convertLocaleGuiString(usestr), NULL, 
					2, 700, GameUI::MESSAGE_TYPE_EXECUTE_TIP);
			} else {
				sp->error("GameScripting::process - executeTipMessage parameter missing.");
			}
			break;

		case GS_CMD_CHARACTERMESSAGE:
			{
				Unit *unit = gsd->unit;
				if (unit != NULL)
				{
					if (stringData != NULL)
					{
						char *usestr;
						if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
							usestr = gsd->stringValue;
						else
							usestr = stringData;

						Visual2D *img = NULL;
						if (unit->getCharacter() != NULL)
						{
							img = unit->getCharacter()->getMessageImage();
						}

						// NEW: messages clear any previous messages!
						//game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_RADIO);
						// cannot do this. we don't want to clear left/right side message when
						// adding the other... need to just make sure the message gets overridden..
						game->gameUI->clearGameMessageDuration(GameUI::MESSAGE_TYPE_RADIO);


						if (unit->getCharacter() != NULL
							&& unit->getCharacter()->getLipsyncId() != NULL)
						{
							int renderTargetNumber = 0;
							GameUI::MESSAGE_TYPE radioSide = GameUI::MESSAGE_TYPE_RADIO;
							if (game->gameUI->getLipsyncManager()->getCharacter(util::LipsyncManager::Right) ==
								unit->getCharacter()->getLipsyncId())
							{
								// this character was on right side...
								radioSide = GameUI::MESSAGE_TYPE_RADIO2;
								renderTargetNumber = 1;
							}

							ui::Visual2D *vis2d = NULL;
							if (renderTargetNumber == 1)
							{
								vis2d = Character::conversationFace2;
								if (vis2d == NULL)
								{
									vis2d = new ui::Visual2D(renderTargetNumber);
									Character::conversationFace2 = vis2d;
								}
							} else {
								vis2d = Character::conversationFace1;
								if (vis2d == NULL)
								{
									vis2d = new ui::Visual2D(renderTargetNumber);
									Character::conversationFace1 = vis2d;
								}
							}

							if (!game->gameUI->getLipsyncManager()->isActive())
							{
								sp->warning("GameScripting::process - Attempt to characterMessage when lipsync manager is inactive.");
							}

							game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), vis2d, 
								1, 240000, radioSide);
						} else {
							game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), img, 
								1, 240000, GameUI::MESSAGE_TYPE_RADIO);
						}
					} else {
						sp->error("GameScripting::process - characterMessage parameter missing.");
					}
				} else {
					sp->warning("GameScripting::process - Attempt to characterMessage for null unit.");
				}
			}
			break;

		case GS_CMD_PLAYLIPSYNC:
			{
				Unit *unit = gsd->unit;
				if (unit != NULL)
				{
					if (stringData != NULL)
					{
						char *usestr;
						if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
							usestr = gsd->stringValue;
						else
							usestr = stringData;

						if (unit->getCharacter() != NULL
							&& unit->getCharacter()->getLipsyncId() != NULL)
						{
							boost::shared_ptr<sfx::AmplitudeArray> amplitudes = game->gameUI->getLipsyncManager()->getAmplitudeBuffer(convertLocaleSpeechString(usestr));
							game->gameUI->getLipsyncManager()->playSpeech(unit->getCharacter()->getLipsyncId(), amplitudes, lipsync_start_time);

							if (!game->gameUI->getLipsyncManager()->isActive())
							{
								sp->warning("GameScripting::process - Attempt to playLipsync when lipsync manager is inactive.");
							}
						} else {
							sp->warning("GameScripting::process - Attempt to playLipsync for unit with no lipsync id.");
						}
					} else {
						sp->error("GameScripting::process - playLipsync parameter missing.");
					}
				} else {
					sp->warning("GameScripting::process - Attempt to playLipsync for null unit.");
				}
			}
			break;

		case GS_CMD_SETPLAYER:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				gsd->player = intData;
			} else {
				sp->error("GameScripting::process - setPlayer value out of range.");
			}
			break;

		case GS_CMD_SETPLAYERTOVALUE:
			if (*lastValue >= 0 && *lastValue < ABS_MAX_PLAYERS)
			{
				gsd->player = *lastValue;
			} else {
				sp->error("GameScripting::process - setPlayerToValue last value out of range.");
			}
			break;

		case GS_CMD_GETPLAYER:
			*lastValue = gsd->player;
			break;

		case GS_CMD_GETSINGLEPLAYER:
			*lastValue = game->singlePlayerNumber;
			break;

		case GS_CMD_DISABLEVISIBILITYUPDATE:
			game->visibilityChecker->setUpdateEnabled(false);
			break;

		case GS_CMD_ENABLEVISIBILITYUPDATE:
			game->visibilityChecker->setUpdateEnabled(true);
			break;

		case GS_CMD_PAUSE:
			game->gameUI->setScrollyTemporarilyDisabled(false);
			game->setPaused(true);
			break;

		case GS_CMD_UNPAUSE:
			// FIXME: warning! set scrolly on, regardless of 
			// what its state was before pause (as it might have
			// been disabled by a cinematic script)
			game->gameUI->setScrollyTemporarilyDisabled(true);
			game->setPaused(false);
			break;

		case GS_CMD_PRINTVALUE:
			Logger::getInstance()->info(int2str(*lastValue));
			break;

		case GS_CMD_DISABLECONTROLS:
			game->gameUI->setControlsEnabled(game->singlePlayerNumber, false);
			break;

		case GS_CMD_ENABLECONTROLS:
			game->gameUI->setControlsEnabled(game->singlePlayerNumber, true);
			break;

		case GS_CMD_WAITVALUE:
			{
				int paramTimeMsec = *lastValue;
				gsd->waitCounter = ((paramTimeMsec + gsd->waitRemainder) / GAME_TICK_MSEC);
				gsd->waitRemainder = paramTimeMsec - (gsd->waitCounter * GAME_TICK_MSEC);
				if (gsd->waitRemainder < 0) gsd->waitRemainder = 0;
				assert(gsd->waitRemainder < GAME_TICK_MSEC);
				gsd->waitCounter--; 
				if (gsd->waitCounter < 0) gsd->waitCounter = 0;
			}

			if (*lastValue < 0)
			{
				sp->warning("MiscScripting::process - waitValue with negative parameter.");
			}
			*pause = true;
			break;

		case GS_CMD_STARTSCRIPTPROCESS:
			if (stringData != NULL)
			{
				char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
				{
					s = gsd->stringValue;
				}
				if (s != NULL)
				{
					std::vector<int> *params = NULL;
					std::string name = s;

					// TODO: refactor this param parsing, and use that with other startScriptProcess... commands too
					//parse_script_name_and_params(s, name, params);
					//void parse_script_name_and_params(const char *s, std::string &name, std::vector<int> **params);
					// move this to that function...

					int slen = strlen(s);
					for (int i = 0; i < slen; i++)
					{
						if (s[i] == '(')
						{
							std::string tmp = s;
							name = tmp.substr(0, i);
							if (s[i + 1] != ')')
							{
								for (int j = i; j < slen; j++)
								{
									if (s[j] == ')')
									{
										std::string paramtmp = tmp.substr(i + 1, j - (i + 1));
										std::vector<std::string> paramVars;
										util::parse_script_params(paramtmp, paramVars);
										params = new std::vector<int>;
										for (int k = 0; k < (int)paramVars.size(); k++)
										{
											int val = game->gameScripting->getGlobalIntVariableValue(paramVars[k].c_str());
											params->push_back(val);
										}
									}
								}
							}
							break;
						}
					}

					// end of move this.

					*lastValue = game->addCustomScriptProcess(name.c_str(), NULL, params);

					if (params != NULL)
						delete params;

					if (!game->inCombat && !game_in_start_combat)
					{
						sp->warning("GameScripting::process - startScriptProcess called while in menus (will not run until in mission).");
					}
				} else {
					sp->error("GameScripting::process - startScriptProcess, null string value.");
				}
			} else {
				sp->error("GameScripting::process - startScriptProcess parameter missing.");
			}
			break;

		case GS_CMD_GETSCRIPTPROCESSID:
			// (this would be unnecessarily complex.)
			//*lastValue = game->getCustomScriptProcessId(sp);
			//if (*lastValue == 0)
			//{
			//	sp->warning("GameScripting::process - Called getScriptProcessId for script process that is not a custom script process.");
			//}
			*lastValue = sp->getId();
			break;

		case GS_CMD_ISSCRIPTPROCESSOFVALUECINEMATIC:
			if (*lastValue >= SCRIPTPROCESS_MIN_ID 
				&& *lastValue <= SCRIPTPROCESS_MAX_ID)
			{
				if (game->getCinematicScriptProcessId() == *lastValue)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("GameScripting::process - isScriptProcessOfValueCinematic value out of range (script process id expected in value).");
				*lastValue = 0;
			}
			break;

		case GS_CMD_STARTSCRIPTPROCESSFORUNIT:
			if (stringData != NULL)
			{
				char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
				{
					s = gsd->stringValue;
				}
				if (s != NULL)
				{
					if (gsd->unit != NULL)
					{
						if (game->inCombat)
						{
							// TODO: refactor the startScriptProcess command param parsing, and use that here too
							*lastValue = game->addCustomScriptProcess(s, gsd->unit, NULL);
						} else {
							sp->error("GameScripting::process - Attempt to startScriptProcessForUnit while in menus.");
						}
					} else {
						sp->error("GameScripting::process - Attempt to startScriptProcessForUnit for null unit.");
					}
				} else {
					sp->error("GameScripting::process - startScriptProcessForUnit, null string value.");
				}
			} else {
				sp->error("GameScripting::process - startScriptProcessForUnit parameter missing.");
			}
			break;

		case GS_CMD_startScriptProcessForUnifiedHandle:
			if (stringData != NULL)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (game->inCombat)
					{
						// TODO: refactor the startScriptProcess command param parsing, and use that here too
						*lastValue = game->addCustomScriptProcess(stringData, gsd->unifiedHandle, NULL);
					} else {
						sp->error("GameScripting::process - Attempt to startScriptProcessForUnifiedHandle while in menus.");
					}
				} else {
					if (gsd->unifiedHandle == UNIFIED_HANDLE_NONE)
						sp->warning("GameScripting::process - Attempt to startScriptProcessForUnifiedHandle for unified handle none.");
					else
						sp->warning("GameScripting::process - Attempt to startScriptProcessForUnifiedHandle for invalid unified handle.");
				}
			} else {
				sp->error("GameScripting::process - startScriptProcessForUnifiedHandle parameter missing.");
			}
			break;

		case GS_CMD_STOPSCRIPTPROCESSBYID:
			if (*lastValue >= SCRIPTPROCESS_MIN_ID 
				&& *lastValue <= SCRIPTPROCESS_MAX_ID)
			{
				game->stopCustomScriptProcessById(*lastValue);
			} else {
				sp->error("GameScripting::process - stopScriptProcessById value out of range (script process id expected in value).");
			}
			break;

		case GS_CMD_runScriptProcessImmediately:
			if (stringData != NULL)
			{
				// TODO: refactor the startScriptProcess command param parsing, and use that here too

				std::string scriptname = stringData;
				int slen = strlen(stringData);
				std::string subname = "main";
				for (int i = 0; i < slen; i++)
				{
					if (stringData[i] == ':')
					{
						subname = &stringData[i + 1];
						scriptname = scriptname.substr(0, i);
						break;
					}
				}
				game->gameScripting->runOtherScript(scriptname.c_str(), subname.c_str(), NULL, VC3(0,0,0));
			} else {
				sp->error("GameScripting::process - runScriptProcessImmediately parameter missing.");
			}
			break;

		case GS_CMD_runScriptProcessImmediatelyForUnit:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					// TODO: refactor the startScriptProcess command param parsing, and use that here too

					std::string scriptname = stringData;
					int slen = strlen(stringData);
					std::string subname = "main";
					for (int i = 0; i < slen; i++)
					{
						if (stringData[i] == ':')
						{
							subname = &stringData[i + 1];
							scriptname = scriptname.substr(0, i);
							break;
						}
					}
					game->gameScripting->runOtherScript(scriptname.c_str(), subname.c_str(), gsd->unit, VC3(0,0,0));
				} else {
					sp->error("GameScripting::process - Attempt to runScriptProcessImmediatelyForUnit for null unit.");
				}
			} else {
				sp->error("GameScripting::process - runScriptProcessImmediately parameter missing.");
			}
			break;

		case GS_CMD_DISABLEALLAI:
			UnitLevelAI::setAllEnabled(false);
			break;
		
		case GS_CMD_ENABLEALLAI:
			UnitLevelAI::setAllEnabled(true);
			break;

		case GS_CMD_DISABLEHOSTILEAI:
			{
				for (int i = 0; i < ABS_MAX_PLAYERS; i++)
				{
					if (game->isHostile(gsd->player, i))
						UnitLevelAI::setPlayerAIEnabled(i, false);
				}
			}
			break;
		
		case GS_CMD_ENABLEHOSTILEAI:
			{
				for (int i = 0; i < ABS_MAX_PLAYERS; i++)
				{
					if (game->isHostile(gsd->player, i))
						UnitLevelAI::setPlayerAIEnabled(i, true);
				}
			}
			break;

		case GS_CMD_DISABLEAIFORPLAYER:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				UnitLevelAI::setPlayerAIEnabled(intData, false);
			} else {
				sp->error("MiscScripting::process - disableAIForPlayer parameter out of range (player side number expected).");
			}
			break;

		case GS_CMD_ENABLEAIFORPLAYER:
			if (intData >= 0 && intData < ABS_MAX_PLAYERS)
			{
				UnitLevelAI::setPlayerAIEnabled(intData, true);
			} else {
				sp->error("MiscScripting::process - enableAIForPlayer parameter out of range (player side number expected).");
			}
			break;

		case GS_CMD_HUMANPLAYERSAMOUNT:
			*lastValue = 0;
			if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
				(*lastValue)++;
			if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
				(*lastValue)++;
			if (SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
				(*lastValue)++;
			if (SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
				(*lastValue)++;
			break;

		case GS_CMD_FORCECONTROLLERKEYENABLED:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							controller->setForcedEnable(ctrlnum, true);
						} else {
							sp->error("MiscScripting::process - forceControllerKeyEnabled parameter invalid, no such control name."); 
							sp->debug(stringData);	
						}
					} else {
						sp->error("MiscScripting::process - forceControllerKeyEnabled parameter missing, control name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - forceControllerKeyEnabled, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - forceControllerKeyEnabled last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_ISCONTROLLERKEYDOWN:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							// THIS IS INCORRECT CRAP THAT JUST SCREWS UP THE ENABLED FLAGS! --jpk
							//bool controller_status = controller->getControlsEnabled();
							//if( controller_status == false )
							//	controller->setControlsEnabled( true );

							if (controller->isKeyDown(ctrlnum))
								*lastValue = 1;
							else
								*lastValue = 0;

							//if( controller_status == false )
							//	controller->setControlsEnabled( controller_status );
						} else {
							sp->error("MiscScripting::process - isControllerKeyDown parameter invalid, no such control name."); 
							sp->debug(stringData);	
						}
					} else {
						sp->error("MiscScripting::process - isControllerKeyDown parameter missing, control name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - isControllerKeyDown, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - isControllerKeyDown last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_WASCONTROLLERKEYCLICKED:
			// NOTE: this only correctly applies to scripts that call this once per frame (not once per tick)
			// if it is called once per tick, the script needs to handle multiple true returns within that frame
			// (see some script macro implementations for that) --jpk
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							if (controller->wasKeyClicked(ctrlnum))
								*lastValue = 1;
							else
								*lastValue = 0;
						} else {
							sp->error("MiscScripting::process - wasControllerKeyClicked parameter invalid, no such control name."); 
							sp->debug(stringData);	
						}
					} else {
						sp->error("MiscScripting::process - wasControllerKeyClicked parameter missing, control name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - wasControllerKeyClicked, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - wasControllerKeyClicked last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_SETCONTROLLERKEYDOWN:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							controller->setControlOn(ctrlnum);
						} else {
							sp->error("MiscScripting::process - setControllerKeyDown parameter invalid, no such control name."); 
							sp->debug(stringData);	
						}
					} else {
						sp->error("MiscScripting::process - setControllerKeyDown parameter missing, control name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - setControllerKeyDown, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - setControllerKeyDown last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_SETCONTROLLERKEYUP:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							controller->setControlOff(ctrlnum);
						} else {
							sp->error("MiscScripting::process - setControllerKeyUp parameter invalid, no such control name."); 
							sp->debug(stringData);	
						}
					} else {
						sp->error("MiscScripting::process - setControllerKeyUp parameter missing, control name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - setControllerKeyUp, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - setControllerKey last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_getControllerSceneSelectionPosition:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				SceneSelection *s = game->gameUI->getSceneSelection(*lastValue);
				if (s->unit != NULL)
				{
					gsd->position = s->unit->getPosition();
				} else {
					gsd->position.x = s->scaledMapX;
					gsd->position.z = s->scaledMapY;
					if (game->gameMap->isWellInScaledBoundaries(gsd->position.x, gsd->position.z))
					{
						gsd->position.y = game->gameMap->getScaledHeightAt(gsd->position.x, gsd->position.z);
					} else {
						gsd->position.y = 0.0f;
					}
				}
			} else {
				sp->error("MiscScripting::process - getControllerSceneSelectionPosition last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_getControllerSceneSelectionUnit:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				SceneSelection *s = game->gameUI->getSceneSelection(*lastValue);
				if (s->unit != NULL)
				{
					gsd->unit = s->unit;
					*lastValue = 1;
				} else {
					gsd->unit = NULL;
					*lastValue = 0;
				}
			} else {
				sp->error("MiscScripting::process - getControllerSceneSelectionUnit last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
				*lastValue = 0;
			}
			break;

		case GS_CMD_isControllerSceneSelectionAvailable:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				SceneSelection *s = game->gameUI->getSceneSelection(*lastValue);
				if (s->hit)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("MiscScripting::process - isControllerSceneSelectionAvailable last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
				*lastValue = 0;
			}
			break;

		case GS_CMD_getControllerCursorScreenPositionX:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				*lastValue = game->gameUI->getCursorScreenX(*lastValue);
			} else {
				sp->error("MiscScripting::process - getControllerCursorScreenPositionX last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
				*lastValue = 0;
			}
			break;

		case GS_CMD_getControllerCursorScreenPositionY:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				*lastValue = game->gameUI->getCursorScreenY(*lastValue);
			} else {
				sp->error("MiscScripting::process - getControllerCursorScreenPositionY last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
				*lastValue = 0;
			}
			break;

		case GS_CMD_MAKELIGHTNING:
			if (game->inCombat)
			{
				game->gameUI->setEnvironmentLightning(gsd->position);
			}
			break;
 
		case GS_CMD_BIND:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						char buf[256];
						if (strlen(stringData) < 255)
						{
							strcpy(buf, stringData);
						} else {
							buf[0] = '\0';
						}
						int buflen = strlen(buf);
						int splitpos = 0;
						for (int i = 0; i < buflen; i++)
						{
							if (buf[i] == ',') 
							{
								buf[i] = '\0';
								if (i > 0 && buf[i - 1] == ' ')
								{
									buf[i - 1] = '\0';
								}
								splitpos = i + 1;
								if (buf[i + 1] == ' ')
									splitpos++;
								break;
							}
						}
						int ctrlnum = controller->getControlNumberForName(&buf[0]);
						int keycode = controller->getKeycodeNumberForName(&buf[splitpos]);
						if (ctrlnum != -1)
						{
							if (splitpos != 0)
							{
								if (keycode != -1)
								{
									controller->bindKey(ctrlnum, keycode, -1, false);
								} else {
									sp->error("MiscScripting::process - bind parameter invalid, no such keycode name.");	
									sp->debug(&buf[splitpos]);	
								}
							} else {
								sp->error("MiscScripting::process - bind parameter invalid, keycode name expected after control name.");	
								sp->debug(&buf[splitpos]);	
							}
						} else {
							sp->error("MiscScripting::process - bind parameter invalid, no such control name.");	
							sp->debug(&buf[0]); 
						}
					} else {
						sp->error("MiscScripting::process - bind parameter missing, control and keycode name expected."); 
					}
				} else {
					sp->error("MiscScripting::process - bind, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - bind last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_UNBIND:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						int ctrlnum = controller->getControlNumberForName(stringData);
						if (ctrlnum != -1)
						{
							controller->unbindKey(ctrlnum);
						} else {
							sp->debug("MiscScripting::process - unbind parameter was not a control name, assuming it is keycode name.");
							sp->debug(stringData);	
							int keycode = controller->getKeycodeNumberForName(stringData);
							if (keycode != -1)
							{
								controller->unbindKeyByKeycode(keycode);
							} else {
								sp->error("MiscScripting::process - unbind parameter invalid, no such control or keycode name."); 
								sp->debug(stringData);	
							}
						}
					} else {
						sp->error("MiscScripting::process - unbind parameter missing, control or keycode name expected.");	
					}
				} else {
					sp->error("MiscScripting::process - unbind, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - unbind last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_LISTBINDS:
			// TODO: ...
			assert(!"TODO");
			break;

		case GS_CMD_QUIT:
			game->gameUI->setQuitRequested();
			break;

		case GS_CMD_LOADBINDS:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				ui::GameController *controller = game->gameUI->getController(*lastValue);
				if (controller != NULL)
				{
					if (stringData != NULL)
					{
						controller->loadConfiguration(stringData);
					} else {
						sp->error("MiscScripting::process - loadBinds parameter missing, filename expected.");
					}
				} else {
					sp->error("MiscScripting::process - loadBinds, null controller for given last value (controller number).");
					sp->debug(int2str(*lastValue));
				}
			} else {
				sp->error("MiscScripting::process - loadBinds last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
			}
			break;

		case GS_CMD_PRELOADTEXTURE:
			if (stringData != NULL)
			{
				game->gameUI->getTextureCache()->loadTexture(stringData, false);
			} else {
				sp->error("MiscScripting::process - preloadTexture, parameter missing, texture filename expected.");
			}
			break;

		case GS_CMD_PRELOADTEMPORARYTEXTURE:
			if (stringData != NULL)
			{
				game->gameUI->getTextureCache()->loadTexture(stringData, true);
			} else {
				sp->error("MiscScripting::process - preloadTemporaryTexture, parameter missing, texture filename expected.");
			}
			break;

		case GS_CMD_CLEANTEMPORARYTEXTURECACHE:
			game->gameUI->getTextureCache()->clearTemporary();
			break;

		case GS_CMD_SETGLOBALTIMEFACTOR:
			if (stringData != NULL)
			{
				float factor = (float)atof(stringData);
				if (factor < 0.1f)
				{
					factor = 0.1f;
				}
				if (factor > 10.0f)
				{
					factor = 10.0f;
				}
				game->gameUI->setTimeFactor(factor);
			} else {
				sp->error("MiscScripting::process - setGlobalTimeFactor, parameter missing.");
			}
			break;

		case GS_CMD_ADDPARTICLESPAWNER:
			if (stringData != NULL)
			{
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("MiscScripting::process - addParticleSpawner, reference to unloaded part type.");
				} else {
					if (pt->isInherited(
						getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
					{ 
						// WARNING: unsafe cast!
						Weapon *weap = (Weapon *)pt;
						VC3 particlepos = gsd->position;
						game->gameMap->keepWellInScaledBoundaries(&particlepos.x, &particlepos.z);
						float angle = (float)(*lastValue);
						VC3 dir = VC3(0,0,0);
						dir.x = -sinf(UNIT_ANGLE_TO_RAD(angle));
						dir.z = -cosf(UNIT_ANGLE_TO_RAD(angle));

						if (gsd->stringValue != NULL)
						{
							ParticleSpawner *duplicateTest = game->particleSpawnerManager->getParticleSpawnerByName(gsd->stringValue);
							if (duplicateTest != NULL)
							{
								sp->warning("MiscScripting::process - addParticleSpawner, created particle spawner with duplicate name.");
							}
						}

						ParticleSpawner *spawner = game->particleSpawnerManager->createParticleSpawner();
						spawner->setName(gsd->stringValue);
						spawner->setPosition(particlepos);
						spawner->setDirection(dir);
						spawner->setSpawnerWeapon(weap);
						spawner->enable();

						gs_lastAddedParticleSpawnerHandle = game->particleSpawnerManager->getUnifiedHandle(spawner);
					} else {
						sp->error("MiscScripting::process - addParticleSpawner, attempt to use non-weapon part type.");
					}
				}
			} else {
				sp->error("MiscScripting::process - AddParticleSpawner parameter missing, weapon name expected.");
			}
			break;

		case GS_CMD_DISABLEPARTICLESPAWNERBYNAME:
			if (stringData != NULL)
			{
				ParticleSpawner *spawner = game->particleSpawnerManager->getParticleSpawnerByName(stringData);
				if (spawner != NULL)
				{
					spawner->disable();
				} else {
					sp->error("MiscScripting::process - disableParticleSpawnerByName, no particle spawner with given name found.");
				}
			} else {
				sp->error("MiscScripting::process - disableParticleSpawnerByName parameter missing.");
			}
			break;

		case GS_CMD_ENABLEPARTICLESPAWNERBYNAME:
			if (stringData != NULL)
			{
				ParticleSpawner *spawner = game->particleSpawnerManager->getParticleSpawnerByName(stringData);
				if (spawner != NULL)
				{
					spawner->enable();
				} else {
					sp->error("MiscScripting::process - enableParticleSpawnerByName, no particle spawner with given name found.");
				}
			} else {
				sp->error("MiscScripting::process - enableParticleSpawnerByName parameter missing.");
			}
			break;

		case GS_CMD_disableAllParticleSpawners:
			game->particleSpawnerManager->disableAllParticleSpawners();
			break;

		case GS_CMD_deleteAllParticleSpawners:
			game->particleSpawnerManager->deleteAllParticleSpawners();
			break;

		case GS_CMD_SPAWNPROJECTILE:
			if (stringData != NULL)
			{
				char *s;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;
				else
					s = stringData;

				if (s != NULL)
				{
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(s));
					if (pt == NULL) 
					{ 
						sp->error("MiscScripting::process - spawnProjectile, reference to unloaded part type.");
					} else {
						if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
						{
							// WARNING: unsafe cast!
							Weapon *weap = (Weapon *)pt;
							VC3 projpos = gsd->position;
							game->gameMap->keepWellInScaledBoundaries(&projpos.x, &projpos.z);
							float angle = (float)(*lastValue);
							VC3 dir = VC3(0,0,0);
							dir.x = -sinf(UNIT_ANGLE_TO_RAD(angle));
							dir.z = -cosf(UNIT_ANGLE_TO_RAD(angle));

							ParticleSpawner::spawnProjectileWithWeapon(game, weap, projpos, dir);
						} else {
							sp->error("MiscScripting::process - spawnProjectile, attempt to use non-weapon part type.");
						}
					}
				} else {
					sp->error("MiscScripting::process - spawnProjectile, null string value.");
				}
			} else {
				sp->error("MiscScripting::process - spawnProjectile parameter missing, weapon name expected.");
			}
			break;

		case GS_CMD_spawnProjectileWithShooter:
			if (stringData != NULL)
			{
				char *s;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;
				else
					s = stringData;

				if (s != NULL)
				{
					PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(s));
					if (pt == NULL) 
					{ 
						sp->error("MiscScripting::process - spawnProjectileWithShooter, reference to unloaded part type.");
					} else {
						if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
						{
							// WARNING: unsafe cast!
							Weapon *weap = (Weapon *)pt;
							VC3 projpos = gsd->position;
							game->gameMap->keepWellInScaledBoundaries(&projpos.x, &projpos.z);
							float angle = (float)(*lastValue);
							VC3 dir = VC3(0,0,0);
							dir.x = -sinf(UNIT_ANGLE_TO_RAD(angle));
							dir.z = -cosf(UNIT_ANGLE_TO_RAD(angle));

							Projectile *proj = ParticleSpawner::spawnProjectileWithWeapon(game, weap, projpos, dir);
							proj->setShooter(gsd->unit);
						} else {
							sp->error("MiscScripting::process - spawnProjectileWithShooter, attempt to use non-weapon part type.");
						}
					}
				} else {
					sp->error("MiscScripting::process - spawnProjectileWithShooter, null string value.");
				}
			} else {
				sp->error("MiscScripting::process - spawnProjectileWithShooter parameter missing, weapon name expected.");
			}
			break;

		case GS_CMD_MAKEHEIGHTAREABLOCKED:
			game->gameMap->makeHeightAreaBlocked(game->gameMap->scaledToHeightmapX(gsd->position.x), game->gameMap->scaledToHeightmapY(gsd->position.z));
			break;

		case GS_CMD_makeUnreachableAreaFromPositionBlocked:
			game->gameMap->makeUnreachableAreasBlocked(game->gameMap->scaledToObstacleX(gsd->position.x), game->gameMap->scaledToObstacleY(gsd->position.z));
			break;

		case GS_CMD_RANDOMBELOWCURRENTVALUE:
			if (*lastValue > 0)
			{
				*lastValue = (game->gameRandom->nextInt() % *lastValue);
			} else {
				sp->warning("GameScripting::process - randomToValue with zero or negative register value.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_RANDOMOFFSETVALUE:
			if (intData > 0)
			{
				*lastValue += (game->gameRandom->nextInt() % (intData * 2 + 1)) - intData;
			} else {
				sp->warning("GameScripting::process - randomOffsetValue with zero or negative input.");
			}
			break;

		case GS_CMD_RANDOMPOSITIVEOFFSETVALUE:
			if (intData > 0)
			{
				*lastValue += (game->gameRandom->nextInt() % (intData + 1));
			} else {
				sp->warning("GameScripting::process - randomPositiveOffsetValue with zero or negative input.");
			}
			break;

		case GS_CMD_RANDOMNEGATIVEOFFSETVALUE:
			if (intData > 0)
			{
				*lastValue -= (game->gameRandom->nextInt() % (intData + 1));
			} else {
				sp->warning("GameScripting::process - randomNegativeOffsetValue with zero or negative input.");
			}
			break;

		case GS_CMD_RESTOREPARTTYPEORIGINALS:
			restorePartTypeOriginals();
			break;

		case GS_CMD_FORCEBUILDINGROOFHIDE:
			game->gameUI->forceBuildingRoofHide();
			break;

		case GS_CMD_FORCEBUILDINGROOFSHOW:
			game->gameUI->forceBuildingRoofShow();
			break;

		case GS_CMD_ENDFORCEDBUILDINGROOF:
			game->gameUI->endForcedBuildingRoof();
			break;

		case GS_CMD_HIDESKYMODEL:
			game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderSkyModel, false);
			break;

		case GS_CMD_SHOWSKYMODEL:
			game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderSkyModel, true);
			break;

		case GS_CMD_SAVEPERMANENTVARIABLESTOTEMPORARY:
			game->saveVariablesToTemporary();
			break;

		case GS_CMD_LOADPERMANENTVARIABLESFROMTEMPORARY:
			game->loadVariablesFromTemporary();
			break;

		case GS_CMD_SAVEGAME:
			if (stringData != NULL)
			{
				game->saveGame(stringData);
			} else {
				sp->warning("GameScripting::process - saveGame parameter missing, expected save id.");
			}
			break;

		case GS_CMD_LOADGAME:
			if (stringData != NULL)
			{
				game->loadGame(stringData);
			} else {
				sp->warning("GameScripting::process - loadGame parameter missing, expected save id.");
			}
			break;

		case GS_CMD_SAVEGAMETYPE:
			if (stringData != NULL)
			{
				savegame_type = stringData;
			} else {
				sp->warning("GameScripting::process - savegameType parameter missing, expected 'savegame' or 'newgame'.");
			}
			break;

		case GS_CMD_SAVEGAMEVERSION:
			if (stringData != NULL)
			{
				savegame_version = stringData;
			} else {
				sp->warning("GameScripting::process - savegameVersion parameter missing, expected savegame version string.");
			}
			break;

		case GS_CMD_SAVEGAMEDESCRIPTION:
			if (stringData != NULL)
			{
				savegame_description = stringData;
			} else {
				sp->warning("GameScripting::process - savegameDescription parameter missing, expected savegame description string.");
			}
			break;

		case GS_CMD_SAVEGAMETIME:
			if (stringData != NULL)
			{
				savegame_time = stringData;
			} else {
				sp->warning("GameScripting::process - savegameDescription parameter missing, expected savegame time string.");
			}
			break;

		case GS_CMD_enableMaterialScrollByName:
			if (stringData != NULL)
			{
				IStorm3D *storm = game->gameUI->getStorm3D();
				if(storm)
				{
					boost::scoped_ptr<Iterator<IStorm3D_Model *> > model_iterator(storm->ITModel->Begin());
					for(; !model_iterator->IsEnd(); model_iterator->Next())
					{
						IStorm3D_Model *model = model_iterator->GetCurrent();
						if(!model)
							continue;

						boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
						for(; !object_iterator->IsEnd(); object_iterator->Next())
						{
							IStorm3D_Model_Object *object = object_iterator->GetCurrent();
							if(!object)
								continue;

							IStorm3D_Mesh *mesh = object->GetMesh();
							if(!mesh)
								continue;

							IStorm3D_Material *m = mesh->GetMaterial();
							if(!m)
								continue;
							const char *name = m->GetName();
							if(!name)
								continue;

							if(strstr(name, stringData))
								m->EnableScroll(true);
						}
					}
				}
			} else {
				sp->warning("GameScripting::process - enableMaterialScrollByName parameter missing.");
			}
			break;

		case GS_CMD_disableMaterialScrollByName:
			if (stringData != NULL)
			{
				IStorm3D *storm = game->gameUI->getStorm3D();
				if(storm)
				{
					boost::scoped_ptr<Iterator<IStorm3D_Model *> > model_iterator(storm->ITModel->Begin());
					for(; !model_iterator->IsEnd(); model_iterator->Next())
					{
						IStorm3D_Model *model = model_iterator->GetCurrent();
						if(!model)
							continue;

						boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
						for(; !object_iterator->IsEnd(); object_iterator->Next())
						{
							IStorm3D_Model_Object *object = object_iterator->GetCurrent();
							if(!object)
								continue;

							IStorm3D_Mesh *mesh = object->GetMesh();
							if(!mesh)
								continue;

							IStorm3D_Material *m = mesh->GetMaterial();
							if(!m)
								continue;
							const char *name = m->GetName();
							if(!name)
								continue;

							if(strstr(name, stringData))
								m->EnableScroll(false);
						}
					}
				}
			} else {
				sp->warning("GameScripting::process - disableMaterialScrollByName parameter missing.");
			}
			break;

		case GS_CMD_addAlienRandomSpawnPoint:
			if (stringData != NULL)
			{
				game->alienSpawner->addSpawnPoint(gsd->position, stringData);
			} else {
				sp->warning("GameScripting::process - addAlienRandomSpawnPoint parameter missing.");
			}
			break;

		case GS_CMD_resetAlienSpawner:
			game->alienSpawner->reset();
			break;

		case GS_CMD_enableAlienSpawner:
			game->alienSpawner->enable();
			break;

		case GS_CMD_disableAlienSpawner:
			game->alienSpawner->disable();
			break;

		case GS_CMD_disableAlienSpawnerSpawnPoint:
			if (stringData != NULL)
			{
				game->alienSpawner->disableSpawnerScript(stringData);
			} else {
				sp->warning("GameScripting::process - disableAlienSpawnerSpawnPoint parameter missing.");
			}
			break;

		case GS_CMD_enableAlienSpawnerSpawnPoint:
			if (stringData != NULL)
			{
				game->alienSpawner->enableSpawnerScript(stringData);
			} else {
				sp->warning("GameScripting::process - enableAlienSpawnerSpawnPoint parameter missing.");
			}
			break;

		case GS_CMD_setAlienSpawnerSpawnRate:
			if (intData >= GAME_TICK_MSEC)
			{
				game->alienSpawner->setSpawnRate(intData);
			} else {
				sp->warning("GameScripting::process - setAlienSpawnerSpawnRate parameter out of range.");
			}
			break;

		case GS_CMD_setAlienSpawnerNextSpawnDelay:
			if (intData >= GAME_TICK_MSEC)
			{
				game->alienSpawner->setNextSpawnDelay(intData);
			} else {
				sp->warning("GameScripting::process - setNextAlienSpawnerNextSpawnDelay parameter out of range.");
			}
			break;

		case GS_CMD_setProfile:
			if (stringData != NULL)
			{
				if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
				{
					game->getGameProfiles()->setCurrentProfile(stringData, *lastValue);

					if( std::string( stringData ).empty() == false )
					{
						int c = *lastValue;

#ifdef LEGACY_FILES
						std::string tmp = "Profiles/";
						tmp += game->getGameProfiles()->getCurrentProfile( c );
						tmp += "/Config/keybinds.txt";
#else
						std::string tmp = "profiles/";
						tmp += game->getGameProfiles()->getCurrentProfile( c );
						tmp += "/config/keybinds.txt";
#endif
						game->gameUI->getController( c )->loadConfiguration( tmp.c_str() );

						if( SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + c ) )
						{
							GameController* gameController = game->getGameUI()->getController( c );
							
							if( gameController->controllerTypeHasMouse() )
							{
								SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, true );
							}
							else
							{
								SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, false );
							}
							SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + c, gameController->getControllerType() );
						}
					}
					else
					{
						game->gameUI->getController( *lastValue )->unloadConfiguration();
					}
			
				} else {
					sp->warning("GameScripting::process - setProfile value out of range.");
				}
			} else {
				sp->warning("GameScripting::process - setProfile parameter missing.");
			}
			break;

		case GS_CMD_statDeath:
			if (gsd->unit != NULL)
			{
				for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
				{
					if (gsd->unit == game->gameUI->getFirstPerson(i))
					{
						game::GameStats::instances[i]->addDeath();
					}
				}
			} else {
				sp->warning("GameScripting::process - Attempt to statDeath for null unit.");
			}
			break;

		case GS_CMD_statKill:
			if (stringData != NULL)
			{
				if (gsd->shooter != NULL)
				{
					for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
					{
						if (gsd->shooter == game->gameUI->getFirstPerson(i))
						{
							game::GameStats::instances[i]->addKill(stringData);
						}
					}
				} else {
					sp->warning("GameScripting::process - Attempt to statKill for null shooter unit.");
				}
			} else {
				sp->warning("GameScripting::process - statKill parameter missing.");
			}
			break;

		case GS_CMD_statPickup:
			if (stringData != NULL)
			{
				if (gsd->unit != NULL)
				{
					for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
					{
						if (gsd->unit == game->gameUI->getFirstPerson(i))
						{
							game::GameStats::instances[i]->addPickup(stringData);
						}
					}
				} else {
					sp->warning("GameScripting::process - Attempt to statPickup for null unit.");
				}
			} else {
				sp->warning("GameScripting::process - statPickup parameter missing.");
			}
			break;

		case GS_CMD_statMarker:
			if (stringData != NULL)
			{
				for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
				{
					if (game->gameUI->getFirstPerson(i) != NULL)
					{
						game::GameStats::instances[i]->addMarker(stringData);
					}
				}
			} else {
				sp->warning("GameScripting::process - statPickup parameter missing.");
			}
			break;

		case GS_CMD_deleteAllProjectiles:
			{
				LinkedList *projlist = game->projectiles->getAllProjectiles();
				SafeLinkedListIterator projIter = SafeLinkedListIterator(projlist);
				while (projIter.iterateAvailable())
				{
					Projectile *p = (Projectile *)projIter.iterateNext();
					game->projectiles->removeProjectile(p);
					delete p;
				}
			}
			break;

		case GS_CMD_disablePathfindPortals:
			if (game->getGameScene() != NULL)
			{
				game->getGameScene()->getPathFinder()->disablePortalRoutes(true);
			}
			break;

		case GS_CMD_enablePathfindPortals:
			if (game->getGameScene() != NULL)
			{
				game->getGameScene()->getPathFinder()->disablePortalRoutes(false);
			}
			break;

		case GS_CMD_localizeSubtitleStringValue:
			if (gsd->stringValue != NULL)
			{
				const char *localized = convertLocaleSubtitleString(gsd->stringValue);
				gsd->setStringValue(localized);
			} else {
				sp->error("GameScripting::process - Attempt to localizeSubtitleStringValue for null string.");
			}
			break;

		case GS_CMD_localizeGUIStringValue:
			if (gsd->stringValue != NULL)
			{
				const char *localized = convertLocaleGuiString(gsd->stringValue);
				gsd->setStringValue(localized);
			} else {
				sp->error("GameScripting::process - Attempt to localizeGUIStringValue with null string.");
			}
			break;

		case GS_CMD_localizeSpeechStringValue:
			if (gsd->stringValue != NULL)
			{
				const char *localized = convertLocaleSpeechString(gsd->stringValue);
				gsd->setStringValue(localized);
			} else {
				sp->error("GameScripting::process - Attempt to localizeSpeechStringValue with null string.");
			}
			break;

		// oops, this was already in StringScripting...
		//case GS_CMD_printStringValue:
		//	Logger::getInstance()->info(gsd->stringValue);
		//	break;

		case GS_CMD_printValueToConsole:
			if (game->gameUI->getErrorWindow() != NULL)
			{
				game->gameUI->getErrorWindow()->logMessage(int2str(*lastValue), LOGGER_LEVEL_INFO);
			} else {
				sp->warning("GameScripting::process - Attempt to printValueToConsole when no error window exists.");
			}
			break;

		case GS_CMD_printStringValueToConsole:
			if (game->gameUI->getErrorWindow() != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					game->gameUI->getErrorWindow()->logMessage(gsd->stringValue, LOGGER_LEVEL_INFO);
				} else {
					game->gameUI->getErrorWindow()->logMessage("(null)", LOGGER_LEVEL_INFO);
				}
			} else {
				sp->warning("GameScripting::process - Attempt to printStringValueToConsole when no error window exists.");
			}
			break;

		case GS_CMD_chatConnect:
			//chat::SGScriptChatter::getInstance(game)->connect();
			break;

		case GS_CMD_chatSend:
			/*if (stringData != NULL)
			{
				char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;
				if (s == NULL)
				{
					sp->error("GameScripting::process - chatSend, null string value.");
				} else {
					chat::SGScriptChatter::getInstance(game)->sendMessage(std::string(s));
				}
			} else {
				sp->error("GameScripting::process - chatSend parameter missing.");
			}*/
			break;

		case GS_CMD_chatSendMessage:
			/*if (stringData != NULL)
			{
				char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;
				if (s == NULL)
				{
					sp->error("GameScripting::process - chatSend, null string value.");
				} else {
					std::string tmp = std::string("{SGC_S}message ") + s + std::string("{SGC_E}");
					chat::SGScriptChatter::getInstance(game)->sendMessage(tmp);
				}
			} else {
				sp->error("GameScripting::process - chatSend parameter missing.");
			}*/
			break;

		case GS_CMD_chatReceive:
			//chat::SGScriptChatter::getInstance(game)->receiveMessages();
			break;

		case GS_CMD_consoleMiniQuery:
			if (stringData != NULL)
			{
				game->gameUI->showConsole();
				game->gameUI->getConsole()->setMiniQueryMode();
				game->gameUI->getConsole()->setLine(stringData);
			} else {
				sp->error("GameScripting::process - consoleMiniQuery parameter missing.");
			}
			break;

		case GS_CMD_isScriptLoaded:
			if (stringData != NULL)
			{
				util::Script *s = util::ScriptManager::getInstance()->getScript(stringData);
				if (s != NULL)
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->error("GameScripting::process - isScriptLoaded parameter missing.");
			}
			break;

		case GS_CMD_resetRenderer:
			if(game->gameUI->getStorm3D())
				game->gameUI->getStorm3D()->forceReset();
			break;

		case GS_CMD_saveGridOcclusionCulling:
			if (stringData != NULL)
			{
				if (game->gameUI->getGridOcclusionCuller() != NULL)
				{
					game->gameUI->getGridOcclusionCuller()->save(stringData);
				} else {
					sp->error("MiscScripting::process - saveGridOcclusionCulling, no grid occlusion culler available (called at menus?).");
				}
			} else {
				sp->error("MiscScripting::process - saveGridOcclusionCulling, grid occlusion filename parameter expected.");
			}
			break;

		case GS_CMD_loadGridOcclusionCulling:
			if (stringData != NULL)
			{
				if (game->gameUI->getGridOcclusionCuller() != NULL)
				{
					game->gameUI->getGridOcclusionCuller()->load(stringData);
				} else {
					sp->error("MiscScripting::process - loadGridOcclusionCulling, no grid occlusion culler available (called at menus?).");
				}
			} else {
				sp->error("MiscScripting::process - loadGridOcclusionCulling, grid occlusion filename parameter expected.");
			}
			break;

		case GS_CMD_applyGridOcclusionCulling:
			if (game->gameUI->getGridOcclusionCuller() != NULL
				&& game->gameUI->getTerrain() != NULL)
			{
				VC3 camPos = game->gameUI->getOcclusionCheckPosition();
				int cameraArea = GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK;
				if (game->gameUI->getGridOcclusionCuller()->isWellInScaledBoundaries(camPos.x, camPos.z))
				{
					int occx = game->gameUI->getGridOcclusionCuller()->scaledToOcclusionX(camPos.x);
					int occy = game->gameUI->getGridOcclusionCuller()->scaledToOcclusionY(camPos.z);
					cameraArea = game->gameUI->getGridOcclusionCuller()->getCameraArea(occx, occy);
				} else {
					sp->warning("MiscScripting::process - applyGridOcclusionCulling, camera outside occlusion culling area (using default area).");
				}
				game->gameUI->getTerrain()->updateOcclusionForAllObjects(game->gameUI->getGridOcclusionCuller(), cameraArea);
			} else {
				sp->error("MiscScripting::process - applyGridOcclusionCulling, no grid occlusion culler or terrain available (called at menus?).");
			}
			break;

		case GS_CMD_previewGridOcclusionCullingByValue:
			if (game->gameUI->getGridOcclusionCuller() != NULL)
			{
				if (*lastValue >= 0 && *lastValue < GRIDOCCLUSIONCULLER_MAX_AREAS)
				{
					int cameraArea = game->gameUI->getGridOcclusionCuller()->getAreaForOrderNumber(*lastValue);
					game->gameUI->getTerrain()->updateOcclusionForAllObjects(game->gameUI->getGridOcclusionCuller(), cameraArea);
				} else {
					sp->error("MiscScripting::process - previewGridOcclusionCullingByValue, value out of range.");
				}
			} else {
				sp->error("MiscScripting::process - previewGridOcclusionCullingByValue, no grid occlusion culler available (called at menus?).");
			}
			break;

		case GS_CMD_spawnRandomAt:
			if (stringData != NULL)
			{
				game->alienSpawner->spawnRandomAt(stringData);
			} else {
				sp->warning("GameScripting::process - spawnRandomAt parameter missing.");
			}
			break;

		case GS_CMD_openCombatSubWindow:
			if( stringData != NULL && game->gameUI->getCombatWindow( gsd->player ) )
			{
				game->gameUI->getCombatWindow( gsd->player )->openSubWindow( stringData, gsd->player );
			}
			break;
		
		case GS_CMD_closeCombatSubWindow:
			if( stringData != NULL && game->gameUI->getCombatWindow( gsd->player ) )
			{
				game->gameUI->getCombatWindow( gsd->player )->closeSubWindow( stringData );
			}
			break;

		case GS_CMD_setElaborateHintStyle:
			if( game && game->gameUI && game->gameUI->getCombatWindow( gsd->player ) &&
				game->gameUI->getCombatWindow( gsd->player )->getSubWindow( "ElaborateHintMessageWindow" ) &&
				stringData != NULL )
			{
				((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( gsd->player )->getSubWindow( "ElaborateHintMessageWindow" ))->setStyle( stringData );
			}
			else
			{
				Logger::getInstance()->warning( "MiscScripting::process - setElaborateHintStyle failed, probably because there's no ElaborateHintMessageWindow" );
			}

			break;

		case GS_CMD_elaborateHint:
			if( game && game->gameUI && game->gameUI->getCombatWindow( gsd->player ) &&
				game->gameUI->getCombatWindow( gsd->player )->getSubWindow( "ElaborateHintMessageWindow" ) &&
				stringData != NULL )
			{
				char *usestr;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					usestr = gsd->stringValue;
				else
					usestr = stringData;

				// NEW: messages clear any previous messages!
				// game->gameUI->clearGameMessage(GameUI::MESSAGE_TYPE_NORMAL);

				// game->gameUI->gameMessage(convertLocaleSubtitleString(usestr), NULL, 
				// 	1, 3000 + strlen(usestr) * 40, GameUI::MESSAGE_TYPE_NORMAL);
				// } else {
				//	sp->error("GameScripting::process - message parameter missing.");
				// }

				((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( gsd->player )->getSubWindow( "ElaborateHintMessageWindow" ))->showMessage( convertLocaleSubtitleString(usestr) );
			}
			else
			{
				Logger::getInstance()->warning( "MiscScripting::process - elaborateHint failed, probably because there's no ElaborateHintMessageWindow" );
			}


			break;


#ifdef USE_CLAW_CONTROLLER
		case GS_CMD_reloadClawSettings:
			if(game && game->getClawController())
				game->getClawController()->reloadConfig();
			break;

		case GS_CMD_enableClaw:
			game->getClawController()->setClawEnabled(true);			
			break;

		case GS_CMD_disableClaw:
			game->getClawController()->setClawEnabled(false);
			break;

		case GS_CMD_setClawToPosition:
			game->getClawController()->setTargetPosition(gsd->position);			
			break;

		case GS_CMD_getClawPosition:
			gsd->position = game->getClawController()->getClawPosition();
			break;

		case GS_CMD_clawHasObject:
			*lastValue = game->getClawController()->hasActor() ? 1 : 0;
			break;
#endif

		case GS_CMD_isUnifiedHandleValid:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleTerrainObject:
			if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleUnit:
			if (IS_UNIFIED_HANDLE_UNIT(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleTracker:
			if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleLight:
			if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleAmbientSound:
			if (IS_UNIFIED_HANDLE_AMBIENT_SOUND(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleItem:
			if (IS_UNIFIED_HANDLE_ITEM(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_isUnifiedHandleParticleSpawner:
			if (IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(gsd->unifiedHandle))
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_doesUnifiedHandleObjectExist:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
			} else {
				sp->error("MiscScripting::process - doesUnifiedHandleObjectExist, invalid unified handle.");
			}
			break;

		case GS_CMD_getUnifiedHandleObjectPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					gsd->position = game->unifiedHandleManager->getObjectPosition(gsd->unifiedHandle);
				} else {
					sp->error("MiscScripting::process - getUnifiedHandleObjectPosition, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - getUnifiedHandleObjectPosition, invalid unified handle.");
			}
			break;

		case GS_CMD_setUnifiedHandleObjectPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					game->unifiedHandleManager->setObjectPosition(gsd->unifiedHandle, gsd->position);
				} else {
					sp->error("MiscScripting::process - setUnifiedHandleObjectPosition, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - setUnifiedHandleObjectPosition, invalid unified handle.");
			}
			break;

		case GS_CMD_getUnifiedHandleObjectRotation:
			assert(!"TODO - needs quaternion support to scripts... =/");
			break;

		case GS_CMD_setUnifiedHandleObjectRotation:
			assert(!"TODO - needs quaternion support to scripts... =/");
			break;

		case GS_CMD_getUnifiedHandleObjectVelocity:
			assert(!"TODO");
			break;

		case GS_CMD_setUnifiedHandleObjectVelocity:
			assert(!"TODO");
			break;

		case GS_CMD_getUnifiedHandleObjectCenterPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					gsd->position = game->unifiedHandleManager->getObjectCenterPosition(gsd->unifiedHandle);
				} else {
					sp->error("MiscScripting::process - getUnifiedHandleObjectCenterPosition, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - getUnifiedHandleObjectCenterPosition, invalid unified handle.");
			}
			break;

		case GS_CMD_setUnifiedHandleObjectCenterPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					game->unifiedHandleManager->setObjectCenterPosition(gsd->unifiedHandle, gsd->position);
				} else {
					sp->error("MiscScripting::process - setUnifiedHandleObjectCenterPosition, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - setUnifiedHandleObjectCenterPosition, invalid unified handle.");
			}
			break;

		case GS_CMD_getUnifiedHandleObjectDistanceToPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					VC3 pos = game->unifiedHandleManager->getObjectPosition(gsd->unifiedHandle);
					VC3 diffVec = pos - gsd->position;
					*lastValue = (int)diffVec.GetLength();
				} else {
					sp->error("MiscScripting::process - getUnifiedHandleObjectDistanceToPosition, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - getUnifiedHandleObjectDistanceToPosition, invalid unified handle.");
			}
			break;			

		case GS_CMD_getUnifiedHandleObjectDistanceToPositionFloat:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
					VC3 pos = game->unifiedHandleManager->getObjectPosition(gsd->unifiedHandle);
					VC3 diffVec = pos - gsd->position;
					gsd->floatValue = diffVec.GetLength();
				} else {
					sp->error("MiscScripting::process - getUnifiedHandleObjectDistanceToPositionFloat, object does not exist with given unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - getUnifiedHandleObjectDistanceToPositionFloat, invalid unified handle.");
			}
			break;			

		case GS_CMD_unifiedHandleToValue:
			*lastValue = gsd->unifiedHandle;
			break;

		case GS_CMD_valueToUnifiedHandle:
			gsd->unifiedHandle = *lastValue;
			break;			

		case GS_CMD_savegameStats:
			if (stringData != NULL)
			{
				savegame_stats = stringData;
			} else {
				sp->warning("GameScripting::process - saveGameStats parameter missing, expected string.");
			}
			break;

		case GS_CMD_statMarkTimeOfDeath:
			{
				for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
				{
					if (gsd->unit == game->gameUI->getFirstPerson(i))
					{
						game::GameStats::instances[i]->markTimeOfDeath();
					}
				}
			}
			break;

		case GS_CMD_getUnitPlayerNumber:
			if (gsd->unit != NULL)
			{
				*lastValue = -1;
				for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
				{
					if (gsd->unit == game->gameUI->getFirstPerson(i))
					{
						*lastValue = i;
						break;
					}
				}
				if (*lastValue == -1)
				{
					sp->warning("MiscScripting::process - getUnitPlayerNumber, unit is not human player controlled.");
				}
			} else {
				sp->warning("MiscScripting::process - Attempt to getUnitPlayerNumber for null unit.");
			}
			break;

		case GS_CMD_resetWorldFold:
			GameWorldFold::getInstance()->reset();
			break;

		case GS_CMD_addWorldFoldToPosition:
			{
				float floatData = *((float *)&intData);
				VC3 tmp = gsd->position;
				// Where the heck does this offset come from?
				//tmp.x -= 0.4475f;
				*lastValue = GameWorldFold::getInstance()->addFold(tmp, floatData);
			}
			break;

		case GS_CMD_getWorldFoldNumberAtPosition:
			{
				*lastValue = GameWorldFold::getInstance()->getFoldNumberAtPosition(gsd->position);
				if (*lastValue == 0)
				{
					sp->warning("MiscScripting::process - getWorldFoldNumberAtPosition, no fold exists at given position.");
				}
			}
			break;

		case GS_CMD_moveWorldFoldToPosition:
			if (*lastValue > 0) 
			{
				GameWorldFold::getInstance()->moveFold(*lastValue, gsd->position);
			} else {
				sp->warning("MiscScripting::process - moveWorldFoldToPosition, invalid world fold number.");
			}
			break;

		case GS_CMD_setWorldFoldAngle:
			if (*lastValue > 0) 
			{
				float floatData = *((float *)&intData);
				GameWorldFold::getInstance()->setFoldAngle(*lastValue, floatData);
			} else {
				sp->warning("MiscScripting::process - setWorldFoldAngle, invalid world fold number.");
			}
			break;

		case GS_CMD_getShooter:
			gsd->unit = gsd->shooter;
			break;

		case GS_CMD_getEngineMetaValueInt:
			if (stringData != NULL)
			{
				if (!EngineMetaValues::doesMetaValueExist(stringData))
				{
					//EngineMetaValues::setMetaValueInt(stringData, 0);
					sp->warning("MiscScripting::process - getEngineMetaValueInt, requested meta value did not exist (define it before use to remove this warning).");
				} else {
					if (!EngineMetaValues::isMetaValueTypeInt(stringData))
					{
						sp->warning("MiscScripting::process - getEngineMetaValueInt, requested meta value is not int type (should use a meta key as single type only).");
					}
				}
				*lastValue = EngineMetaValues::getMetaValueInt(stringData);
			} else {
				sp->error("MiscScripting::process - getEngineMetaValueInt, parameter missing.");
			}
			break;

		case GS_CMD_setEngineMetaValueInt:
			if (stringData != NULL)
			{
				if (!EngineMetaValues::doesMetaValueExist(stringData))
				{
					sp->warning("MiscScripting::process - setEngineMetaValueInt, requested meta value did not exist (define it before use to remove this warning).");
				}
				if (!EngineMetaValues::isMetaValueTypeInt(stringData))
				{
					sp->warning("MiscScripting::process - getEngineMetaValueInt, requested meta value is not int type (should use a meta key as single type only).");
				}
				EngineMetaValues::setMetaValueInt(stringData, *lastValue);
			} else {
				sp->error("MiscScripting::process - getEngineMetaValueInt, parameter missing.");
			}
			break;

		case GS_CMD_defineEngineMetaValueInt:
			if (stringData != NULL)
			{
				if (!EngineMetaValues::doesMetaValueExist(stringData))
				{
					EngineMetaValues::setMetaValueInt(stringData, 0);
				} else {
					if (!EngineMetaValues::isMetaValueTypeInt(stringData))
					{
						sp->warning("MiscScripting::process - defineEngineMetaValueInt, requested meta value already exists with different type (should use a meta key as single type only).");
					}
				}
			} else {
				sp->error("MiscScripting::process - defineEngineMetaValueInt, parameter missing.");
			}
			break;

		case GS_CMD_getControllerSceneSelectionUnifiedHandle:
			if (*lastValue >= 0 && *lastValue < MAX_PLAYERS_PER_CLIENT)
			{
				SceneSelection *s = game->gameUI->getSceneSelection(*lastValue);
				if (s->unit != NULL)
				{
					gsd->unifiedHandle = game->units->getUnifiedHandle(game->units->getIdForUnit(s->unit));
					*lastValue = 1;
				}
#ifdef LEGACY_FILES
				// no support for unified handles.
#else
				else if (s->unifiedHandle != UNIFIED_HANDLE_NONE)
				{
					gsd->unifiedHandle = s->unifiedHandle;
					*lastValue = 1;
				}
#endif
				else {
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					*lastValue = 0;
				}
			} else {
				sp->error("MiscScripting::process - getControllerSceneSelectionUnifiedHandle last value (controller number) out of range.");
				sp->debug(int2str(*lastValue));
				*lastValue = 0;
			}
			break;

		case GS_CMD_isRunningScriptProcessForUnit:
			if (gsd->unit != NULL)
			{
				if(stringData != NULL)
				{
					*lastValue = game->isRunningCustomScriptProcess(stringData, gsd->unit) ? 1 : 0;
				}
				else
				{
					*lastValue = 0;
					sp->error("MiscScripting::process - isRunningScriptProcessForUnit missing parameter");
				}
			}
			else
			{
				*lastValue = 0;
				sp->error("MiscScripting::process - isRunningScriptProcessForUnit for null unit");
			}
			break;

		case GS_CMD_isUnitPlayerControlled:
			{
				*lastValue = 0;
				if (gsd->unit != NULL)
				{
					for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
					{
						if (gsd->unit == game->gameUI->getFirstPerson(i))
						{
							*lastValue = 1;
							break;
						}
					}
				}
			}
			break;

		case GS_CMD_openCombatSubWindowWithText:
			{
#ifdef PROJECT_SURVIVOR
				if( stringData != NULL && game->gameUI->getCombatWindow( gsd->player ) )
				{
					game->gameUI->getCombatWindow( gsd->player )->openSubWindow( stringData, gsd->player );
					GenericTextWindow *textw = GenericTextWindow::last_opened_window;
					if(textw != NULL)
					{
						if(gsd->stringValue != NULL)
						{
							textw->setText(convertLocaleSubtitleString(gsd->stringValue));
						}
						else
						{
							sp->error("MiscScripting::process - openCombatSubWindowWithText must have string set.");
						}
					}
					else
					{
						sp->error("MiscScripting::process - openCombatSubWindowWithText cannot be used for non-text windows.");
					}
					break;
				}
#endif
				sp->error("MiscScripting::process - openCombatSubWindowWithText not available in this project.");
			}
			break;

		case GS_CMD_doesFileExist:
			{
				frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(stringData, "rb");
				if (f != NULL)
				{
					fb_fclose(f);
					*lastValue = 1;
				}
				else
				{
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_getUnifiedHandleObjectByUniqueEditorObjectHandleString:
		case GS_CMD_getUnifiedHandleObjectByUniqueEditorObjectHandleHex:
			if (stringData != NULL)
			{
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				UniqueEditorObjectHandle ueoh = 0;

				bool paramOk = false;
				if (command == GS_CMD_getUnifiedHandleObjectByUniqueEditorObjectHandleString)
				{
					if (strlen(stringData) <= 8
						&& (strlen(stringData) < 7 || (stringData[7] & 0x80) == 0))
					{
						ueoh = id_string_to_ueoh(stringData);
						paramOk = true;
					} else {
						sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString, parameter invalid (string must be at most 8 characters, using 7 bit chars).");
					}
				}
				if (command == GS_CMD_getUnifiedHandleObjectByUniqueEditorObjectHandleHex)
				{
					if (strlen(stringData) != 16)
					{
						ueoh = (UniqueEditorObjectHandle)hex_to_int64(stringData);
						if (hex_to_int64_errno() == 0 && ueoh != 0)
						{
							paramOk = true;
						}
					}
					if (!paramOk)
					{
						sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleHex, parameter invalid (expected 16 character long hexadecimal string).");
					}
				}

				if (paramOk)
				{
					if (ueoh == 0)
					{
						// this should never happen! (due to the above string validity check)
						sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString/Hex, internal error.");
						assert(!"MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString/Hex, internal error.");
					} else {

						// first seek if it is found in units
						gsd->unifiedHandle = game->units->findUnifiedHandleByUniqueEditorObjectHandle(ueoh);

						// then seek if it is found in lights
						if (gsd->unifiedHandle == UNIFIED_HANDLE_NONE)
						{
							gsd->unifiedHandle = game->gameUI->getLightManager()->findUnifiedHandleByUniqueEditorObjectHandle(ueoh);
						}

						// then seek if it is found in terrain
						if (gsd->unifiedHandle == UNIFIED_HANDLE_NONE && game->gameUI->getTerrain() != NULL)
						{
							gsd->unifiedHandle = game->gameUI->getTerrain()->findUnifiedHandleByUniqueEditorObjectHandle(ueoh);
						}

						// if found in none, show an error message
						if (gsd->unifiedHandle == UNIFIED_HANDLE_NONE)
						{
							sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString/Hex, no object with given UEOH.");
						}
					}
				//} else {
					// this is now handled above...
					//sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString/Hex, parameter invalid.");
					//gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("MiscScripting::process - getUnifiedHandleObjectByUniqueEditorObjectHandleString/Hex, parameter missing.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
			break;

		case GS_CMD_attachAddedParticleSpawnerToUnifiedHandleObject:
			if (gs_lastAddedParticleSpawnerHandle != UNIFIED_HANDLE_NONE)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gs_lastAddedParticleSpawnerHandle))
				{
					if (IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(gs_lastAddedParticleSpawnerHandle))
					{
						if (game->unifiedHandleManager->doesObjectExist(gs_lastAddedParticleSpawnerHandle))
						{
							if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle)
								&& !IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(gsd->unifiedHandle))
							{
								if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
								{
									game->particleSpawnerManager->attachParticleSpawner(gs_lastAddedParticleSpawnerHandle, gsd->unifiedHandle);
								} else {
									sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, object with given unified handle does not exist.");
								}
							} else {
								sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, invalid unified handle or handle points to a particle spawner.");
							}
						} else {
							sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, particle spawner no-longer exists.");
						}
					} else {
						sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, unexpected particlespawner handle type (internal error).");
					}
				} else {
					sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, invalid particlespawner unified handle (internal error).");
				}
			} else {
				sp->error("MiscScripting::process - attachAddedParticleSpawnerToUnifiedHandleObject, must first add a particle spawner.");
			}
			gs_lastAddedParticleSpawnerHandle = UNIFIED_HANDLE_NONE;
			break;

		case GS_CMD_spawnProjectileAttachingToUnifiedHandle:
			if (stringData != NULL)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
						if (pt == NULL) 
						{ 
							sp->error("MiscScripting::process - spawnProjectileAttachingToUnifiedHandle, reference to unloaded part type.");
						} else {
							if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
							{
								// WARNING: unsafe cast!
								Weapon *weap = (Weapon *)pt;
								VC3 projpos = gsd->position;
								game->gameMap->keepWellInScaledBoundaries(&projpos.x, &projpos.z);
								float angle = (float)(*lastValue);
								VC3 dir = VC3(0,0,0);
								dir.x = -sinf(UNIT_ANGLE_TO_RAD(angle));
								dir.z = -cosf(UNIT_ANGLE_TO_RAD(angle));

								Projectile *proj = ParticleSpawner::spawnProjectileWithWeapon(game, weap, projpos, dir, 0.0f, gsd->unifiedHandle);
							} else {
								sp->error("MiscScripting::process - spawnProjectileAttachingToUnifiedHandle, attempt to use non-weapon part type.");
							}
						}
					} else {
						sp->error("MiscScripting::process - spawnProjectileAttachingToUnifiedHandle, object with given unified handle does not exist.");
					}
				} else {
					sp->error("MiscScripting::process - spawnProjectileAttachingToUnifiedHandle, invalid unified handle.");
				}
			} else {
				sp->error("MiscScripting::process - spawnProjectileAttachingToUnifiedHandle, parameter missing.");
			}
			break;

		case GS_CMD_preloadPartTypeModel:
			if (stringData != NULL)
			{
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("MiscScripting::process - preloadPartTypeModel reference to unloaded part type.");
					break;
				}

				if (pt->getVisualObjectModel() == NULL)
				{
					sp->error("MiscScripting::process - preloadPartTypeModel part type has no model.");
					break;
				}

				ui::VisualObject *obj = pt->getVisualObjectModel()->getNewObjectInstance();
				delete obj;

			} else {
				sp->error("MiscScripting::process - preloadPartTypeModel parameter missing.");
			}
			break;

		case GS_CMD_preloadAnimation:
			if (stringData != NULL)
			{
				Animator::preloadAnimation(stringData);
			} else {
				sp->error("MiscScripting::process - preloadAnimation parameter missing.");
			}
			break;

		case GS_CMD_setReflectionEnabled:
			{
				if(SimpleOptions::getBool(DH_OPT_B_RENDER_REFLECTION))
					game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Reflection, intData == 0 ? false : true );
			}
			break;

		default:
			sp->error("MiscScripting::process - Unknown command.");
			assert(0);
		}
	}
}


