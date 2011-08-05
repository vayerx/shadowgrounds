
#ifndef ENVIRONMENTALEFFECTMANAGER_H
#define ENVIRONMENTALEFFECTMANAGER_H

#include <DatatypeDef.h>

namespace ui
{
	class VisualEffectManager;
}

namespace game
{
	struct EnvironmentalEffectManagerImpl;
	class Game;

	class EnvironmentalEffectManager
	{
		EnvironmentalEffectManagerImpl *impl;

	public:
		EnvironmentalEffectManager(Game *game, ui::VisualEffectManager *visualEffectManager);
		~EnvironmentalEffectManager();

		void addParticleEffect(const char *particleFilename, bool fade);
		void removeParticleEffectByFilename(const char *particleFilename, bool fade);
		void fadeOutAllParticleEffects(int time = -1);
		void fadeInAllParticleEffects(int time = -1);
		void removeAllParticleEffects();

		void setEffectGroup(const char *group);
		void fadeEffectGroup(const char *group);

		void init();
		void run();

		void disableSunlight();
		void enableSunlight();
		void setSunlightDirection(const VC3 &direction);
		void setSunlightColor(const COL &color);

		// updates the sunlight so that it is properly focused
		// to current view.. (as it is not really a directional
		// light, but a spotlight instead)
		void updateSunlightFocus();
	};
}

#endif
