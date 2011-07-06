
#ifndef QUADAREA_H
#define QUADAREA_H

#include "IArea.h"
#include "area_flags.h"

#define QUADAREA_CORNER_NUMBER_UPPER_LEFT 0
#define QUADAREA_CORNER_NUMBER_UPPER_RIGHT 1
#define QUADAREA_CORNER_NUMBER_LOWER_RIGHT 2
#define QUADAREA_CORNER_NUMBER_LOWER_LEFT 3

#define QUADAREA_CORNER_NUMBER_AMOUNT 4

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

#endif


