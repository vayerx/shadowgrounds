
#include "precompiled.h"

#include "NoCameraBoundary.h"


namespace ui
{

	NoCameraBoundary::NoCameraBoundary()
	{
		// nop
	}



	NoCameraBoundary::~NoCameraBoundary()
	{
		// nop
	}


	bool NoCameraBoundary::isPositionInsideBoundaries(const VC3 &position)
	{
		return true;
	}


	VC3 NoCameraBoundary::getVectorToInsideBoundaries(const VC3 &position)
	{
		return VC3(0,0,0);
	}

}

