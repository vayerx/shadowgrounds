#ifndef INCLUDED_DIRECTIONAL_LIGHT_H
#define INCLUDED_DIRECTIONAL_LIGHT_H

#include <DatatypeDef.h>

namespace util {

class DirectionalLight
{
	struct Flash
	{
		VC3 direction;

		int fadeInTime;
		int stayTime;
		int fadeOutTime;
		int time;

		Flash();
	};

	VC3 sunDirection;
	float sunStrength;
	Flash flash;

public:
	DirectionalLight();
	~DirectionalLight();

	// ToDo: add color to directional
	void setSun(const VC3 &dir, float strength);
	void addFlash(const VC3 &direction, int fadeInTime, int stayTime, int fadeOutTime);
	void update(int ms);

	void getResult(VC3 &direction, float &strength) const;
};

} // util

#endif
