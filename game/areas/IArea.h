
#ifndef IAREA_H
#define IAREA_H

namespace game
{

class IArea
{
public:
	// should return the unique implementing class id number for the area
	// (or that of a base class, say AREA_CLASS_ID_QUAD if inherited from the QuadArea class)
	virtual int getAreaClassId() = 0;
	
	// should return true if this the class implements the IPrioritizedArea interface
	virtual bool isPrioritized() = 0;

	// returns true if the given position inside the area
	virtual bool isInsideArea(const VC3 &position) = 0;

	// should return true if the area somehow properly supports fade factors
	virtual bool hasAreaFadeFactor() = 0;

	// should return 0 when outside area
	// should return a value of 0.0 (preferably exclusive) - 1.0 (inclusive) when inside area depending
	// on area's fade-out properties
	virtual float getAreaFadeFactorForPosition(const VC3 &position) = 0;

	// this should return the amount of unified handles for this area
	// for example, the QuadArea will have 5 handles, the "center" (the actual area) and 4 corners
	virtual int getHandleAmount() = 0;

	// TODO: this might be useful, but may be difficult to provide for all types of areas...
	// distance to enter the area, if outside 
	// (and possibly distance to exit area as negative value if inside area)
	// virtual float getDistanceToAreaBorder(const VC3 &position) = 0;
};

}

#endif


