
#include "precompiled.h"

#include "ItemScripting.h"

#include "scripting_macros_start.h"
#include "item_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>
#include <IStorm3D.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../Item.h"
#include "../GameMap.h"
#include "../GameScene.h"
#include "../ProgressBar.h"
#include "../GameRandom.h"
#include "../scaledefs.h"
#include "../createparts.h"
#include "../Character.h"
#include "../UnitVisibilityChecker.h"
#include "../UnitLevelAI.h"
#include "../SimpleOptions.h"
#include "../options/options_players.h"
#include "../options/options_physics.h"
#include "../ItemManager.h"
#include "../../ui/GameController.h"

#include "../../system/Logger.h"
#include "../../system/Timer.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../util/TextureCache.h"
#include "../DHLocaleManager.h"
#include "../ParticleSpawner.h"
#include "../ParticleSpawnerManager.h"

#include "../../util/Debug_MemoryManager.h"

#include "GameScripting.h"


using namespace ui;

namespace game
{
	Item *itemScriptItem = NULL;
	char *itemSpecialString = NULL;
	char *itemCustomTipText = NULL;
	char *itemCustomHighlightText = NULL;

	void ItemScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_REMOVEITEM:
			// once the script process ends, gamescripting will check this
			// variable to decide whether the initiating item got removed.
			game->gameScripting->itemMarkedForRemove = true;
			break;

		case GS_CMD_DISABLEITEM:
			if (intData >= 0)
			{
				if (intData == 0)
				{
					sp->debug("GameScripting::process - disableItem with zero time parameter, thus item will never re-enable.");
				}
				game->gameScripting->itemMarkedForDisable = true;
				int val = intData;
				if (val > 999999) val = 999999;
				game->gameScripting->itemMarkedForDisableTime = val * GAME_TICKS_PER_SECOND;
			} else {
				sp->error("GameScripting::process - disableItem value out of range, non-negative disable time expected.");
			}
			break;

		case GS_CMD_ADDITEM:
			if (stringData != NULL)
			{
				VC3 itempos = gsd->position;
				game->gameMap->keepWellInScaledBoundaries(&itempos.x, &itempos.z);
				// NOTE: new behaviour, item position no longer moved on
				// top of heightmap, height taken from gsd->position!
				//itempos.y = game->gameMap->getScaledHeightAt(itempos.x, itempos.z);
				//int px = game->gameMap->scaledToPathfindX(itempos.x);
				//int py = game->gameMap->scaledToPathfindY(itempos.z);
				// HACK: raise always!
				//if (game->getGameScene()->getBuildingModelAtPathfind(px, py) != NULL)
				//{
					// raise a bit inside a building (5 cm)
					//itempos.y += 0.05f;
				//}
				Item *item = game->itemManager->createNewItem(ItemManager::getItemIdByName(stringData), itempos);
				if (item != NULL)
				{
					VC3 rotation = item->getRotation();
					rotation += VC3(0, (float)(*lastValue), 0);
					item->setRotation(rotation);
					item->setCustomScript(gsd->stringValue);
					item->setSpecialString(itemSpecialString);
					item->setCustomTipText(itemCustomTipText);

#ifndef PHYSICS_NONE
					if( game::SimpleOptions::getBool( DH_OPT_B_PHYSICS_ENABLED ) ) 
					{
						game->itemManager->enablePhysics( item, ItemManager::getItemIdByName(stringData) );
					}
#endif

					if (itemCustomHighlightText != NULL)
					{
						item->setHighlightText(itemCustomHighlightText);
					}
					if (itemSpecialString != NULL)
					{
						delete [] itemSpecialString;
						itemSpecialString = NULL;
					}
					if (itemCustomTipText != NULL)
					{
						delete [] itemCustomTipText;
						itemCustomTipText = NULL;
					}
					if (itemCustomHighlightText != NULL)
					{
						delete [] itemCustomHighlightText;
						itemCustomHighlightText = NULL;
					}
				}
			} else {
				sp->error("ItemScripting::process - AddItem parameter missing, item type expected.");
			}
			break;

		case GS_CMD_setItemSpecialString:
			if (stringData != NULL)
			{
				if (itemSpecialString != NULL)
				{
					delete [] itemSpecialString;
					itemSpecialString = NULL;
				}
				itemSpecialString = new char[strlen(stringData) + 1];
				strcpy(itemSpecialString, stringData);
			} else {
				sp->error("ItemScripting::process - setItemSpecialString parameter missing.");
			}
			break;

		case GS_CMD_setItemSpecialStringNull:
			if (itemSpecialString != NULL)
			{
				delete [] itemSpecialString;
				itemSpecialString = NULL;
			}
			break;

		case GS_CMD_getItemSpecialString:
			if (itemScriptItem != NULL)
			{
				gsd->setStringValue(itemScriptItem->getSpecialString());
			} else {
				sp->error("ItemScripting::process - Attempt to getItemSpecialString outside item script.");
			}
			break;

		case GS_CMD_setItemCustomTipText:
			if (stringData != NULL)
			{
				if (itemCustomTipText != NULL)
				{
					delete [] itemCustomTipText;
					itemCustomTipText = NULL;
				}
				itemCustomTipText = new char[strlen(stringData) + 1];
				strcpy(itemCustomTipText, stringData);
			} else {
				sp->error("ItemScripting::process - setItemCustomTipText parameter missing.");
			}
			break;

		case GS_CMD_setItemCustomTipTextNull:
			if (itemCustomTipText != NULL)
			{
				delete [] itemCustomTipText;
				itemCustomTipText = NULL;
			}
			break;

		case GS_CMD_getItemCustomTipText:
			if (itemScriptItem != NULL)
			{
				gsd->setStringValue(itemScriptItem->getCustomTipText());
			} else {
				sp->error("ItemScripting::process - Attempt to getItemCustomTipText outside item script.");
			}
			break;

		case GS_CMD_setItemCustomHighlightText:
			if (stringData != NULL)
			{
				if (itemCustomHighlightText != NULL)
				{
					delete [] itemCustomHighlightText;
					itemCustomHighlightText = NULL;
				}
				itemCustomHighlightText = new char[strlen(stringData) + 1];
				strcpy(itemCustomHighlightText, stringData);
			} else {
				sp->error("ItemScripting::process - setItemCustomHighlightText parameter missing.");
			}
			break;

		case GS_CMD_setItemCustomHighlightTextNull:
			if (itemCustomHighlightText != NULL)
			{
				delete [] itemCustomHighlightText;
				itemCustomHighlightText = NULL;
			}
			break;

		case GS_CMD_REMOVECLOSESTITEMOFTYPE:
			if (stringData != NULL)
			{
				int itemId = game->itemManager->getItemIdByName(stringData);
				if (itemId != -1)
				{
					ItemType *itype = game->itemManager->getItemTypeById(itemId);
					assert(itype != NULL);
					Item *i = game->itemManager->getNearestItemOfType(gsd->position, itype);
					if (i != NULL)
					{
						game->itemManager->deleteItem(i);
					} else {
						sp->warning("ItemScripting::process - removeClosestItemOfType, no item of given type found for deletion.");
					}
				} else {
					sp->error("ItemScripting::process - removeClosestItemOfType, item type with given name not found.");
				}
			} else {
				sp->error("ItemScripting::process - removeClosestItemOfType parameter missing, item type expected.");
			}
			break;

		case GS_CMD_DISABLECLOSESTITEMOFTYPE:
			if (stringData != NULL)
			{
				int itemId = game->itemManager->getItemIdByName(stringData);
				if (itemId != -1)
				{
					ItemType *itype = game->itemManager->getItemTypeById(itemId);
					assert(itype != NULL);
					Item *i = game->itemManager->getNearestItemOfType(gsd->position, itype);
					if (i != NULL)
					{
						// NOTE: always disables the item for good, it won't re-enable.
						// TODO: need to be able to somehow set the disable time...
						game->itemManager->disableItem(i, 0);
					} else {
						sp->warning("ItemScripting::process - disableClosestItemOfType, no item of given type found for deletion.");
					}
				} else {
					sp->error("ItemScripting::process - disableClosestItemOfType, item type with given name not found.");
				}
			} else {
				sp->error("ItemScripting::process - disableClosestItemOfType parameter missing, item type expected.");
			}
			break;

		case GS_CMD_STOPBLINKINGFORCLOSESTITEMOFTYPE:
		case GS_CMD_STARTBLINKINGFORCLOSESTITEMOFTYPE:
			if (stringData != NULL)
			{
				int itemId = game->itemManager->getItemIdByName(stringData);
				if (itemId != -1)
				{
					ItemType *itype = game->itemManager->getItemTypeById(itemId);
					assert(itype != NULL);
					Item *i = game->itemManager->getNearestItemOfType(gsd->position, itype);
					if (i != NULL)
					{
						if (command == GS_CMD_STARTBLINKINGFORCLOSESTITEMOFTYPE)
							i->setBlinking(true);
						else
							i->setBlinking(false);
					} else {
						sp->warning("ItemScripting::process - stop/startBlinkingForClosestItemOfType, no item of given type found.");
					}
				} else {
					sp->error("ItemScripting::process - stop/startBlinkingForClosestItemOfType, item type with given name not found.");
				}
			} else {
				sp->error("ItemScripting::process - stop/startBlinkingForClosestItemOfType parameter missing, item type expected.");
			}
			break;

		case GS_CMD_SETPROGRESSLABEL:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (stringData != NULL)
					{
						itemScriptItem->getProgressBar()->setLabel(convertLocaleSubtitleString(stringData));
					} else {
						sp->error("ItemScripting::process - setProgressLabel parameter missing, label text expected.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressLabel for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressLabel outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSTOTALTIME:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (intData / GAME_TICK_MSEC > 0)
					{
						itemScriptItem->getProgressBar()->setTotalTime(intData / GAME_TICK_MSEC);
					} else {
						sp->error("ItemScripting::process - setProgressTotalTime parameter invalid, progress total time expected (msec).");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressTotalTime for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressTotalTime outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSTICKTIME:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (intData / GAME_TICK_MSEC > 0)
					{
						itemScriptItem->getProgressBar()->setTickTime(intData / GAME_TICK_MSEC);
					} else {
						sp->error("ItemScripting::process - setProgressTickTime parameter invalid, progress tick interval expected (msec).");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressTickTime for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressTickTime outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSDONELABEL:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (stringData != NULL)
					{
						itemScriptItem->getProgressBar()->setDoneLabel(convertLocaleSubtitleString(stringData));
					} else {
						sp->error("ItemScripting::process - setProgressDoneLabel parameter missing, label text expected.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressDoneLabel for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressDoneLabel outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSINTERRUPTEDLABEL:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (stringData != NULL)
					{
						itemScriptItem->getProgressBar()->setInterruptedLabel(convertLocaleSubtitleString(stringData));
					} else {
						sp->error("ItemScripting::process - setProgressInterruptedLabel parameter missing, label text expected.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressInterruptedLabel for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressInterruptedLabel outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSBARIMAGE:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (stringData != NULL)
					{
						itemScriptItem->getProgressBar()->setBarImage(convertLocaleSubtitleString(stringData));
					} else {
						sp->error("ItemScripting::process - setProgressBarImage parameter missing, image filename expected.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressBarImage for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressBarImage outside item script.");
			}
			break;

		case GS_CMD_SETPROGRESSBARBORDERIMAGE:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					if (stringData != NULL)
					{
						itemScriptItem->getProgressBar()->setBorderImage(convertLocaleSubtitleString(stringData));
					} else {
						sp->error("ItemScripting::process - setProgressBarBorderImage parameter missing, image filename expected.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressBorderImage for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressBorderImage outside item script.");
			}
			break;

		case GS_CMD_GETPROGRESSDONETIME:			
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					*lastValue = itemScriptItem->getProgressBar()->getProgressDone() * GAME_TICK_MSEC;
				} else {
					sp->error("ItemScripting::process - Attempt to getProgressDoneTime for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to getProgressDoneTime outside item script.");
			}
			break;

		case GS_CMD_STARTPROGRESS:
			if (itemScriptItem != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (itemScriptItem->getProgressBar() == NULL)
					{
						itemScriptItem->createProgressBar();
					}
					if (!itemScriptItem->getProgressBar()->isProgressing())
					{
						itemScriptItem->getProgressBar()->startProgress(gsd->unit, gsd->unit->getPosition(), gsd->unit->getRotation().y);
					} else {
						sp->debug("ItemScripting::process - Attempt to startProgress for item with another progress running.");
					}
				} else {
					sp->error("ItemScripting::process - Attempt to startProgress for null unit.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to startProgress outside item script.");
			}
			break;

		case GS_CMD_STOPPROGRESS:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					itemScriptItem->getProgressBar()->stopProgress();
				} else {
					sp->warning("ItemScripting::process - Attempt to stopProgress for item with no progress running.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to stopProgress outside item script.");
			}
			break;

		case GS_CMD_INTERRUPTPROGRESS:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					itemScriptItem->getProgressBar()->interruptProgress();
				} else {
					sp->warning("ItemScripting::process - Attempt to interruptProgress for item with no progress running.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to interruptProgress outside item script.");
			}
			break;

		case GS_CMD_CHANGEITEMMODEL:
			if (itemScriptItem != NULL)
			{
				if (stringData != NULL)
				{
					game->itemManager->changeItemVisual(itemScriptItem, stringData);
				} else {
					sp->error("ItemScripting::process - changeItemModel parameter missing, model filename expected.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to changeItemModel outside item script.");
			}
			break;

		case GS_CMD_CHANGEITEMMODELFORCLOSESTITEMOFTYPETOSTRINGVALUE:
			if (stringData != NULL)
			{
				int itemId = game->itemManager->getItemIdByName(stringData);
				if (itemId != -1)
				{
					ItemType *itype = game->itemManager->getItemTypeById(itemId);
					assert(itype != NULL);
					Item *i = game->itemManager->getNearestItemOfType(gsd->position, itype);
					if (i != NULL)
					{
						game->itemManager->changeItemVisual(i, gsd->stringValue);
					}
				}
			} else {
				sp->error("ItemScripting::process - changeItemModelForClosestItemOfTypeToStringValue parameter missing, item type name expected.");
			}
			break;

		case GS_CMD_DELETELONGTIMEDISABLEDITEMS:
			game->itemManager->deleteAllLongTimeDisabledItems();
			break;

		case GS_CMD_REENABLEALLTIMEDISABLEDITEMS:
			game->itemManager->reEnableAllTimeDisabledItems();
			break;

		case GS_CMD_setItemHighlightStyle:
			if (itemScriptItem != NULL)
			{
				itemScriptItem->setHighlightStyle( intData );
			} else {
				sp->error("ItemScripting::process - Attempt to setItemHighlightStyle outside item script.");
			}
			break;

		case GS_CMD_setItemHighlightText:
			if (itemScriptItem != NULL)
			{
				if(stringData != NULL)
					itemScriptItem->setHighlightText( stringData );
				else
					itemScriptItem->setHighlightText( "" );
			} else {
				sp->error("ItemScripting::process - Attempt to setItemHighlightText outside item script.");
			}
			break;
		
		case GS_CMD_setItemHighlightTextBySpecialString:
			if (itemScriptItem != NULL)
			{
				itemScriptItem->setHighlightText( itemScriptItem->getSpecialString()?itemScriptItem->getSpecialString():NULL );
			} else {
				sp->error("ItemScripting::process - Attempt to setItemHighlightText outside item script.");
			}
			break;
		
		case GS_CMD_openTerminalWindow:
			if( game != NULL )
			{
				if( stringData != NULL )
				{
					game->gameUI->openTerminalWindow( stringData );
				} else {
					sp->error("ItemScripting::process - openTerminalWindow parameter missing.");
				}
			}
			break;

		case GS_CMD_closeTerminalWindow:
			if( game != NULL )
			{
				game->gameUI->closeTerminalWindow();
			}
			break;

		case GS_CMD_openTerminalWindowByItemSpecialString:
			if (itemScriptItem != NULL)
			{
				if (game != NULL)
				{
					if (itemScriptItem->getSpecialString() != NULL)
					{
						game->gameUI->openTerminalWindow(itemScriptItem->getSpecialString());
					} else {
						sp->error("ItemScripting::process - Attempt to openTerminalWindowByItemSpecialString for item with null special string.");
					}
				}
			} else {
				sp->error("ItemScripting::process - Attempt to openTerminalWindowByItemSpecialString outside item script.");
			}
			break;

		case GS_CMD_openTerminalWindowByStringValue:
			if (game != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					game->gameUI->openTerminalWindow(gsd->stringValue);
				} else {
					sp->error("ItemScripting::process - Attempt to openTerminalWindowByStringValue with null string value.");
				}
			}
			break;

		case GS_CMD_setProgressInterruptPercent:
			if (itemScriptItem != NULL)
			{
				if (itemScriptItem->getProgressBar() != NULL)
				{
					itemScriptItem->getProgressBar()->setInterruptPercent( intData );
				} else {
					sp->error("ItemScripting::process - Attempt to setProgressInterruptPercent for item with no progress started.");
				}
			} else {
				sp->error("ItemScripting::process - Attempt to setProgressInterruptPercent outside item script.");
			}
			break;

		case GS_CMD_createItemSpawner:
			if (stringData != NULL)
			{
				const char *parse_error = "ItemScripting::process - createItemSpawner expects format \"groupname,radius:itemname,respawntime,itemname,respawntime...\".";
				VC3 pos = gsd->position;
				std::string str = stringData;
				std::string::size_type i;

				// get group name
				//
				i = str.find(',');
				if(i == std::string::npos)
				{
					sp->error(parse_error);
					break;
				}
				std::string group_name = str.substr(0, i);
				i++;

				// get radius
				//
				std::string::size_type j = str.find(':', i);
				if(j == std::string::npos)
				{
					sp->error(parse_error);
					break;
				}
				std::string radius_str = str.substr(i, j - i);
				i = j;

				float radius = 0.0f;
				if(sscanf(radius_str.c_str(), "%f", &radius) != 1)
				{
					sp->error(parse_error);
					break;
				}

				while(true)
				{
					i++;
					// no spawns at all (dummy command from editor)
					if(i == str.size())
					{
						break;
					}

					std::string::size_type j = str.find(',' , i);
					if(j == std::string::npos)
					{
						sp->error(parse_error);
						break;
					}
					std::string item_name = str.substr(i, j - i);

					j++;
					i = str.find(',' , j);
					std::string respawn_str = str.substr(j, i - j);
					if(i == std::string::npos)
					{
						respawn_str = str.substr(j);
					}

					int respawn_time = 0;
					int item_id = game->itemManager->getItemIdByName(item_name.c_str());
					if (item_id != -1 && sscanf(respawn_str.c_str(), "%i", &respawn_time) == 1)
					{
						game->itemManager->createSpawner(group_name.c_str(), item_id, item_name, GAME_TICKS_PER_SECOND*respawn_time/1000, pos, radius);
					}
					else
					{
						sp->error(parse_error);
						break;
					}

					// end of string
					if(i == std::string::npos)
						break;
				}

			} else {
				sp->error("ItemScripting::process - Attempt to createItemSpawner with null string value.");
			}
			break;

		case GS_CMD_activateItemSpawnerGroup:
		case GS_CMD_deactivateItemSpawnerGroup:
			if (stringData != NULL)
			{
				ItemSpawnerGroup *isg = game->itemManager->getSpawnerGroup(stringData);
				if(isg == NULL)
				{
					// if there is no group, create an empty one (so that any
					// spawners later added to it will be active automagicly)
					isg = game->itemManager->createSpawnerGroup(stringData);

					//sp->error("ItemScripting::process - Attempt to (de)activateItemSpawnerGroup with invalid group name (group has no spawners).");
					//sp->debug(stringData);
					//break;
				}

				if(command == GS_CMD_deactivateItemSpawnerGroup)
					game->itemManager->setSpawnerGroupActive(isg, false);
				else
					game->itemManager->setSpawnerGroupActive(isg, true);

			} else {
				sp->error("ItemScripting::process - Attempt to (de)activateItemSpawnerGroup with null string value.");
			}
			break;

		case GS_CMD_itemPosition:
			if (itemScriptItem != NULL)
			{
				gsd->position = itemScriptItem->getPosition();
			} else {
				sp->error("ItemScripting::process - Attempt to itemPosition outside item script.");
			}
			break;

		default:
			sp->error("ItemScripting::process - Unknown command.");
			assert(0);
		}

	}
}


