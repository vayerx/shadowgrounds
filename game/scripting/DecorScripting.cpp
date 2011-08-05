
#include "precompiled.h"

#include "DecorScripting.h"

#include "scripting_macros_start.h"
#include "decor_script_commands.h"
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
#include "../../util/ColorMap.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	void DecorScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
			case GS_CMD_CREATEDECOR:
				gsd->decoration = game->decorationManager->createDecoration();
				break;

			case GS_CMD_SETDECORNAME:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						gsd->decoration->setName(stringData);
					} else {
						sp->error("DecorScripting::process - Attempt to setDecorName for null decoration.");
					} 			
				} else {
					sp->error("DecorScripting::process - setDecorName parameter missing.");
				}
				break;

			case GS_CMD_SETDECORPOSITION:
				{
					VC3 tmp(0,0,0);
					if (gs_coordinate_param(game->gameMap, stringData, &tmp))
					{
						gsd->decoration->setPosition(tmp);
					} else {
						sp->error("DecorScripting::process - Missing or bad setDecorPosition parameter.");
					}
				}
				break;

			case GS_CMD_SETDECORROTATIONTOVALUE:
				{
					VC3 tmp(0,(float)(*lastValue),0);
					gsd->decoration->setStartRotation(tmp);
				}
				break;

			case GS_CMD_SETDECORHEIGHT:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						gsd->decoration->setHeight((float)atof(stringData));
					} else {
						sp->error("DecorScripting::process - Attempt to setDecorHeight for null decoration.");
					} 			
				} else {
					sp->error("DecorScripting::process - setDecorHeight parameter missing.");
				}
				break;

			case GS_CMD_STRETCHDECOR:
				if (gsd->decoration != NULL)
				{
					gsd->decoration->stretchBetweenPositions();
				} else {
					sp->error("DecorScripting::process - Attempt to stretchDecor for null decoration.");
				}
				break;

			case GS_CMD_LOADDECORMODEL:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						gsd->decoration->loadModel(stringData);

						game->decorationManager->updateDecorationIllumination(game->gameMap->colorMap);
						/*
						// TODO: move to proper class (or something)...
						VisualObject *vo = gsd->decoration->getVisualObject();
						if (vo != NULL)
						{
							if(game->gameMap->colorMap)
							{
								GameMap *gameMap = game->gameMap;
								VC3 position = gsd->decoration->getPosition();

								position.x = position.x / gameMap->getScaledSizeX() + .5f;
								position.z = position.z / gameMap->getScaledSizeY() + .5f;

								COL color = gameMap->colorMap->getColor(position.x, position.z);

								vo->setSelfIllumination(color);
							}
						}
						*/
					
					} else {
						sp->error("DecorScripting::process - Attempt to loadDecorModel for null decoration.");
					}
				} else {
					sp->error("DecorScripting::process - loadDecorModel parameter missing.");
				} 			
				break;

			case GS_CMD_ENABLEDECOREFFECT:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						Decoration::DECORATION_EFFECT ef = Decoration::getDecorationEffectByName(stringData);
						if (ef != Decoration::DECORATION_EFFECT_INVALID)
						{
							gsd->decoration->setEffect(ef, true);
						} else {
							sp->error("DecorScripting::process - enableDecorEffect parameter bad.");
						}
					} else {
						sp->error("DecorScripting::process - Attempt to enableDecorEffect for null decoration.");
					}
				} else {
					sp->error("DecorScripting::process - enableDecorEffect parameter missing.");
				}
				break;

			case GS_CMD_DISABLEDECOREFFECT:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						Decoration::DECORATION_EFFECT ef = Decoration::getDecorationEffectByName(stringData);
						if (ef != Decoration::DECORATION_EFFECT_INVALID)
						{
							gsd->decoration->setEffect(ef, false);
						} else {
							sp->error("DecorScripting::process - enableDecorEffect parameter bad.");
						}
					} else {
						sp->error("DecorScripting::process - Attempt to disableDecorEffect for null decoration.");
					}
				} else {
					sp->error("DecorScripting::process - disableDecorEffect parameter missing.");
				}
				break;

			case GS_CMD_SETDECORENDPOSITION:
				{
					VC3 tmp(0,0,0);
					if (gs_coordinate_param(game->gameMap, stringData, &tmp))
					{
						if (gsd->decoration != NULL)
						{
							gsd->decoration->setEndPosition(tmp);
						} else {
							sp->error("DecorScripting::process - Attempt to setDecorEndPosition for null decoration.");
						}
					} else {
						sp->error("DecorScripting::process - Missing or bad setDecorEndPosition parameter.");
					}
				}
				break;

			case GS_CMD_SETDECORPOSITIONTOPOSITION:
				if (gsd->decoration != NULL)
				{
					gsd->decoration->setPosition(gsd->position);
				} else {
					sp->error("DecorScripting::process - Attempt to setDecorPositionToPosition for null decoration.");
				} 			
				break;

			case GS_CMD_SETDECORHEIGHTTOVALUE:
				if (gsd->decoration != NULL)
				{
					gsd->decoration->setHeight((float)intData / 100.0f);
				} else {
					sp->error("DecorScripting::process - Attempt to setDecorHeightToValue for null decoration.");
				} 			
				break;

			case GS_CMD_GETDECORBYNAME:
				if (stringData != NULL)
				{
					gsd->decoration = game->decorationManager->getDecorationByName(stringData);
					if (gsd->decoration != NULL)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					*lastValue = 0;
					sp->error("DecorScripting::process - getDecorByName parameter missing.");
				}
				break;

			case GS_CMD_getDecorById:
				gsd->decoration = game->decorationManager->getDecorationById(*lastValue);
				if (gsd->decoration != NULL)
					*lastValue = 1;
				else
					*lastValue = 0;
				break;

			case GS_CMD_getDecorId:
				if (gsd->decoration != NULL)
					*lastValue = game->decorationManager->getIdForDecoration(gsd->decoration);
				else
					*lastValue = 0;
				break;

			case GS_CMD_SETDECORNOSHADOW:
				{
					if (gsd->decoration != NULL)
					{
						gsd->decoration->setNoShadow(true);
					} else {
						sp->error("DecorScripting::process - Attempt to setDecorNoShadow for null decoration.");
					}
				}
				break;

			case GS_CMD_SETDECORSPEED:
				if (stringData != NULL)
				{
					if (gsd->decoration != NULL)
					{
						gsd->decoration->setSpeed((float)atof(stringData));
					} else {
						sp->error("DecorScripting::process - Attempt to setDecorSpeed for null decoration.");
					} 			
				} else {
					sp->error("DecorScripting::process - setDecorSpeed parameter missing.");
				}
				break;

			default:
				sp->error("DecorScripting::process - Unknown command.");				
				assert(0);
		}
	}
}


