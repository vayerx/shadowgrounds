
#include "precompiled.h"

#include "EnvironmentScripting.h"

#include "scripting_macros_start.h"
#include "environment_script_commands.h"
#include "scripting_macros_end.h"

#include <math.h>
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../../ui/Terrain.h"
#include "../../ui/VisualEffectManager.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../EnvironmentalEffectManager.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	void EnvironmentScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
			case GS_CMD_ADDENVIRONMENTALEFFECT:
				if (stringData != NULL)
				{
					if (game->getEnvironmentalEffectManager() != NULL
						&& game->inCombat)
						game->getEnvironmentalEffectManager()->addParticleEffect(stringData, false);
				} else {
					sp->error("EnvironmentScripting::process - addEnvironmentalEffect parameter missing.");
				}
				break;

			case GS_CMD_REMOVEENVIRONMENTALEFFECT:
				if (stringData != NULL)
				{
					if (game->getEnvironmentalEffectManager() != NULL
						&& game->inCombat)
						game->getEnvironmentalEffectManager()->removeParticleEffectByFilename(stringData, false);
				} else {
					sp->error("EnvironmentScripting::process - addEnvironmentalEffect parameter missing.");
				}
				break;

			case GS_CMD_REMOVEALLENVIRONMENTALEFFECTS:
				if (game->getEnvironmentalEffectManager() != NULL
					&& game->inCombat)
					game->getEnvironmentalEffectManager()->removeAllParticleEffects();
				break;

			// TODO: loads of stuff

			case GS_CMD_SETSUNLIGHTDIRECTION:
				// TODO: proper implementation
				if (stringData != NULL)
				{
					if (game->getEnvironmentalEffectManager() != NULL
						&& game->inCombat)
					{
						VC3 result;
						if (gs_tricoord_param(stringData, &result))
						{
							VC3 dir = result;
							dir.Normalize();
							game->getEnvironmentalEffectManager()->setSunlightDirection(dir);
						} else {
							sp->error("EnvironmentScripting::process - setSunlightDirection parameter bad."); 
						}
					}
				} else {
					sp->error("EnvironmentScripting::process - setSunlightDirection parameter missing (expected vector)."); 
				}
				break;

			case GS_CMD_SETSUNLIGHTCOLOR:
				// TODO: proper implementation
				if (stringData != NULL)
				{
					if (game->getEnvironmentalEffectManager() != NULL
						&& game->inCombat)
					{
						VC3 result;
						if (gs_tricoord_param(stringData, &result))
						{
							COL col = COL(result.x, result.y, result.z);
							game->getEnvironmentalEffectManager()->setSunlightColor(col);
						} else {
							sp->error("EnvironmentScripting::process - setSunlightColor parameter bad."); 
						}
					}
				} else {
					sp->error("EnvironmentScripting::process - setSunlightColor parameter missing (expected color)."); 
				}
				break;

			case GS_CMD_ENABLESUNLIGHT:
				// TODO: proper implementation
				if (game->getEnvironmentalEffectManager() != NULL
					&& game->inCombat)
				{
					game->getEnvironmentalEffectManager()->enableSunlight();
				}
				break;

			case GS_CMD_DISABLESUNLIGHT:
				// TODO: proper implementation
				if (game->getEnvironmentalEffectManager() != NULL
					&& game->inCombat)
				{
					game->getEnvironmentalEffectManager()->disableSunlight();
				}
				break;

			case GS_CMD_UPDATESUNLIGHTFOCUS:
				// TODO: proper implementation
				if (game->getEnvironmentalEffectManager() != NULL
					&& game->inCombat)
				{
					game->getEnvironmentalEffectManager()->updateSunlightFocus();
				}
				break;

			case GS_CMD_GETTERRAINSUNLIGHTAMOUNT:
				if (game->inCombat)
				{
					*lastValue = (int)(100.0f * game->gameUI->getTerrain()->getSunlightAmount());
				}
				break;

			case GS_CMD_SETFOGID:
				if (stringData != NULL)
				{
					if(game && game->getGameUI() && game->getGameUI()->getTerrain())
						game->getGameUI()->getTerrain()->setFogId(stringData);
				} else {
					sp->error("EnvironmentScripting::process - setFogId parameter missing.");
				}
				break;

			case GS_CMD_SETFOGINTERPOLATE:
				if(game && game->getGameUI() && game->getGameUI()->getTerrain())
				{
					game->getGameUI()->getTerrain()->setFogInterpolate(intData);
				}
				break;

			case GS_CMD_setEnvironmentEffectGroup:
				if(stringData != NULL)
				{
					if(game->getEnvironmentalEffectManager() && game->inCombat)
						game->getEnvironmentalEffectManager()->setEffectGroup(stringData);
				} else {
					sp->error("EnvironmentScripting::process - addEnvironmentEffectGroup parameter missing.");
				}
				break;

			case GS_CMD_fadeEnvironmentEffectGroup:
				if(stringData)
				{
					if(game->getEnvironmentalEffectManager() && game->inCombat)
						game->getEnvironmentalEffectManager()->fadeEffectGroup(stringData);
				} else {
					sp->error("EnvironmentScripting::process - fadeEnvironmentEffectGroup parameter missing.");
				}
				break;


			case GS_CMD_enableParticleInsideCheck:
				{
					bool enable = (intData) ? true: false;
					game->gameUI->getVisualEffectManager()->enableParticleInsideCheck(enable);
				}
				break;

			case GS_CMD_fadeOutAllEnvironmentEffects:
				{
					game->getEnvironmentalEffectManager()->fadeOutAllParticleEffects(intData);
				}
				break;

			case GS_CMD_fadeInAllEnvironmentEffects:
				{
					game->getEnvironmentalEffectManager()->fadeInAllParticleEffects(intData);
				}
				break;

			default:
				sp->error("EnvironmentScripting::process - Unknown command.");				
				assert(0);
		}
	}
}


