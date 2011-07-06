
#ifndef QUADAREA_H
#define QUADAREA_H

#include "IArea.h"
#include "area_flags.h"

namespace game
{

class QuadArea : public IArea
{
public:
	virtual int getAreaClassId() { return AREA_CLASS_ID_QUAD; }

	virtual bool isInsideArea(const VC3 &position);

	virtual bool hasAreaFadeFactor() { return false; }

	virtual float getAreaFadeFactorForPosition(const VC3 &position) 
	{ 
		if (isInsideArea())
			return 1.0f;
		else
			return 0.0f;
	}

	virtual float getFactorForCorner(const VC3 &position, int cornerNumber);

};

}

#endif


