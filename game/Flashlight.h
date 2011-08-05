
#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include <DatatypeDef.h>

namespace util
{
	class ColorMap;
}

namespace ui
{
	class VisualObject;
}

namespace game
{
	class Game;

	class FlashlightImpl;

	class Flashlight
	{
		public:
			// TODO: replace colormap with the alienscare lightmap??
			/**
			 * @param VisualObject* origin, the originating visual object
			 *        that the flashlight is attached to (the object will
			 *        not be drawn in the flashlight's shadowmap)
			 */
			Flashlight(Game *game, ui::VisualObject *origin);

			~Flashlight();

			void resetOrigin(ui::VisualObject *origin);

			void run(const VC3 &position);

			void setTemporaryBrightnessFactor(float brightness);

			void prepareForRender();

			void setFlashlightOperable(bool operable);

			void setRotation(float angle);
			void setRotationToward(float angle, int timeElapsed);

			void setBetaRotation(float betaAngle);

			void setOffset(float offset);

			void setSwayFactor(float factor);
			void setSwayTime(int time);

			void setShakeFactor(float factor);
			void setShakeTime(int time);

			void setImpact(float factor);


			void toggleOn();
			bool isFlashlightOn();
			void setFlashlightOn(bool flashlightOn);

			int getFlashlightEnergy();
			void setFlashlightEnergy(int energyPercentage);

			float getFlashlightIlluminationFactor();

			//void setLightRecharge(bool lightRecharge);
			//void setAutomaticRecharge(bool automaticRecharge);
			//void setConsumesEnergy(bool consumesEnergy);

			//void setFlashlightEnergyBonus(bool energyBonus);

			bool doesNeedRecharge();
			int getRechargingAmount();

		private:
			FlashlightImpl *impl;

	};

}

#endif


