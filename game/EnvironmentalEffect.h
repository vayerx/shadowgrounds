#ifndef ENVIRONMENTALEFFECT_H
#define ENVIRONMENTALEFFECT_H

#include <string>
#include <DatatypeDef.h>

namespace ui {
	class VisualEffect;
}

namespace game
{
	class EnvironmentalEffectManager;
	struct EnvironmentalEffectManagerImpl;

	class EnvironmentalEffect
	{
		std::string filename;
		ui::VisualEffect *visualEffect;
		VC3 position;

		friend class EnvironmentalEffectManager;
		friend struct EnvironmentalEffectManagerImpl;

		enum FadeState
		{
			None,
			FadeIn,
			FadeOut
		};

		FadeState state;
		int time;
		int fadeTime;

	public:
		EnvironmentalEffect(const char *particleFilename, ui::VisualEffect *visualEffect);
		~EnvironmentalEffect();

		bool isFinished() const;

		void fadeIn(int time);
		void fadeOut(int time);
		void setPosition(const VC3 &position);
		void update(int ms);
	};
}

#endif
