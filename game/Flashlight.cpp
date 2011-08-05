
#include "precompiled.h"

#include "Flashlight.h"

#include "Game.h"
#include "GameUI.h"
#include "GameMap.h"
#include "GameScene.h"
#include "GameRandom.h"

#include "options/options_game.h"
#include "SimpleOptions.h"

#include "../util/ColorMap.h"
#include "../util/LightMap.h"
#include "../ui/VisualObject.h"
#include "../ui/Spotlight.h"
#include "../util/SpotLightCalculator.h"
#include "../util/LightAmountManager.h"

#include <boost/shared_ptr.hpp>


#define FLASHLIGHT_HEIGHT 0.0f

// THESE ARE IN GAME OPTIONS NOW
//#define FLASHLIGHT_MAX_ENERGY 60000
//#define FLASHLIGHT_LOW_ENERGY 15000
#define FLASHLIGHT_LOW_BLINK_ENERGY 1000

//#define FLASHLIGHT_ENERGY_USAGE 10

//#define FLASHLIGHT_ENERGY_RECHARGE_AMOUNT_DYNAMIC 50

//#define FLASHLIGHT_ENERGY_RECHARGE_AMOUNT_STATIC 50

//#define FLASHLIGHT_ENERGY_MIN_RECHARGE_ILLUM 0.15f


namespace game
{
  class FlashlightImpl
	{
		private:
			Game *game;
			util::ColorMap *colorMap;
			util::LightMap *lightMap;
			ui::VisualObject *origin;
			ui::Spotlight *spotlight;
			VC3 position;
			int energy;
			bool needRecharge;
			bool operable;
			float angle;
			float betaAngle;
			float offset;
			int lastRecharge;
			float tempBrightnessFactor;
			float lastBrightnessFactor;
			boost::shared_ptr<util::SpotLightCalculator> lightCalculator;
			float swayFactor;
			float shakeFactor;
			int swayCurrentTime;
			int shakeCurrentTime; 
			int swayTotalTime;
			int shakeTotalTime; 
			float impactFactor;
			float impactDirection;


			FlashlightImpl(Game *game, ui::VisualObject *origin)
			{
				this->game = game;
				this->colorMap = game->gameMap->colorMap;
				this->lightMap = game->gameMap->lightMap;
				this->origin = origin;
				this->spotlight = NULL;
				this->position = VC3(0,0,0);
				this->energy = SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX);
				this->needRecharge = false;
				this->operable = true;
				this->angle = 0;
				this->betaAngle = 0;
				this->lastRecharge = 0;
				this->offset = 0.0f;
				this->tempBrightnessFactor = 1.0f;
				this->lastBrightnessFactor = 1.0f;
				this->swayFactor = 0.0f;
				this->shakeFactor = 0.0f;
				this->swayCurrentTime = 0;
				this->shakeCurrentTime = 0; 
				this->swayTotalTime = 100;
				this->shakeTotalTime = 10;
				this->impactFactor = 0.0f;
				this->impactDirection = 0.0f;
			}

		friend class Flashlight;
	};



	Flashlight::Flashlight(Game *game, ui::VisualObject *origin)
	{
		this->impl = new FlashlightImpl(game, origin);
	}


	Flashlight::~Flashlight()
	{
		if (isFlashlightOn())
			setFlashlightOn(false);

		delete impl;
	}


	void Flashlight::resetOrigin(ui::VisualObject *origin)
	{
		bool wasOn = isFlashlightOn();
		setFlashlightOn(false);
		impl->origin = origin;
		if (wasOn)
			setFlashlightOn(true);
	}


	void Flashlight::setOffset(float offset)
	{
		impl->offset = offset;
	}


	void Flashlight::run(const VC3 &position)
	{
		impl->position = position;

		impl->swayCurrentTime += GAME_TICK_MSEC;
		impl->shakeCurrentTime += GAME_TICK_MSEC;

		if (impl->impactFactor > 0.0f)
		{
			impl->impactFactor -= 0.01f;

			if (impl->impactFactor < 0.0f)
				impl->impactFactor = 0.0f;
		}

		if (impl->spotlight != NULL)
		{
			VC3 pos = impl->position;
			pos.y += FLASHLIGHT_HEIGHT;

			if (impl->offset != 0.0f)
			{
				if (fabs(impl->betaAngle) > 0.001f)
				{
					// TODO: this rotation is done in 2 seperate places
					// very inefficient and bad copy&paste coding.
					// redo.
					VC3 dir = VC3(-sinf(impl->angle), -.3f, -cosf(impl->angle));

					QUAT rot = QUAT();
					rot.MakeFromAngles(0, 0, -impl->betaAngle);
					dir = rot.GetRotated(dir);

					dir.Normalize();
					pos += (dir * impl->offset);
				} else {
					// TODO: other angles than y-angle too...
					VC3 dir = VC3(-sinf(impl->angle), -.3f, -cosf(impl->angle));
					dir.Normalize();
					pos += (dir * impl->offset);
				};
			}

			impl->spotlight->setPosition(pos);
		}

		int recharging = 0;	

		// consume energy if on, or recharge based on light if off...
		if (isFlashlightOn())
		{
			VC3 dir = VC3(-sinf(impl->angle), -.3f, -cosf(impl->angle));
			dir.Normalize();
			impl->lightCalculator->update(position, dir);

			assert(impl->spotlight != NULL);
			impl->energy -= SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_USAGE);

			if (impl->energy < SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW))
			{
				if (impl->energy > FLASHLIGHT_LOW_BLINK_ENERGY
				  || (impl->game->gameTimer % 11) < impl->energy / (FLASHLIGHT_LOW_BLINK_ENERGY / 20))
				{
					float lowFactor = (float)impl->energy / (float)SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW);
					COL lowIntensity = COL(0.20f + lowFactor * 0.80f, 0.15f + lowFactor * 0.85f, 0.10f + lowFactor * 0.90f);
					impl->spotlight->setSpotlightParams(lowIntensity);
				} else {
					COL noIntensity = COL(0.15f, 0.15f, 0.05f);
					impl->spotlight->setSpotlightParams(noIntensity);
				}
			}
			if (impl->energy < 0)
			{
				impl->energy = 0;
				impl->needRecharge = true;
				setFlashlightOn(false);
			}
			impl->lastRecharge = 0;
		} else {
			// recharge amount is based on _minimum_ light component..
			// thus, one colored lights won't recharge (just plain red,
			// green, blue or combination of two of these),
			// whereas white lights will recharge nicely.

			//VC3 pos = VC3(0,0,0);
			//pos.x = impl->position.x / impl->game->gameMap->getScaledSizeX() + .5f;
			//pos.z = impl->position.z / impl->game->gameMap->getScaledSizeY() + .5f;
			//COL col = impl->colorMap->getColor(pos.x, pos.z);
			VC3 boundedPos = impl->position;
			impl->game->gameMap->keepWellInScaledBoundaries(&boundedPos.x, &boundedPos.z);

			COL col = impl->colorMap->getColorAtScaled(boundedPos.x, boundedPos.z);
			COL fullCol = impl->colorMap->getUnmultipliedColorAtScaled(boundedPos.x, boundedPos.z);
			float illum = col.r;
			if (col.g < illum) illum = col.g;
			if (col.b < illum) illum = col.b;

			// HACK: if unmultiplied colormap component avg below lightmap brightness,
			// we must be near some dynamic spotlight giving some light..
			// then, use the lightmap brightness...
			float fullColAvg = (fullCol.r + fullCol.g + fullCol.b) / 3.0f;
			float lightmapBri = (float)impl->lightMap->getLightAmount(impl->lightMap->scaledToLightMapX(boundedPos.x), impl->lightMap->scaledToLightMapY(boundedPos.z));
			if ((fullColAvg * 255.0f) < lightmapBri - 10.0f)
			{
				illum = lightmapBri / 255.0f;
			}

			// NEW: flashlight recharges everywhere!!! even in darkness.
/*
			illum = FLASHLIGHT_ENERGY_MIN_RECHARGE_ILLUM + 0.01f;

			if (illum >= FLASHLIGHT_ENERGY_MIN_RECHARGE_ILLUM)
			{
				recharging = FLASHLIGHT_ENERGY_RECHARGE_AMOUNT_STATIC + int(illum * FLASHLIGHT_ENERGY_RECHARGE_AMOUNT_DYNAMIC);
			} else {
				recharging = 0;
			}
			// NEW: flashlight recharges only if energy low
			if (impl->energy < FLASHLIGHT_MAX_ENERGY)

// TEMP: for demo!
//			if (impl->energy < FLASHLIGHT_LOW_ENERGY)
			{
				impl->energy += recharging;
//				if (impl->energy > FLASHLIGHT_LOW_ENERGY)
//				{
//					impl->energy = FLASHLIGHT_LOW_ENERGY;
//				}
// TEMP: for demo!
				if (impl->energy > FLASHLIGHT_MAX_ENERGY)
				{
					impl->energy = FLASHLIGHT_MAX_ENERGY;
				}
			}
			if (impl->energy >= FLASHLIGHT_LOW_ENERGY)
			{
				impl->needRecharge = false;
			}
			impl->lastRecharge = recharging;
*/

			if (illum >= (float)SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_RECHARGE_MIN_ILLUMINATION) / 100.0f)
			{
				recharging = SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_RECHARGE_STATIC) + int(illum * SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_RECHARGE_DYNAMIC));
			} else {
				recharging = 0;
				if (impl->energy < SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW))
				{
					// really slow recharge even in darkness, if low energy
					recharging = SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_RECHARGE_STATIC_DARK);
				}
			}
			if (impl->energy < SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX))
			{
				impl->energy += recharging;
				if (impl->energy > SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX))
				{
					impl->energy = SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX);
				}
			}
			if (impl->energy >= SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW))
			{
				impl->needRecharge = false;
			}
			impl->lastRecharge = recharging;

		}

	}


	void Flashlight::prepareForRender()
	{
		if (impl->tempBrightnessFactor != impl->lastBrightnessFactor)
		{
			// FIXME: should use current intensity as base value for the factor!
			// (now non-full-bright flashlight may get too bright because of this)
			COL tempIntensity = COL(impl->tempBrightnessFactor, impl->tempBrightnessFactor, impl->tempBrightnessFactor);
			if (impl->spotlight != NULL)
			{
				impl->spotlight->setSpotlightParams(tempIntensity);
			}
			impl->lastBrightnessFactor = impl->tempBrightnessFactor;
			impl->tempBrightnessFactor = 1.0f;
		}

		if (impl->spotlight != NULL)
		{
			impl->spotlight->prepareForRender();
		}
	}


	void Flashlight::setFlashlightOperable(bool operable)
	{
		// switch flashlight off if we're setting it inoperable
		if (!operable && isFlashlightOn())
		{
			setFlashlightOn(false);
		}
		impl->operable = operable;
	}


	void Flashlight::setSwayFactor(float factor)
	{
		if (impl->swayFactor <= factor - 0.001f)
		{
			impl->swayFactor += 0.001f;
		} else {
			if (impl->swayFactor >= factor + 0.0001f)
			{
				// HACK: when tuning down the sway, do it really slow..
				// this way after some running, the light still seems
				// more alive for a while...
				if ((impl->game->gameTimer & 1) == 0)
				{
					impl->swayFactor -= 0.0001f;
				}
				//impl->swayFactor -= 0.001f;
			} else {
				impl->swayFactor = factor;
			}
		}
		//impl->swayFactor = factor;
	}


	void Flashlight::setShakeFactor(float factor)
	{
		if (impl->shakeFactor <= factor - 0.001f)
		{
			impl->shakeFactor += 0.001f;
		} else {
			if (impl->shakeFactor >= factor + 0.001f)
			{
				impl->shakeFactor -= 0.001f;
			} else {
				impl->shakeFactor = factor;
			}
		}
		//impl->shakeFactor = factor;
	}

	void Flashlight::setShakeTime(int time)
	{
		if (time == impl->shakeTotalTime)
			return;

		if (time < 1) time = 1;
		impl->shakeTotalTime = time;
	}

	void Flashlight::setSwayTime(int time)
	{
		if (time == impl->swayTotalTime)
			return;

		if (time < 1) time = 1;
		float relTime = ((float)impl->swayCurrentTime / (float)impl->swayTotalTime);
		//float sinPhase = relTime - float(int(relTime));
		//impl->swayCurrentTime = sinPhase * time;
		impl->swayTotalTime = time;
		impl->swayCurrentTime = (int)((float)relTime * (float)impl->swayTotalTime);
	}

	void Flashlight::setImpact(float factor)
	{
		impl->impactFactor = factor;
		impl->impactDirection = (impl->game->gameRandom->nextInt() % 360) / 180.0f * 3.1415926f;
	}


	void Flashlight::setRotation(float angle)
	{
		impl->angle = angle;
		if (impl->spotlight != NULL)
		{
			if (fabs(impl->betaAngle) > 0.001f)
			{
				// TODO...
				//VC3 dir = VC3(-sinf(angle), -.3f, -cosf(angle));

				//QUAT rot = QUAT();
				//rot.MakeFromAngles(0, 0, -impl->betaAngle);
				//dir = rot.GetRotated(dir);

				// shake and sway...
				float shakenAngle = angle;
				float shakenBetaAngle = impl->betaAngle;
				if (impl->shakeFactor != 0.0f)
				{
					shakenAngle += sinf((float)impl->shakeCurrentTime / (float)impl->shakeTotalTime) * impl->shakeFactor;
					shakenBetaAngle += sinf((float)impl->shakeCurrentTime / (float)impl->shakeTotalTime) * impl->shakeFactor;
				}
				if (impl->swayFactor != 0.0f)
				{
					shakenAngle += sinf((float)impl->swayCurrentTime / (float)impl->swayTotalTime * 0.5f) * impl->swayFactor;
					shakenBetaAngle += sinf((float)impl->swayCurrentTime / (float)impl->swayTotalTime) * impl->swayFactor * 0.5f;
				}

				QUAT rot = QUAT(0, shakenAngle, 0);
				QUAT rot2 = QUAT(-shakenBetaAngle, 0, 0);
				rot = rot2 * rot;

				VC3 dir = VC3(0, -.3f, -1.0f);
				dir = rot.GetRotated(dir);

				dir.Normalize();

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
dir.x = -dir.x;
dir.z = -dir.z;
#endif

#ifdef PROJECT_AOV
			// don't touch the direction!
#else
				impl->spotlight->setDirection(dir);				
#endif
			} else {
				float shakenAngle = angle;
				float shakenBeta = 0.0f;
				if (impl->shakeFactor != 0.0f)
				{
					shakenAngle += sinf((float)impl->shakeCurrentTime / (float)impl->shakeTotalTime) * impl->shakeFactor;
					shakenBeta += sinf((float)impl->shakeCurrentTime / (float)impl->shakeTotalTime) * impl->shakeFactor;
				}
				if (impl->swayFactor != 0.0f)
				{
					shakenAngle += sinf((float)impl->swayCurrentTime / (float)impl->swayTotalTime * 0.5f) * impl->swayFactor;
					shakenBeta += sinf((float)impl->swayCurrentTime / (float)impl->swayTotalTime) * impl->swayFactor * 0.5f;
				}
				if (impl->impactFactor != 0.0f)
				{
					shakenAngle += cosf(impl->impactDirection) * impl->impactFactor;
					shakenBeta += sinf(impl->impactDirection) * impl->impactFactor;
				}

				VC3 dir = VC3(-sinf(shakenAngle), -.3f + shakenBeta, -cosf(shakenAngle));
				dir.Normalize();

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
dir.x = -dir.x;
dir.z = -dir.z;
#endif

#ifdef PROJECT_AOV
			// don't touch the direction!
#else
				impl->spotlight->setDirection(dir);
#endif
			}
		}
	}


	void Flashlight::setRotationToward(float angle, int timeElapsed)
	{
		impl->angle = angle;
		if (impl->spotlight != NULL)
		{
			VC3 pos = impl->position;
			VC3 dir = VC3(-sinf(angle), 0, -cosf(angle));
			pos.y += FLASHLIGHT_HEIGHT;
			pos += dir * 0.2f;
			dir.y = -.70f;
			dir.Normalize();

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
dir.x = -dir.x;
dir.z = -dir.z;
#endif

#ifdef PROJECT_AOV
			// don't touch the direction!
#else
			impl->spotlight->setDirectionToward(dir, timeElapsed);
#endif
		}
	}


	void Flashlight::toggleOn()
	{
		if (isFlashlightOn())
			setFlashlightOn(false);
		else
			setFlashlightOn(true);
	}


	bool Flashlight::isFlashlightOn()
	{
		if (impl->spotlight != NULL)
		{
			return true;
		} else {
			return false;
		}
	}


	void Flashlight::setFlashlightOn(bool flashlightOn)
	{
		// return if already in requested state
		if ((isFlashlightOn() && flashlightOn) || (!isFlashlightOn() && !flashlightOn))
			return;

		impl->swayCurrentTime = 0;
		impl->shakeCurrentTime = 0;
		impl->impactFactor = 0;
		impl->impactDirection = 0;

		// return if energy very very low and attempting to set 
		// flashlight on.
		// actually, if energy was totally depleted, cannot set 
		// flashlight on before recharge.
		//if (flashlightOn && impl->energy < FLASHLIGHT_LOW_ENERGY)
		if (flashlightOn && impl->needRecharge)
			return;

		// return if flashlight not operable and trying to set on
		if (flashlightOn && !impl->operable)
			return;

		if (flashlightOn)
		{
			// set it on
			IStorm3D_Model *stormModel = NULL;
			if (impl->origin != NULL) 
					stormModel = impl->origin->getStormModel();
			ui::Spotlight *sp = new ui::Spotlight(
				*impl->game->getGameScene()->getStorm3D(), *impl->game->gameUI->getTerrain()->GetTerrain(),
				*impl->game->getGameScene()->getStormScene(), stormModel, 
				std::string("flashlight"));
			impl->spotlight = sp;
			this->setRotation(impl->angle);

			impl->lightCalculator = boost::shared_ptr<util::SpotLightCalculator> (new util::SpotLightCalculator(40.0f, 20.0f, impl->origin->getDataObject()));
			util::LightAmountManager::getInstance()->add(impl->lightCalculator);

			VC3 pos = impl->position;
			this->run(pos);

		} else {

			// FIXME: loses all flashlights??
			// should re-add the others maybe?
			impl->lightCalculator.reset();

			// set it off
			delete impl->spotlight;
			impl->spotlight = NULL;
		}
	}


	int Flashlight::getFlashlightEnergy()
	{
		return ((impl->energy * 100) / SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX));
	}


	void Flashlight::setFlashlightEnergy(int energyPercentage)
	{
		//assert(energyPercentage >= 0 && energyPercentage <= 100);

		if (energyPercentage < 0) energyPercentage = 0;
		//if (energyPercentage > 100) energyPercentage = 100;
		// HACK: allow over-charge
		if (energyPercentage > 2000) energyPercentage = 2000;

		impl->energy = (energyPercentage * SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_MAX)) / 100;

		if (isFlashlightOn())
		{
			assert(impl->spotlight != NULL);
			COL fullIntensity = COL(1.0f, 1.0f, 1.0f);
			impl->spotlight->setSpotlightParams(fullIntensity);
			VC3 pos = impl->position;
			this->run(pos);
		}
	}


	void Flashlight::setBetaRotation(float betaAngle)
	{
		impl->betaAngle = betaAngle;
	}

	float Flashlight::getFlashlightIlluminationFactor()
	{
		if (impl->energy > SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW))
		{
			return 1.0f;
		} else {
			float lowFactor = (float)impl->energy / (float)SimpleOptions::getInt(DH_OPT_I_GAME_FLASHLIGHT_ENERGY_LOW);
			return lowFactor;
		}
	}


	bool Flashlight::doesNeedRecharge()
	{
		return impl->needRecharge;
	}

	int Flashlight::getRechargingAmount()
	{
		return impl->lastRecharge;
	}

	void Flashlight::setTemporaryBrightnessFactor(float brightness)
	{
		impl->tempBrightnessFactor = brightness;
	}

}


