
#ifndef LIGHTBLINKER_H
#define LIGHTBLINKER_H

#include <DatatypeDef.h>

namespace game
{
	class Game;
	class LightBlinkerImpl;

	class LightBlinker
	{
		public:
			enum LightBlinkType
			{
				LightBlinkTypeSinWave = 1,
				LightBlinkTypeBiasColor2 = 2
			};

			LightBlinker(Game *game, bool outdoor);

			void run();

			void enable();
			void disable();

			void setBlinkingLightType(LightBlinkType blinkType);

			void setBlinkingLightColorDefault(COL col);
			void setBlinkingLightColor1(COL col);
			void setBlinkingLightColor2(COL col);

			void setBlinkingLightColor1Red(float value);
			void setBlinkingLightColor1Green(float value);
			void setBlinkingLightColor1Blue(float value);

			void setBlinkingLightColor2Red(float value);
			void setBlinkingLightColor2Green(float value);
			void setBlinkingLightColor2Blue(float value);

			void setBlinkingLightRandomVariation(float value);
			void setBlinkingLightStrength(float value);
			// not really frequency, actually "cycle time in millisec" 
			void setBlinkingLightFrequency(int cycleTime);

			void setBlinkingLightPauseTime(int pauseTime);
			void setBlinkingLightPauseTimeRandomVariation(int pauseTimeRandomVariation);

			void setBlinkingLightRandomTime(int randomTime);

		private:
			LightBlinkerImpl *impl;
	};
}


#endif



