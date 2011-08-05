
#include "precompiled.h"

#include "PositionScripting.h"

#include "scripting_macros_start.h"
#include "position_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../scaledefs.h"
#include "../Game.h"
#include "../GameMap.h"
#include "../GameRandom.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	VC3 gs_global_temp_position(0,0,0);
	bool gs_global_temp_position_used = false;

	void PositionScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_SETPOSITION:
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					gsd->position = VC3(
						x, game->gameMap->getScaledHeightAt(x,y), y);
				} else {
					sp->error("PositionScripting::process - Missing or bad setPosition parameter.");
				}
			}
			break;

		case GS_CMD_POSITIONRANDOMOFFSET:
			if (intData > 0)
			{
				int o1 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				int o2 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				int o3 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				gsd->position.x += (float)o1 * 0.1f;
				gsd->position.y += (float)o2 * 0.1f;
				gsd->position.z += (float)o3 * 0.1f;
			} else {
				sp->warning("PositionScripting::process - positionRandomOffset with zero parameter.");
				lastValue = 0;
			}
			break;

		case GS_CMD_MOVEPOSITIONZ:
			gsd->position.z += (float)intData / 100.0f;
			break;

		case GS_CMD_MOVEPOSITIONX:
			gsd->position.x += (float)intData / 100.0f;
			break;

		case GS_CMD_MOVEPOSITIONZFLOAT:
			{
				float floatData = intFloat.f;
				gsd->position.z += floatData;
			}
			break;

		case GS_CMD_MOVEPOSITIONXFLOAT:
			{
				float floatData = intFloat.f;
				gsd->position.x += floatData;
			}
			break;

		case GS_CMD_GETPOSITIONX:
			*lastValue = (int)gsd->position.x;
			break;

		case GS_CMD_GETPOSITIONZ:
			*lastValue = (int)gsd->position.z;
			break;
							 
		case GS_CMD_GETPOSITIONHEIGHT:
			*lastValue = (int)gsd->position.y;
			break;
							 
		case GS_CMD_GETACCURATEPOSITIONX:
			*lastValue = (int)(gsd->position.x * 1000.0f);
			break;

		case GS_CMD_GETACCURATEPOSITIONZ:
			*lastValue = (int)(gsd->position.z * 1000.0f);
			break;
							 
		case GS_CMD_SETPOSITIONHEIGHT:
			if (stringData != NULL)
			{
				gsd->position.y = (float)atof(stringData);
			} else {
				sp->error("PositionScripting::process - Missing setPositionHeight parameter.");
			}
			break;

		case GS_CMD_SETPOSITIONHEIGHTONGROUND:
			{
				float x = gsd->position.x;
				float y = gsd->position.z;
				game->gameMap->keepWellInScaledBoundaries(&x, &y);
				gsd->position = VC3(
					x, game->gameMap->getScaledHeightAt(x,y), y);
			}
			break;

		case GS_CMD_POSITIONACCURATERANDOMOFFSET:
			if (intData > 0)
			{
				// intData values in cm
				int o1 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				int o2 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				int o3 = (game->gameRandom->nextInt() % (intData * 20 + 10)) - intData*10;
				gsd->position.x += (float)o1 * 0.1f * 0.01f;
				gsd->position.y += (float)o2 * 0.1f * 0.01f;
				gsd->position.z += (float)o3 * 0.1f * 0.01f;
			} else {
				sp->warning("PositionScripting::process - positionRandomOffset with zero parameter.");
				lastValue = 0;
			}
			break;

		case GS_CMD_MOVEPOSITIONHEIGHT:
			gsd->position.y += (float)intData / 100.0f;
			break;

		case GS_CMD_SETSECONDARYPOSITION:
			gsd->secondaryPosition = gsd->position;
			break;

		case GS_CMD_GETSECONDARYPOSITION:
			gsd->position = gsd->secondaryPosition;
			break;

		case GS_CMD_SETPOSITIONX:
			gsd->position.x = (float)*lastValue;
			break;

		case GS_CMD_SETPOSITIONZ:
			gsd->position.z = (float)*lastValue;
			break;
							 
		case GS_CMD_SETACCURATEPOSITIONX:
			gsd->position.x = (float)*lastValue / 1000.0f;
			break;

		case GS_CMD_SETACCURATEPOSITIONZ:
			gsd->position.z = (float)*lastValue / 1000.0f;
			break;

		case GS_CMD_setAccuratePositionHeight:
			gsd->position.y = (float)*lastValue / 1000.0f;
			break;

		case GS_CMD_getAccuratePositionHeight:
			*lastValue = (int)(gsd->position.y * 1000.0f);
			break;

		case GS_CMD_PUSHGLOBALTEMPPOSITION:
			if (gs_global_temp_position_used)
			{
				sp->warning("PositionScripting::process - pushGlobalTempPosition, stack full.");
			}
			gs_global_temp_position_used = true;
			gs_global_temp_position = gsd->position;
			break;
							 
		case GS_CMD_POPGLOBALTEMPPOSITION:
			if (!gs_global_temp_position_used)
			{
				sp->warning("PositionScripting::process - popGlobalTempPosition, stack empty.");
			}
			gsd->position = gs_global_temp_position;
			gs_global_temp_position_used = false;
			gs_global_temp_position = VC3(0,0,0);
			break;
							 
		case GS_CMD_MOVEPOSITIONTOANGLEVALUE:
			{
				float angle = (float)(*lastValue);
				if (angle < 0) angle += 360;
				if (angle >= 360) angle -= 360;
				float amount= (float)intData / 100.0f;
				gsd->position.x += -amount * sinf(UNIT_ANGLE_TO_RAD(angle));
				gsd->position.z += -amount * cosf(UNIT_ANGLE_TO_RAD(angle));
			}
			break;

		case GS_CMD_ISPOSITIONINSIDEBUILDING:
			{
				VC3 pos = gsd->position;
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
			}
			break;

		case GS_CMD_isPositionBlockedByUnmoving:
			{
				VC3 pos = gsd->position;
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					if (game->gameMap->getObstacleHeight(x, y) > 0
						&& !game->gameMap->isMovingObstacle(x, y))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_isPositionBlockedByUnmovingOrDoor:
			{
				VC3 pos = gsd->position;
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					// HACK: rounded moving obstacles are actually doors
					if (game->gameMap->getObstacleHeight(x, y) > 0
						&& (!game->gameMap->isMovingObstacle(x, y)
						|| game->gameMap->isRoundedObstacle(x, y)))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_isPositionBlockedByMoving:
			{
				VC3 pos = gsd->position;
				int x = game->gameMap->scaledToPathfindX(pos.x);
				int y = game->gameMap->scaledToPathfindY(pos.z);
				if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					if (game->gameMap->getObstacleHeight(x, y) > 0
						&& game->gameMap->isMovingObstacle(x, y))
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_setPositionVariable:
			if (stringData != NULL)
			{
				bool success = sp->getScript()->setGlobalPositionVariableValue(stringData, gsd->position.x, gsd->position.y, gsd->position.z);
				if (!success)
				{
					sp->error("PositionScripting::process - setPositionVariable, failed to set position variable value (variable does not exist or type mismatch).");
					sp->debug(stringData);
				}
			} else {
				sp->error("PositionScripting::process - setPositionVariable parameter missing, position variable name expected.");
			}
			break;

		case GS_CMD_getPositionVariable:
			if (stringData != NULL)
			{				
				float tmpx, tmpy, tmpz;
				bool success = sp->getScript()->getGlobalPositionVariableValue(stringData, &tmpx, &tmpy, &tmpz);
				if (success)
				{
					gsd->position = VC3(tmpx, tmpy, tmpz);
				} else {
					sp->error("PositionScripting::process - getPositionVariable, failed to get position variable value (variable does not exist or type mismatch).");
					sp->debug(stringData);
				}
			} else {
				sp->error("PositionScripting::process - getPositionVariable parameter missing, position variable name expected.");
			}
			break;

		case GS_CMD_addPositionVariableToPosition:
			if (stringData != NULL)
			{				
				float tmpx, tmpy, tmpz;
				bool success = sp->getScript()->getGlobalPositionVariableValue(stringData, &tmpx, &tmpy, &tmpz);
				if (success)
				{
					gsd->position = VC3(tmpx, tmpy, tmpz);
				} else {
					sp->error("PositionScripting::process - addPositionVariableToPosition, failed to get position variable value (variable does not exist or type mismatch).");
					sp->debug(stringData);
				}
			} else {
				sp->error("PositionScripting::process - addPositionVariableToPosition parameter missing, position variable name expected.");
			}
			break;

		case GS_CMD_setPositionKeepingHeight:
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					gsd->position.x = tmp.x;
					gsd->position.y = tmp.y;
				} else {
					sp->error("PositionScripting::process - Missing or bad setPositionKeepingHeight parameter.");
				}
			}
			break;

		case GS_CMD_setPositionXToFloat:
			{
				float floatData = intFloat.f;
				gsd->position.x = floatData;
			}
			break;

		case GS_CMD_setPositionZToFloat:
			{
				float floatData = intFloat.f;
				gsd->position.z = floatData;
			}
			break;

		default:
			sp->error("PositionScripting::process - Unknown command.");
			assert(0);
		}
	}
}


