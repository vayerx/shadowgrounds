
#include "precompiled.h"

#include "WaterScripting.h"

#include "scripting_macros_start.h"
#include "water_script_commands.h"
#include "scripting_macros_end.h"

#include <math.h>
#include "../Game.h"
#include "../GameMap.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../ui/Decoration.h"
#include "../../ui/DecorationManager.h"
#include "../../game/Water.h"
#include "../../game/WaterManager.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	void WaterScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		switch(command)
		{
			case GS_CMD_CREATEWATER:
				gsd->water = game->waterManager->createWater();
				break;

			case GS_CMD_SETWATERNAME:
				if (stringData != NULL)
				{
					if (gsd->water != NULL)
					{
						char *watername;
						if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
							watername = gsd->stringValue;
						else
							watername = stringData;

						gsd->water->setName(watername);
					} else {
						sp->error("WaterScripting::process - Attempt to setWaterName for null water.");
					} 			
				} else {
					sp->error("WaterScripting::process - setWaterName parameter missing.");
				}
				break;

			case GS_CMD_SETWATERPOSITION:
				{
					VC3 tmp(0,0,0);
					if (gs_coordinate_param(game->gameMap, stringData, &tmp))
					{
						if (gsd->water != NULL)
						{
							gsd->water->setPosition(tmp);
						} else {
							sp->error("WaterScripting::process - Attempt to setWaterPosition for null water.");
						}
					} else {
						sp->error("WaterScripting::process - Missing or bad setWaterPosition parameter.");
					}
				}
				break;

			case GS_CMD_setWaterAtPosition:
				if (gsd->water != NULL)
				{
					gsd->water->setPosition(gsd->position);
				} else {
					sp->error("WaterScripting::process - Attempt to setWaterAtPosition for null water.");
				}
				break;

			case GS_CMD_getWaterDepthAtPosition:
				*lastValue = (int)(game->waterManager->getWaterDepthAt(gsd->position) * 100.0f);
				break;

			case GS_CMD_SETWATERHEIGHT:
				if (stringData != NULL)
				{
					if (gsd->water != NULL)
					{
						gsd->water->setHeight((float)atof(stringData));
					} else {
						sp->error("WaterScripting::process - Attempt to setWaterHeight for null water.");
					} 			
				} else {
					sp->error("WaterScripting::process - setWaterHeight parameter missing.");
				}
				break;

			case GS_CMD_SETWATERHEIGHTTOVALUE:
				if (gsd->water != NULL)
				{
					gsd->water->setHeight((float)*lastValue / 100.0f);
				} else {
					sp->error("WaterScripting::process - Attempt to setWaterHeightToValue for null water.");
				} 			
				break;

			case GS_CMD_GETWATERBYNAME:
				if (stringData != NULL)
				{
					gsd->water = game->waterManager->getWaterByName(stringData);
					if (gsd->water != NULL)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
					sp->error("WaterScripting::process - getWaterByName parameter missing.");
				}
				break;

			case GS_CMD_CREATEWATERDECOR:
				if (gsd->water != NULL)
				{
					gsd->decoration = game->decorationManager->createDecoration();
					gsd->water->setDecoration(gsd->decoration);
				} else {
					sp->error("WaterScripting::process - Attempt to createWaterDecor for null water.");
				} 			
				break;

			case GS_CMD_UPDATEWATER:
				if (gsd->water != NULL)
				{
					gsd->water->updateBoundaries();
					game->waterManager->recalculate();
				} else {
					sp->error("WaterScripting::process - Attempt to updateWater for null water.");
				} 			
				break;

			default:
				sp->error("WaterScripting::process - Unknown command.");				
				assert(0);
		}
	}
}


