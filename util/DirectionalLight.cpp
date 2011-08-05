
#include "precompiled.h"

#include "DirectionalLight.h"

namespace util {

DirectionalLight::Flash::Flash()
:	fadeInTime(0),
	stayTime(0),
	fadeOutTime(0),
	time(0)
{
}

DirectionalLight::DirectionalLight()
:	sunStrength(0)
{
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::setSun(const VC3 &dir, float strength)
{
	sunDirection = dir;
	sunStrength = strength;
}

void DirectionalLight::addFlash(const VC3 &direction, int fadeInTime, int stayTime, int fadeOutTime)
{
	flash.direction = direction;
	flash.fadeInTime = fadeInTime;
	flash.stayTime = flash.fadeInTime + stayTime;
	flash.fadeOutTime = flash.stayTime + fadeOutTime;
	flash.time = 0;
}

void DirectionalLight::update(int ms)
{
	flash.time += ms;
}

void DirectionalLight::getResult(VC3 &direction, float &strength) const
{
	float factor = 1.f;
	direction = sunDirection;
	strength = sunStrength;

	// No flash
	if(flash.time >= flash.fadeOutTime)
	{
		return;
	}
	// Fading out
	else if(flash.time >= flash.stayTime)
	{
		int fadeTime = flash.fadeOutTime - flash.stayTime;
		int currentTime = flash.time - flash.stayTime;
		factor = 1.f - (float(currentTime) / float(fadeTime));
	}
	// Flash at max
	else if(flash.time >= flash.fadeInTime)
	{
	}
	// Fading in
	else
	{
		int fadeTime = flash.fadeInTime;
		int currentTime = flash.time;
		factor = float(currentTime) / float(fadeTime);
	}

	factor *= .5f;
	direction *= 1.f - factor;
	direction += (flash.direction * factor);
}

} // util
