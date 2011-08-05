
#include "precompiled.h"

#include "LightBlinker.h"
#include "Game.h"
#include "GameRandom.h"
#include "GameUI.h"
#include "../ui/LightManager.h"
#include "../util/ColorMap.h"
#include "../util/SelfIlluminationChanger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"

#include "../system/Logger.h"
#include "../convert/str2int.h"

#include <IStorm3D_Terrain.h>
#include <istorm3D_terrain_renderer.h>

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{
	class LightBlinkerImpl
	{
		private:
			LightBlinkerImpl(Game *game, bool outdoor)
			{
				this->game = game;

				enabled = false;
				pauseTime = 0;
				pauseTimeVariation = 0;

				blinkerTimer = 0;
				thisFrameRandomPauseTime = 0;

				cycleTime = GAME_TICK_MSEC;
				randomVariation = 0.0f;
				strength = 0.0f;
				randomTime = 0;

				lastGameTimer = 0;

				randomTimeLeft = 0;
				lastRandomValue = 0.0f;

				lightColorDefault = COL(1,1,1);
				lightColor1 = COL(1,1,1);
				lightColor2 = COL(1,1,1);

				lastResultColor = COL(1,1,1);

				this->outdoor = outdoor;
			}

			Game *game;

			int blinkerTimer;
			int lastGameTimer;

			bool enabled;
			int pauseTime;
			int pauseTimeVariation;

			int thisFrameRandomPauseTime;

			int cycleTime;
			float randomVariation;
			float strength;
			int randomTime;

			float lastRandomValue;
			int randomTimeLeft;

			COL lightColorDefault;
			COL lightColor1;
			COL lightColor2;

			COL lastResultColor;

			bool outdoor;

			LightBlinker::LightBlinkType blinkType;

		friend class LightBlinker;
	};

	void LightBlinker::run()
	{
		COL result = impl->lightColorDefault;

		// update our local timer which is at correct (possibly varying) interval... 
		fb_assert(impl->game->gameTimer >= impl->lastGameTimer);
		impl->blinkerTimer += impl->game->gameTimer - impl->lastGameTimer;
		if (impl->blinkerTimer >= (impl->cycleTime + impl->pauseTime + impl->thisFrameRandomPauseTime) / GAME_TICK_MSEC)
		{
			// incorrect if lastGameTimer lags badly behind gameTimer?...
			//impl->blinkerTimer -= (impl->cycleTime + impl->pauseTime + impl->thisFrameRandomPauseTime) / GAME_TICK_MSEC;
			// incorrent, as should select the smallest of old/new pausetime to make sure this actually does something...
			//impl->blinkerTimer = impl->blinkerTimer % (impl->cycleTime + impl->pauseTime + impl->thisFrameRandomPauseTime) / GAME_TICK_MSEC;
			// should work?...
			impl->blinkerTimer = 0;

			impl->thisFrameRandomPauseTime = (int)(impl->pauseTimeVariation * (float)(impl->game->gameRandom->nextInt() % 101) / 100.0f);
		}
		impl->lastGameTimer = impl->game->gameTimer;


		if (impl->enabled)
		{
			float randomizedStrength = impl->strength;
			if (impl->randomVariation > 0.0f)
			{
				impl->randomTimeLeft -= GAME_TICK_MSEC;
				if (impl->randomTimeLeft <= 0)
				{
					impl->randomTimeLeft = impl->randomTime;

					impl->lastRandomValue = impl->randomVariation * (float)((impl->game->gameRandom->nextInt() % 201) - 100) / 100.0f;
				}
				randomizedStrength += impl->lastRandomValue;
				if (randomizedStrength > 1.0f) randomizedStrength = 1.0f;
				if (randomizedStrength < 0.0f) randomizedStrength = 0.0f;
			}

			float defaultColWeight = 1.0f - randomizedStrength;
			float weight = defaultColWeight;
			result = COL(defaultColWeight*impl->lightColorDefault.r,defaultColWeight*impl->lightColorDefault.g,defaultColWeight*impl->lightColorDefault.b);

			// color1/2 balance, 0 - 1 (- x if pausetime greater than zero)
			float colorBalanceZeroOne = (float)(impl->blinkerTimer % ((impl->cycleTime + impl->pauseTime + impl->thisFrameRandomPauseTime) / GAME_TICK_MSEC)) / (float)(impl->cycleTime / GAME_TICK_MSEC);

			// change the shape of the following sinwave...
			if (impl->blinkType == LightBlinker::LightBlinkTypeBiasColor2)
			{
				if (colorBalanceZeroOne <= 1.0f)
				{
					// going down faster
					if (colorBalanceZeroOne < 0.25f)
					{
						colorBalanceZeroOne *= 2.0f;
					} else {
						// and raising up slower
						colorBalanceZeroOne = 0.5f + (colorBalanceZeroOne - 0.25f) / 1.5f;
					}
				}
			}

			// color1/2 balance, sinwave -1 - +1
			// notice sinwave phase shift (90deg, thus, full color1 at 0 and 1 time - required by proper pausetime usage)
			float colorBalanceSinWave = (float)sinf((0.25f + colorBalanceZeroOne) * 3.1415926f * 2.0f);

			if (colorBalanceZeroOne > 1.0f)
			{
				// at pause time (use the default color only, strength zero)
				colorBalanceSinWave = 1.0f;
			}

			float col1Weight = randomizedStrength * (0.5f + colorBalanceSinWave * 0.5f);
			result.r += impl->lightColor1.r * col1Weight;
			result.g += impl->lightColor1.g * col1Weight;
			result.b += impl->lightColor1.b * col1Weight;
			weight += col1Weight;

			float col2Weight = randomizedStrength * (0.5f - colorBalanceSinWave * 0.5f);
			result.r += impl->lightColor2.r * col2Weight;
			result.g += impl->lightColor2.g * col2Weight;
			result.b += impl->lightColor2.b * col2Weight;
			weight += col2Weight;

			fb_assert(weight >= 0.99f);
			fb_assert(weight <= 2.11f);

			result.r /= weight;
			result.g /= weight;
			result.b /= weight;
		}

		// apply values only if changed since last run
		if (impl->lastResultColor.r != result.r
			|| impl->lastResultColor.g != result.g
			|| impl->lastResultColor.b != result.b)
		{
			// lightmap
			{
				//COL col = impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().getColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor);
				COL col = COL(result.r, result.g, result.b);

				if (impl->outdoor)
				{
					impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::OutdoorLightmapMultiplierColor, col);

					VC3 camGroundPos = impl->game->gameUI->getGameCamera()->getPosition();
					impl->game->gameMap->keepWellInScaledBoundaries(&camGroundPos.x, &camGroundPos.z);
					camGroundPos.y = impl->game->gameMap->getScaledHeightAt(camGroundPos.x, camGroundPos.z);

					// HACK: some magic numbers here.
					// NEW: no longer necessary
					//if (SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) >= 33)
					//{
						//float blinkRadius = 10.0f;
						//if (SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) >= 66)
						//{
						//	blinkRadius = 20.0f;
						//}
						float blinkRadius = 20.0f;
						impl->game->gameUI->getTerrain()->setToOutdoorColorMultiplier(col, camGroundPos, blinkRadius);
					//}
				} else {
					impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::LightmapMultiplierColor, col);
					impl->game->gameUI->getTerrain()->setToColorMultiplier(col);
					impl->game->gameUI->getLightManager()->getSelfIlluminationChanger()->setFactor(col);
				}
			}

			// colormap
			{
				//COL col = impl->game->gameMap->colorMap->getMultiplier();
				COL col = COL(result.r, result.g, result.b);

				if (impl->outdoor)
				{
					impl->game->gameMap->colorMap->setMultiplier(util::ColorMap::Outdoor, col);
				} else {
					impl->game->gameMap->colorMap->setMultiplier(util::ColorMap::Indoor, col);
					impl->game->gameUI->getLightManager()->setLightColorMultiplier(col);
					impl->game->decorationManager->updateDecorationIllumination(impl->game->gameMap->colorMap);
				}

				impl->game->gameUI->updateUnitLighting(true);
			}
		}

		impl->lastResultColor = result;
	}


	LightBlinker::LightBlinker(Game *game, bool outdoor)
	{
		fb_assert(game != NULL);
		this->impl = new LightBlinkerImpl(game, outdoor);
	}

	void LightBlinker::enable()
	{
		impl->enabled = true;
	}

	void LightBlinker::disable()
	{
		impl->enabled = false;
	}

	void LightBlinker::setBlinkingLightColorDefault(COL col)
	{
		impl->lightColorDefault = col;
	}

	void LightBlinker::setBlinkingLightColor1(COL col)
	{
		impl->lightColor1 = col;
	}

	void LightBlinker::setBlinkingLightColor2(COL col)
	{
		impl->lightColor2 = col;
	}

	void LightBlinker::setBlinkingLightColor1Red(float value)
	{
		impl->lightColor1.r = value;
	}

	void LightBlinker::setBlinkingLightColor1Green(float value)
	{
		impl->lightColor1.g = value;
	}

	void LightBlinker::setBlinkingLightColor1Blue(float value)
	{
		impl->lightColor1.b = value;
	}

	void LightBlinker::setBlinkingLightColor2Red(float value)
	{
		impl->lightColor2.r = value;
	}

	void LightBlinker::setBlinkingLightColor2Green(float value)
	{
		impl->lightColor2.g = value;
	}

	void LightBlinker::setBlinkingLightColor2Blue(float value)
	{
		impl->lightColor2.b = value;
	}

	void LightBlinker::setBlinkingLightRandomVariation(float value)
	{
		impl->randomVariation = value;
	}

	void LightBlinker::setBlinkingLightStrength(float value)
	{
		impl->strength = value;
	}

	void LightBlinker::setBlinkingLightFrequency(int cycleTime)
	{
		impl->cycleTime = cycleTime;
	}

	void LightBlinker::setBlinkingLightPauseTime(int pauseTime)
	{
		impl->pauseTime = pauseTime;
	}

	void LightBlinker::setBlinkingLightPauseTimeRandomVariation(int pauseTimeRandomVariation)
	{
		impl->pauseTimeVariation = pauseTimeRandomVariation;
	}

	void LightBlinker::setBlinkingLightRandomTime(int randomTime)
	{
		impl->randomTime = randomTime;
	}

	void LightBlinker::setBlinkingLightType(LightBlinkType blinkType)
	{
		impl->blinkType = blinkType;
	}

}

