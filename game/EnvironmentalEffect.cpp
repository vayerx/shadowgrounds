
#include "precompiled.h"

#include "EnvironmentalEffect.h"
#include "../ui/VisualEffect.h"
#include "../util/Debug_MemoryManager.h"

namespace game {

EnvironmentalEffect::EnvironmentalEffect(const char *particleFilename, ui::VisualEffect *visualEffect_)
:	filename(particleFilename),
	visualEffect(visualEffect_),
	state(None),
	time(0),
	fadeTime(1)
{
	assert(particleFilename);
}

EnvironmentalEffect::~EnvironmentalEffect()
{
	visualEffect->setDeleteFlag();
	visualEffect->freeReference();
	visualEffect = NULL;
}

bool EnvironmentalEffect::isFinished() const
{
	if(state == FadeOut && time == fadeTime)
		return true;

	return false;
}

void EnvironmentalEffect::fadeIn(int fadeTime_)
{
	state = FadeIn;
	time = 0;
	fadeTime = fadeTime_;
}

void EnvironmentalEffect::fadeOut(int fadeTime_)
{
	state = FadeOut;
	time = 0;
	fadeTime = fadeTime_;
}

void EnvironmentalEffect::setPosition(const VC3 &position_)
{
	position = position_;
	if(visualEffect)
		visualEffect->setPosition(position);
}

void EnvironmentalEffect::update(int ms)
{
	if(state == None)
		return;

	time += ms;

	if(visualEffect)
	{
		float factor = float(time) / fadeTime;
		if(state == FadeIn)
		{
			if(factor > 1.0f)
			{
				factor = 1.0f;
				state = None;
			}
		}
		else if(state == FadeOut)
		{
			factor = 1.f - factor;
			if(factor < 0.0f)
			{
				factor = 0.0f;
				state = None;
			}
		}

		visualEffect->setParticleEffectEmitRate(factor);
	}
}

} // game
