
#include "precompiled.h"

#include "LightScripting.h"

#include "scripting_macros_start.h"
#include "light_script_commands.h"
#include "scripting_macros_end.h"

#include "../../util/LightAmountManager.h"

#include <IStorm3D_Terrain.h>
#include <istorm3D_terrain_renderer.h>
#include <math.h>
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../LightBlinker.h"
#include "../unified_handle.h"
#include "../UnifiedHandleManager.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../../ui/Spotlight.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../util/ScriptManager.h"
#include "../../util/TextureSwitcher.h"
#include "../../ui/LightManager.h"
#include "../../util/ColorMap.h"
#include "../../util/SelfIlluminationChanger.h"
#include "../../system/Logger.h"
#include "../../convert/str2int.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	/*
	ui::LightManager::AnimationType lightscripting_light_animation_type = ui::LightManager::NoAnimation;
	ui::LightManager::LightType lightscripting_light_type = ui::LightManager::Lighting;
	int lightscripting_light_color_red = 100;
	int lightscripting_light_color_green = 100;
	int lightscripting_light_color_blue = 100;
	int lightscripting_light_group = 0;
	int lightscripting_light_priority = 1;
	bool lightscripting_light_shadows = true;
	*/

	static ui::SpotProperties lightscripting_spot_props;
	static float lightscripting_light_angle_y = 0.0f;
	static bool lightscripting_add_pointlight = true;


	void LightScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		float floatData = intFloat.f;

		switch(command)
		{
		case GS_CMD_GETPOSITIONLIGHTAMOUNT:
			if (game->inCombat)
			{
				{
					*lastValue = int(util::LightAmountManager::getInstance()->getLightAmount(gsd->position, 1.5f) * 100.0f);
				}
			}

		case GS_CMD_GETPOSITIONDYNAMICLIGHTAMOUNT:
			if (game->inCombat)
			{
				IVisualObjectData *visData;
				*lastValue = int(util::LightAmountManager::getInstance()->getDynamicLightAmount(gsd->position, visData, 1.5f) * 100.0f);
			}
			break;

		case GS_CMD_GETPOSITIONSTATICLIGHTAMOUNT:
			if (game->inCombat)
			{
				if (game->gameMap->isWellInScaledBoundaries(gsd->position.x, gsd->position.z))
				{
					*lastValue = int(util::LightAmountManager::getInstance()->getStaticLightAmount(gsd->position, 1.5f) * 100.0f);
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_GETPOSITIONDYNAMICLIGHTORIGINUNIT:
			if (game->inCombat)
			{
				IVisualObjectData *visData;
				float lightAmount = util::LightAmountManager::getInstance()->getDynamicLightAmount(gsd->position, visData, 1.5f);
				if (visData != NULL && lightAmount > 0.0f)
				{
					*lastValue = 1;
					assert(visData->getVisualObjectDataId() == (void *)&unitDataId);
					// WARNING: unsafe cast, based on assert check above.
					gsd->unit = (Unit *)visData;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_GETPOSITIONMAXSTATICLIGHTAMOUNT:
			if (game->inCombat)
			{
				VC3 pos = gsd->position;

				if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					*lastValue = int(util::LightAmountManager::getInstance()->getStaticLightAmount(pos, 1.5f) * 100.0f);
					int tmp;

					pos.x += 0.5f;
					tmp = int(util::LightAmountManager::getInstance()->getStaticLightAmount(pos, 1.5f) * 100.0f);
					if (tmp > *lastValue)
						*lastValue = tmp;

					pos.x -= (0.5f + 0.5f);
					tmp = int(util::LightAmountManager::getInstance()->getStaticLightAmount(pos, 1.5f) * 100.0f);
					if (tmp > *lastValue)
						*lastValue = tmp;

					pos.x += 0.5f;
					pos.z += 0.5f;
					tmp = int(util::LightAmountManager::getInstance()->getStaticLightAmount(pos, 1.5f) * 100.0f);
					if (tmp > *lastValue)
						*lastValue = tmp;

					pos.z -= (0.5f + 0.5f);
					tmp = int(util::LightAmountManager::getInstance()->getStaticLightAmount(pos, 1.5f) * 100.0f);
					if (tmp > *lastValue)
						*lastValue = tmp;
				} else {
					*lastValue = 0;
				}
			}
			break;

		case GS_CMD_SWAPTEXTURETOSTRINGVALUE:
			if (game->inCombat)
			{
				if (stringData != NULL)
				{
					if (gsd->stringValue != NULL)
					{
						util::TextureSwitcher *ts = game->gameUI->getTextureSwitcher();
						ts->switchTexture(stringData, gsd->stringValue);
					} else {
						sp->error("LightScripting::process - swapTextureToStringValue attempt to swap for null string value.");
					}
				} else {
					sp->error("LightScripting::process - swapTextureToStringValue parameter missing.");
				}
			}
			break;

		case GS_CMD_SETAMBIENTLIGHT:
		case GS_CMD_SETAMBIENTLIGHTTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETAMBIENTLIGHT)
					val = float(intData) / 255.0f;
				else
					val = float(*lastValue) / 255.0f;
				if (val >= 0 && val <= 1.0f)
				{
					COL ambColor = COL(val, val, val);
					game->gameUI->getTerrain()->setAmbient(ambColor);
				} else {
					sp->error("LightScripting::process - setAmbientLight or setAmbientLightToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETFORCEAMBIENTLIGHT:
		case GS_CMD_SETFORCEAMBIENTLIGHTTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETFORCEAMBIENTLIGHT)
					val = float(intData) / 255.0f;
				else
					val = float(*lastValue) / 255.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, val);
				} else {
					sp->error("LightScripting::process - setAmbientLight or setAmbientLightToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETAMBIENTLIGHTRED:
		case GS_CMD_SETAMBIENTLIGHTREDTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETAMBIENTLIGHTRED)
					val = float(intData) / 255.0f;
				else
					val = float(*lastValue) / 255.0f;
				if (val >= 0 && val <= 1.0f)
				{
					COL ambColor = game->gameUI->getTerrain()->getAmbient();
					ambColor.r = val;
					game->gameUI->getTerrain()->setAmbient(ambColor);
				} else {
					sp->error("LightScripting::process - setAmbientLightRed or setAmbientLightRedToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETAMBIENTLIGHTGREEN:
		case GS_CMD_SETAMBIENTLIGHTGREENTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETAMBIENTLIGHTGREEN)
					val = float(intData) / 255.0f;
				else
					val = float(*lastValue) / 255.0f;
				if (val >= 0 && val <= 1.0f)
				{
					COL ambColor = game->gameUI->getTerrain()->getAmbient();
					ambColor.g = val;
					game->gameUI->getTerrain()->setAmbient(ambColor);
				} else {
					sp->error("LightScripting::process - setAmbientLightGreen or setAmbientLightGreenToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETAMBIENTLIGHTBLUE:
		case GS_CMD_SETAMBIENTLIGHTBLUETOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETAMBIENTLIGHTBLUE)
					val = float(intData) / 255.0f;
				else
					val = float(*lastValue) / 255.0f;
				if (val >= 0 && val <= 1.0f)
				{
					COL ambColor = game->gameUI->getTerrain()->getAmbient();
					ambColor.b = val;
					game->gameUI->getTerrain()->setAmbient(ambColor);
				} else {
					sp->error("LightScripting::process - setAmbientLightBlue or setAmbientLightBlueToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETLIGHTPOINTLIGHT:
			if (intData != 0)
			{
				lightscripting_add_pointlight = true;
			} else {
				lightscripting_add_pointlight = false;
			}
			break;

		case GS_CMD_ADDLIGHT:
			{
				VC3 lightpos = gsd->position;
				game->gameMap->keepWellInScaledBoundaries(&lightpos.x, &lightpos.z);

				if(lightscripting_spot_props.type != SpotProperties::ShadowCaster || lightscripting_spot_props.shadow)
					game->gameUI->getLightManager()->addSpot(lightpos, lightscripting_light_angle_y, lightscripting_spot_props);

				if(lightscripting_spot_props.type == SpotProperties::ShadowCaster
					&& lightscripting_add_pointlight)
				{
					ui::Light light;
					light.position = lightpos + VC3(0, lightscripting_spot_props.sourceHeight, 0);
					light.range = lightscripting_spot_props.range;
					light.minPlane = lightscripting_spot_props.minPlane;
					light.maxPlane = lightscripting_spot_props.maxPlane;
					light.color = lightscripting_spot_props.color;

					light.minPlane *= -light.range;
					light.maxPlane *= light.range;

					light.minPlane.x += light.position.x;
					light.minPlane.y += light.position.z;
					light.maxPlane.x += light.position.x;
					light.maxPlane.y += light.position.z;

					game->gameUI->getLightManager()->addLight(light);
				}

				lightscripting_spot_props = ui::SpotProperties();
				lightscripting_add_pointlight = true;
			}
			break;

		case GS_CMD_addBuildingLight:
			{
				VC3 lightpos = gsd->position;
				game->gameMap->keepWellInScaledBoundaries(&lightpos.x, &lightpos.z);
				COL lightcol = lightscripting_spot_props.color;
				float lightrange = lightscripting_spot_props.range;

				game->gameUI->getLightManager()->addBuildingLight(lightpos, lightcol, lightrange);

				lightscripting_spot_props = ui::SpotProperties();
				lightscripting_add_pointlight = true;
			}
			break;

		case GS_CMD_SETLIGHTANIMATIONTYPE:
			sp->warning("LightScripting::process - setLightAnimationType obsolete.");
			break;

		case GS_CMD_SETLIGHTCOLORRED:
			{
				lightscripting_spot_props.color.r = floatData;
			}
			break;

		case GS_CMD_SETLIGHTCOLORGREEN:
			{
				lightscripting_spot_props.color.g = floatData;
			}
			break;

		case GS_CMD_SETLIGHTCOLORBLUE:
			{
				lightscripting_spot_props.color.b = floatData;
			}
			break;

		case GS_CMD_SETLIGHTSHADOWS:
			{

				if (intData != 0)
					lightscripting_spot_props.shadow = true;
				else
					lightscripting_spot_props.shadow = false;
			}
			break;

		case GS_CMD_SETLIGHTGROUP:
			{
				lightscripting_spot_props.group = intData;
				//game::lightscripting_light_group = intData;
			}
			break;

		case GS_CMD_ENABLELIGHTGROUP:
			game->gameUI->getLightManager()->enableGroup(intData, true);
			break;

		case GS_CMD_DISABLELIGHTGROUP:
			game->gameUI->getLightManager()->enableGroup(intData, false);
			break;

		case GS_CMD_SETLIGHTMAPBRIGHTNESS:
		case GS_CMD_SETLIGHTMAPBRIGHTNESSTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETLIGHTMAPBRIGHTNESSTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::LightmapMultiplier, val);
				game->gameUI->getTerrain()->setToColorMultiplier(COL(val,val,val));

				game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(COL(val,val,val));
			}
			break;

		case GS_CMD_SETOUTDOORLIGHTMAPBRIGHTNESS:
		case GS_CMD_SETOUTDOORLIGHTMAPBRIGHTNESSTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETOUTDOORLIGHTMAPBRIGHTNESSTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::OutdoorLightmapMultiplier, val);
				game->gameUI->getTerrain()->setToOutdoorColorMultiplier(COL(val,val,val), VC3(), 100000.f);
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, val);

				//game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(COL(val,val,val));
			}
			break;

		case GS_CMD_setOutdoorLightmapBrightnessNearPositionToValue:
			{
				float val;
				val = (float(*lastValue) / 100.0f);
				float radius = (float)intData;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::OutdoorLightmapMultiplier, val);
				game->gameUI->getTerrain()->setToOutdoorColorMultiplier(COL(val,val,val), gsd->position, radius);
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, val);

				//game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(COL(val,val,val));
			}
			break;

		case GS_CMD_SETCOLORMAPBRIGHTNESS:
		case GS_CMD_SETCOLORMAPBRIGHTNESSTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETCOLORMAPBRIGHTNESSTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, val);
				game->gameUI->getLightManager()->setLightColorMultiplier(COL(val, val, val));
				game->decorationManager->updateDecorationIllumination(game->gameMap->colorMap);
				game->gameUI->updateUnitLighting(false);
			} 		
			break;

		case GS_CMD_SETLIGHTTYPE:
			if (stringData != NULL)
			{
				if (strcmp(stringData, "lighting") == 0)
				{
					lightscripting_spot_props.type = ui::SpotProperties::Lighting;
				}
				else if (strcmp(stringData, "shadow_caster") == 0)
				{
					lightscripting_spot_props.type = ui::SpotProperties::ShadowCaster;
				} else {
					sp->error("LightScripting::process - setLightType parameter invalid.");
				}
			} else {
				sp->error("LightScripting::process - setLightType parameter missing.");
			}
			break;

		case GS_CMD_SETLIGHTPRIORITY:
			{
				if (intData <= 0)
				{
					sp->warning("LightScripting::process - setLightPriority parameter bad.");
				}
				//game::lightscripting_light_priority = intData;
				game::lightscripting_spot_props.priority = intData;
			}
			break;

		case GS_CMD_SETLIGHTMAPBRIGHTNESSRED:
		case GS_CMD_SETLIGHTMAPBRIGHTNESSREDTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETLIGHTMAPBRIGHTNESSREDTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameUI->getTerrain()->GetTerrain()->getRenderer().getColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor);
				col.r = val;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor, col);
				game->gameUI->getTerrain()->setToColorMultiplier(col);
				game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(col);
			}
			break;

		case GS_CMD_SETLIGHTMAPBRIGHTNESSGREEN:
		case GS_CMD_SETLIGHTMAPBRIGHTNESSGREENTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETLIGHTMAPBRIGHTNESSGREENTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameUI->getTerrain()->GetTerrain()->getRenderer().getColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor);
				col.g = val;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor, col);
				game->gameUI->getTerrain()->setToColorMultiplier(col);
				game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(col);
			}
			break;

		case GS_CMD_SETLIGHTMAPBRIGHTNESSBLUE:
		case GS_CMD_SETLIGHTMAPBRIGHTNESSBLUETOVALUE:
			{
				float val;
				if (command == GS_CMD_SETLIGHTMAPBRIGHTNESSBLUETOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameUI->getTerrain()->GetTerrain()->getRenderer().getColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor);
				col.b = val;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor, col);
				game->gameUI->getTerrain()->setToColorMultiplier(col);
				game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(col);
			}
			break;

		case GS_CMD_SETCOLORMAPBRIGHTNESSRED:
		case GS_CMD_SETCOLORMAPBRIGHTNESSREDTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETCOLORMAPBRIGHTNESSREDTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameMap->colorMap->getMultiplier(util::ColorMap::Indoor);
				col.r = val;
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, col);
				game->gameUI->getLightManager()->setLightColorMultiplier(col);
				game->decorationManager->updateDecorationIllumination(game->gameMap->colorMap);
				game->gameUI->updateUnitLighting(false);
			} 		
			break;

		case GS_CMD_SETCOLORMAPBRIGHTNESSGREEN:
		case GS_CMD_SETCOLORMAPBRIGHTNESSGREENTOVALUE:
			{
				float val;
				if (command == GS_CMD_SETCOLORMAPBRIGHTNESSGREENTOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameMap->colorMap->getMultiplier(util::ColorMap::Indoor);
				col.g = val;
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, col);
				game->gameUI->getLightManager()->setLightColorMultiplier(col);
				game->decorationManager->updateDecorationIllumination(game->gameMap->colorMap);
				game->gameUI->updateUnitLighting(false);
			} 		
			break;

		case GS_CMD_SETCOLORMAPBRIGHTNESSBLUE:
		case GS_CMD_SETCOLORMAPBRIGHTNESSBLUETOVALUE:
			{
				float val;
				if (command == GS_CMD_SETCOLORMAPBRIGHTNESSBLUETOVALUE)
					val = (float(*lastValue) / 100.0f);
				else
					val = (float(intData) / 100.0f);
				COL col = game->gameMap->colorMap->getMultiplier(util::ColorMap::Indoor);
				col.b = val;
				game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, col);
				game->gameUI->getLightManager()->setLightColorMultiplier(col);
				game->decorationManager->updateDecorationIllumination(game->gameMap->colorMap);
				game->gameUI->updateUnitLighting(false);
			} 		
			break;

		case GS_CMD_SETLIGHTINGSPOTCULLRANGE:
			{
				float val;
				val = float(intData);
				game->gameUI->getLightManager()->setLightingSpotCullRange(val);
			}
			break;

		case GS_CMD_SETLIGHTINGSPOTFADEOUTRANGE:
			{
				float val;
				val = float(intData);
				game->gameUI->getLightManager()->setLightingSpotFadeoutRange(val);
			}
			break;

		case GS_CMD_SETLIGHTANGLEY:
			{
				lightscripting_light_angle_y = floatData;
			}
			break;

		case GS_CMD_SETLIGHTANGLEX:
			{
				lightscripting_spot_props.angle = floatData;
			}
			break;

		case GS_CMD_SETLIGHTRANGE:
			{
				lightscripting_spot_props.range = floatData;
			}
			break;

		case GS_CMD_SETLIGHTFOV:
			{
				lightscripting_spot_props.fov = floatData;
			}
			break;

		case GS_CMD_SETLIGHTBLINK:
			{
				if (intData != 0)
					lightscripting_spot_props.blink = true;
				else
					lightscripting_spot_props.blink = false;
			}
			break;

		case GS_CMD_SETLIGHTBLINKTIME:
			{
				lightscripting_spot_props.blinkTime = intData;
			}
			break;

		case GS_CMD_SETLIGHTROTATE:
			{
				if (intData != 0)
					lightscripting_spot_props.rotate = true;
				else
					lightscripting_spot_props.rotate = false;
			}
			break;

		case GS_CMD_SETLIGHTROTATETIME:
			{
				lightscripting_spot_props.rotateTime = intData;
			}
			break;

		case GS_CMD_SETLIGHTFADE:
			{
				if (intData != 0)
					lightscripting_spot_props.fade = true;
				else
					lightscripting_spot_props.fade = false;
			}
			break;

		case GS_CMD_SETLIGHTFADETIME:
			{
				lightscripting_spot_props.fadeTime = intData;
			}
			break;

		case GS_CMD_SETLIGHTCONE:
			{
				lightscripting_spot_props.cone = floatData;
			}
			break;

		case GS_CMD_SETLIGHTRESERVED1:
			{
				// nop
			}
			break;

		case GS_CMD_SETLIGHTTEXTURE:
			if (stringData != NULL)
			{
				lightscripting_spot_props.texture = std::string(stringData);
			} else {
				sp->error("LightScripting::process - setLightTexture parameter missing.");
			}
			break;

		case GS_CMD_SETLIGHTCONETEXTURE:
			if (stringData != NULL)
			{
				lightscripting_spot_props.coneTexture = std::string(stringData);
			} else {
				sp->error("LightScripting::process - setLightConeTexture parameter missing.");
			}
			break;

		case GS_CMD_SETLIGHTMODEL:
			if (stringData != NULL)
			{
				lightscripting_spot_props.lightModel = std::string(stringData);
			} else {
				sp->error("LightScripting::process - setLightModel parameter missing.");
			}
			break;

		case GS_CMD_SETLIGHTMINPLANEX:
			{
				lightscripting_spot_props.minPlane.x = floatData;
			}
			break;

		case GS_CMD_SETLIGHTMINPLANEY:
			{
				lightscripting_spot_props.minPlane.y = floatData;
			}
			break;

		case GS_CMD_SETLIGHTMAXPLANEX:
			{
				lightscripting_spot_props.maxPlane.x = floatData;
			}
			break;

		case GS_CMD_SETLIGHTMAXPLANEY:
			{
				lightscripting_spot_props.maxPlane.y = floatData;
			}
			break;

		case GS_CMD_SETLIGHTHEIGHT:
			{
				lightscripting_spot_props.height = floatData - gsd->position.y;
			}
			break;

		case GS_CMD_SETLIGHTSTRENGTH:
			{
				lightscripting_spot_props.strength = floatData;
			}
			break;

		case GS_CMD_SETLIGHTDISABLEOBJECTRENDERING:
			{
				if (intData != 0)
					lightscripting_spot_props.disableObjectRendering = true;
				else
					lightscripting_spot_props.disableObjectRendering = false;
			}
			break;

		case GS_CMD_SETLIGHTSOURCEHEIGHT:
			{
				lightscripting_spot_props.sourceHeight = floatData;
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT1:
		case GS_CMD_SETBLINKINGLIGHT1TOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT1)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor1(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setBlinkingLight1 or setBlinkingLight1ToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTDEFAULT:
		case GS_CMD_SETBLINKINGLIGHTDEFAULTTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHTDEFAULT)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColorDefault(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setBlinkingLightDefault or setBlinkingLightDefaultToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT1RED:
		case GS_CMD_SETBLINKINGLIGHT1REDTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT1RED)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor1Red(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight1Red or setBlinkingLight1RedToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT1GREEN:
		case GS_CMD_SETBLINKINGLIGHT1GREENTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT1GREEN)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor1Green(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight1Green or setBlinkingLight1GreenToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT1BLUE:
		case GS_CMD_SETBLINKINGLIGHT1BLUETOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT1BLUE)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor1Blue(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight1Blue or setBlinkingLight1BlueToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT2:
		case GS_CMD_SETBLINKINGLIGHT2TOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT2)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor2(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setBlinkingLight2 or setBlinkingLight2ToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT2RED:
		case GS_CMD_SETBLINKINGLIGHT2REDTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT2RED)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor2Red(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight2Red or setBlinkingLight2RedToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT2GREEN:
		case GS_CMD_SETBLINKINGLIGHT2GREENTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT2GREEN)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor2Green(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight2Green or setBlinkingLight2GreenToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHT2BLUE:
		case GS_CMD_SETBLINKINGLIGHT2BLUETOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHT2BLUE)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightColor2Blue(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLight2Blue or setBlinkingLight2BlueToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_ENABLEBLINKINGLIGHT:
			game->lightBlinker->enable();
			break;

		case GS_CMD_DISABLEBLINKINGLIGHT:
			game->lightBlinker->disable();
			break;

		case GS_CMD_SETBLINKINGLIGHTRANDOM:
		case GS_CMD_SETBLINKINGLIGHTRANDOMTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHTRANDOM)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightRandomVariation(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLightRandom or setBlinkingLightRandomToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTSTRENGTH:
		case GS_CMD_SETBLINKINGLIGHTSTRENGTHTOVALUE:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_SETBLINKINGLIGHTSTRENGTH)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->lightBlinker->setBlinkingLightStrength(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLightStrength or setBlinkingLightStrengthToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTFREQUENCY:
		case GS_CMD_SETBLINKINGLIGHTFREQUENCYTOVALUE:
			if (game->inCombat)
			{
				int val;
				if (command == GS_CMD_SETBLINKINGLIGHTFREQUENCY)
					val = intData;
				else
					val = *lastValue;
				if (val >= GAME_TICK_MSEC)
				{
					game->lightBlinker->setBlinkingLightFrequency(val);
				} else {
					sp->error("LightScripting::process - setBlinkingLightFrequency or setBlinkingLightFrequencyToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTPAUSETIME:
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->lightBlinker->setBlinkingLightPauseTime(intData);
				} else {
					sp->error("LightScripting::process - setBlinkingLightPauseTime parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTPAUSEVARIATION:
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->lightBlinker->setBlinkingLightPauseTimeRandomVariation(intData);
				} else {
					sp->error("LightScripting::process - setBlinkingLightPauseVariation parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTRANDOMTIME:
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->lightBlinker->setBlinkingLightRandomTime(intData);
				} else {
					sp->error("LightScripting::process - setBlinkingLightRandomTime parameter value out of range.");
				}
			}
			break;

		case GS_CMD_SETBLINKINGLIGHTTYPE:
			if (game->inCombat)
			{
				if (stringData != NULL)
				{
					if (strcmp(stringData, "sinwave") == 0)
					{
						game->lightBlinker->setBlinkingLightType(LightBlinker::LightBlinkTypeSinWave);
					}
					else if (strcmp(stringData, "bias_color_2") == 0)
					{
						game->lightBlinker->setBlinkingLightType(LightBlinker::LightBlinkTypeBiasColor2);
					} else {
						sp->error("LightScripting::process - setBlinkingLightType parameter invalid.");
					}
				} else {
					sp->error("LightScripting::process - setBlinkingLightType parameter missing.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight1:
		case GS_CMD_setOutdoorBlinkingLight1ToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight1)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor1(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight1 or setBlinkingLight1ToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightDefault:
		case GS_CMD_setOutdoorBlinkingLightDefaultToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLightDefault)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColorDefault(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightDefault or setBlinkingLightDefaultToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight1Red:
		case GS_CMD_setOutdoorBlinkingLight1RedToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight1Red)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor1Red(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight1Red or setBlinkingLight1RedToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight1Green:
		case GS_CMD_setOutdoorBlinkingLight1GreenToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight1Green)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor1Green(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight1Green or setBlinkingLight1GreenToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight1Blue:
		case GS_CMD_setOutdoorBlinkingLight1BlueToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight1Blue)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor1Blue(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight1Blue or setBlinkingLight1BlueToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight2:
		case GS_CMD_setOutdoorBlinkingLight2ToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight2)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor2(COL(val,val,val));
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight2 or setBlinkingLight2ToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight2Red:
		case GS_CMD_setOutdoorBlinkingLight2RedToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight2Red)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor2Red(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight2Red or setBlinkingLight2RedToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight2Green:
		case GS_CMD_setOutdoorBlinkingLight2GreenToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight2Green)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor2Green(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight2Green or setBlinkingLight2GreenToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLight2Blue:
		case GS_CMD_setOutdoorBlinkingLight2BlueToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLight2Blue)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightColor2Blue(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLight2Blue or setBlinkingLight2BlueToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_enableOutdoorBlinkingLight:
			game->outdoorLightBlinker->enable();
			break;

		case GS_CMD_disableOutdoorBlinkingLight:
			game->outdoorLightBlinker->disable();
			break;

		case GS_CMD_setOutdoorBlinkingLightRandom:
		case GS_CMD_setOutdoorBlinkingLightRandomToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLightRandom)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightRandomVariation(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightRandom or setBlinkingLightRandomToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightStrength:
		case GS_CMD_setOutdoorBlinkingLightStrengthToValue:
			if (game->inCombat)
			{
				float val;
				if (command == GS_CMD_setOutdoorBlinkingLightStrength)
					val = float(intData) / 100.0f;
				else
					val = float(*lastValue) / 100.0f;
				if (val >= 0 && val <= 1.0f)
				{
					game->outdoorLightBlinker->setBlinkingLightStrength(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightStrength or setBlinkingLightStrengthToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightFrequency:
		case GS_CMD_setOutdoorBlinkingLightFrequencyToValue:
			if (game->inCombat)
			{
				int val;
				if (command == GS_CMD_setOutdoorBlinkingLightFrequency)
					val = intData;
				else
					val = *lastValue;
				if (val >= GAME_TICK_MSEC)
				{
					game->outdoorLightBlinker->setBlinkingLightFrequency(val);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightFrequency or setBlinkingLightFrequencyToValue parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightPauseTime:
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->outdoorLightBlinker->setBlinkingLightPauseTime(intData);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightPauseTime parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightPauseVariation:						
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->outdoorLightBlinker->setBlinkingLightPauseTimeRandomVariation(intData);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightPauseVariation parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightRandomTime:
			if (game->inCombat)
			{
				if (intData >= 0)
				{
					game->outdoorLightBlinker->setBlinkingLightRandomTime(intData);
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightRandomTime parameter value out of range.");
				}
			}
			break;

		case GS_CMD_setOutdoorBlinkingLightType:
			if (game->inCombat)
			{
				if (stringData != NULL)
				{
					if (strcmp(stringData, "sinwave") == 0)
					{
						game->outdoorLightBlinker->setBlinkingLightType(LightBlinker::LightBlinkTypeSinWave);
					}
					else if (strcmp(stringData, "bias_color_2") == 0)
					{
						game->outdoorLightBlinker->setBlinkingLightType(LightBlinker::LightBlinkTypeBiasColor2);
					} else {
						sp->error("LightScripting::process - setOutdoorBlinkingLightType parameter invalid.");
					}
				} else {
					sp->error("LightScripting::process - setOutdoorBlinkingLightType parameter missing.");
				}
			}
			break;

		case GS_CMD_setGlowFadeFactor:
			if(game->inCombat)
			{
				float floatData = intFloat.f;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowFactor, floatData);
			}
			break;

		case GS_CMD_setGlowTransparencyFactor:
			if(game->inCombat)
			{
				float floatData = intFloat.f;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowTransparencyFactor, floatData);
			}
			break;

		case GS_CMD_setGlowAdditiveFactor:
			if(game->inCombat)
			{
				float floatData = intFloat.f;
				game->gameUI->getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowAdditiveFactor, floatData);
			}
			break;

		case GS_CMD_applyLightProperties:
			assert(!"TODO - set current light properties to selected (static) light");
			break;

		case GS_CMD_getLightProperties:
			assert(!"TODO - get selected (static) light properties");
			break;

		case GS_CMD_getDynamicLightProperties:
			assert(!"TODO - get selected (dynamic) light properties");
			break;

		case GS_CMD_setLightIdString:
			assert(!"TODO - set string value to selected light id string");
			break;

		case GS_CMD_getLightIdString:
			assert(!"TODO - set string value to selected light id string");
			break;

		case GS_CMD_setUnifiedHandleByLightIdString:
			assert(!"TODO - set unified handle by given light id string");
			break;

		case GS_CMD_updateLight:
			assert(!"TODO - sync lighting to terrain objects at light radius");
			break;

		case GS_CMD_addDynamicLight:
			assert(!"TODO - add a dynamic light, thru some dynamiclightmanager thingy?");
			break;

		case GS_CMD_applyDynamicLightProperties:
			assert(!"TODO - apply a dynamic light, thru some dynamiclightmanager thingy?");
			break;

		case GS_CMD_changeLightPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
						LightManager *lman = game->gameUI->getLightManager();
						lman->setLightPosition(lman->getLightId(gsd->unifiedHandle), gsd->position);
					} else {
						sp->error("LightScripting::process - changeLightPosition, light with given unified handle does not exist.");
					}
				} else {
					sp->error("LightScripting::process - changeLightPosition, given unified handle is not of light type.");
				}
			} else {
				sp->error("LightScripting::process - changeLightPosition, invalid unified handle.");
			}
			break;

		case GS_CMD_changeLightRadiusToValue:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
						LightManager *lman = game->gameUI->getLightManager();
						lman->setLightRadius(lman->getLightId(gsd->unifiedHandle), (float)*lastValue);
					} else {
						sp->error("LightScripting::process - changeLightRadiusToValue, light with given unified handle does not exist.");
					}
				} else {
					sp->error("LightScripting::process - changeLightRadiusToValue, given unified handle is not of light type.");
				}
			} else {
				sp->error("LightScripting::process - changeLightRadiusToValue, invalid unified handle.");
			}
			break;

		case GS_CMD_changeLightRadiusToFloatValue:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
						LightManager *lman = game->gameUI->getLightManager();
						lman->setLightRadius(lman->getLightId(gsd->unifiedHandle), gsd->floatValue);
					} else {
						sp->error("LightScripting::process - changeLightRadiusToFloatValue, light with given unified handle does not exist.");
					}
				} else {
					sp->error("LightScripting::process - changeLightRadiusToFloatValue, given unified handle is not of light type.");
				}
			} else {
				sp->error("LightScripting::process - changeLightRadiusToFloatValue, invalid unified handle.");
			}
			break;

		case GS_CMD_changeLightColor:
			if (stringData != NULL)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
					{
						if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
						{
							VC3 tmp = VC3(0,0,0);
							bool success = gs_tricoord_param(stringData, &tmp);
							if (success)
							{
								COL col = COL(tmp.x,tmp.y,tmp.z);
								LightManager *lman = game->gameUI->getLightManager();
								lman->setLightColor(lman->getLightId(gsd->unifiedHandle), col);
							} else {
								sp->error("LightScripting::process - changeLightColor, invalid parameter (light r,g,b value expected).");
							}
						} else {
							sp->error("LightScripting::process - changeLightColor, light with given unified handle does not exist.");
						}
					} else {
						sp->error("LightScripting::process - changeLightColor, given unified handle is not of light type.");
					}
				} else {
					sp->error("LightScripting::process - changeLightColor, invalid unified handle.");
				}
			} else {
				sp->error("LightScripting::process - changeLightColor parameter missing.");
			}
			break;

		case GS_CMD_deleteLight:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
						LightManager *lman = game->gameUI->getLightManager();
						lman->removeLight(lman->getLightId(gsd->unifiedHandle));
					} else {
						sp->error("LightScripting::process - deleteLight, light with given unified handle does not exist.");
					}
				} else {
					sp->error("LightScripting::process - deleteLight, given unified handle is not of light type.");
				}
			} else {
				sp->error("LightScripting::process - deleteLight, invalid unified handle.");
			}
			break;

		case GS_CMD_deleteAllLights:
			game->gameUI->getLightManager()->clearLights();
			break;

		case GS_CMD_updateAllTerrainObjectLighting:
			if (game->gameUI->getTerrain())
			{
				VC3 origo = VC3(0,0,0);
				game->gameUI->getTerrain()->updateLighting(origo, 99999.0f);
			}
			break;

		case GS_CMD_getFirstLightUnifiedHandle:
			gsd->unifiedHandle = game->gameUI->getLightManager()->getFirstLight();
			if (gsd->unifiedHandle != UNIFIED_HANDLE_NONE)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_getNextLightUnifiedHandle:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_LIGHT(gsd->unifiedHandle))
				{
					gsd->unifiedHandle = game->gameUI->getLightManager()->getNextLight(gsd->unifiedHandle);
					if (gsd->unifiedHandle != UNIFIED_HANDLE_NONE)
						*lastValue = 1;
					else
						*lastValue = 0;
				} else {
					sp->error("LightScripting::process - getNextLightUnifiedHandle, given unified handle is not of light type.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					*lastValue = 0;
				}
			} else {
				sp->error("LightScripting::process - getNextLightUnifiedHandle, invalid unified handle.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				*lastValue = 0;
			}
			break;

		case GS_CMD_spotTypeAdd:
			ui::Spotlight::addNewType(stringData);
			break;

		case GS_CMD_spotTypeSetTexture:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->textureFilename = stringData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetTexture, parameter missing (filename expected).");
			}
			break;

		case GS_CMD_spotTypeSetConeTexture:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->coneTextureFilename = stringData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetConeTexture, parameter missing (filename expected).");
			}
			break;

		case GS_CMD_spotTypeSetFOV:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->fov = floatData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetConeFOV:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->coneFov = floatData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetRange:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->range = floatData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetType:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (strcmp(stringData, "none") == 0)
					{
						spotProps->type = ui::SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_NONE;
					}
					else if (strcmp(stringData, "directional") == 0)
					{
						spotProps->type = ui::SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_DIRECTIONAL;
					}
					else if (strcmp(stringData, "point") == 0)
					{
						spotProps->type = ui::SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_POINT;
					}
					else if (strcmp(stringData, "flat") == 0)
					{
						spotProps->type = ui::SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_FLAT;
					} else {
						sp->error("LightScripting::process - spotTypeSetType, bad parameter (expected directional, point or flat).");
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetType, parameter missing (spotlight light model type expected).");
			}
			break;

		case GS_CMD_spotTypeSetConeMultiplier:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->coneAlphaMultiplier = floatData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetModel:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->modelFilename = stringData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetModel, parameter missing (filename expected).");
			}
			break;

		case GS_CMD_spotTypeSetShadows:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (intData != 0)
						spotProps->shadows = true;
					else
						spotProps->shadows = false;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetFade:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (intData != 0)
						spotProps->fade = true;
					else
						spotProps->fade = false;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetNoShadowFromOrigin:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (intData != 0)
						spotProps->noShadowFromOrigin = true;
					else
						spotProps->noShadowFromOrigin = false;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetFakelight:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (intData != 0)
						spotProps->fakelight = true;
					else
						spotProps->fakelight = false;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetFadeRange:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->fakeFadeRange = floatData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetScissor:
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (intData != 0)
						spotProps->scissor = true;
					else
						spotProps->scissor = false;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			}
			break;

		case GS_CMD_spotTypeSetDirection:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					VC3 result;
					if (gs_tricoord_param(stringData, &result))
					{
						VC3 dir = result;
						dir.Normalize();
						spotProps->direction = dir;
					} else {
						sp->error("LightScripting::process - spotTypeSetDirection parameter bad."); 
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetDirection, parameter missing (direction vector expected).");
			}
			break;

		case GS_CMD_spotTypeSetColorMultiplier:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					VC3 result;
					if (gs_tricoord_param(stringData, &result))
					{
						COL col = COL(result.x, result.y, result.z);
						spotProps->color = col;
					} else {
						sp->error("LightScripting::process - spotTypeSetColor parameter bad (should be r,g,b in range 0.0 - 1.0)."); 
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetColorMultiplier, parameter missing (color expected).");
			}
			break;

		case GS_CMD_spotTypeSetFakelightColor:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					VC3 result;
					if (gs_tricoord_param(stringData, &result))
					{
						COL col = COL(result.x, result.y, result.z);
						spotProps->fakelightColor = col;
					} else {
						sp->error("LightScripting::process - spotTypeSetFakelightColor parameter bad (should be r,g,b in range 0.0 - 1.0)."); 
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetFakelightColor, parameter missing (color expected).");
			}
			break;

		case GS_CMD_spotTypeSetFlashType:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					if (strcmp(stringData, "none") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_NONE;
					}
					else if (strcmp(stringData, "explosion") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_EXPLOSION;
					}
					else if (strcmp(stringData, "electric_flow") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_ELECTRIC_FLOW;
					}
					else if (strcmp(stringData, "flamethrower") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_FLAMETHROWER;
					}
					else if (strcmp(stringData, "muzzle") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_MUZZLE;
					}
					else if (strcmp(stringData, "flamer_muzzle") == 0)
					{
						spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_FLAMER_MUZZLE;
					} else {
						// assuming it must be a script... check it.
						util::Script *s = util::ScriptManager::getInstance()->getScript(stringData);
						if (s != NULL)
						{
							spotProps->flashType = ui::SpotTypeProperties::FLASH_TYPE_SCRIPT;
							spotProps->flashTypeScript = stringData;
						} else {
							sp->error("LightScripting::process - spotTypeSetType, bad parameter (expected predefined spotlight flash type or name of loaded script).");
						}
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetFlashType, parameter missing (spotlight flash type expected).");
			}
			break;

		case GS_CMD_spotTypeSetLowDetailVersion:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					spotProps->lowDetailVersion = stringData;
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetLowDetailVersion, parameter missing (spotlight type expected).");
			}
			break;

		case GS_CMD_spotTypeSetPositionOffset:
			if (stringData != NULL)
			{
				ui::SpotTypeProperties *spotProps = Spotlight::getAddingSpotTypeProperties();
				if (spotProps != NULL)
				{
					VC3 result;
					if (gs_tricoord_param(stringData, &result))
					{
						spotProps->positionOffset = result;
					} else {
						sp->error("LightScripting::process - spotTypeSetPositionOffset parameter bad."); 
					}
				} else {
					sp->error("LightScripting::process - spotType..., not currently adding a spot type (use spotTypeAdd first).");
				}
			} else {
				sp->error("LightScripting::process - spotTypeSetPositionOffset, parameter missing (direction vector expected).");
			}
			break;

		case GS_CMD_spotTypeAddDone:
			ui::Spotlight::addNewTypeDone();
			break;

		case GS_CMD_setLightLightingModelType:
			lightscripting_spot_props.lightingModelType = (ui::SpotProperties::LightingModelType)intData;
			break;

		case GS_CMD_setLightLightingModelFade:
			lightscripting_spot_props.lightingModelFade = intData == 0 ? false : true;
			break;

		case GS_CMD_setLightRotateRange:
			lightscripting_spot_props.rotateRange = floatData;
			break;

		default:
			sp->error("LightScripting::process - Unknown command.");				
			assert(0);
		}
	}
}


