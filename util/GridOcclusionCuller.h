
#ifndef GRIDOCCLUSIONCULLER_H
#define GRIDOCCLUSIONCULLER_H

#include <assert.h>

// unsigned long gives us a max of 32 (but using 30 just to be safe with signedness, shift bugs, etc.)
#define GRIDOCCLUSIONCULLER_MAX_AREAS 30
#define GRIDOCCLUSIONCULLER_ALL_AREAS_MASK 0xffffffff
#define GRIDOCCLUSIONCULLER_DATATYPE unsigned long

#define GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK 1
#define GRIDOCCLUSIONCULLER_DEFAULT_AREA_ORDER_NUMBER 0

namespace util
{

class GridOcclusionCuller
{
public:
	GridOcclusionCuller(float scaledSizeX, float scaledSizeY, int sizeX, int sizeY);
	~GridOcclusionCuller();

  inline int scaledToOcclusionX(float scaledX) const
  {
    return (int)((scaledX + scaledSizeHalvedX) * scaledToOcclusionMultiplierX);
  }

  inline int scaledToOcclusionY(float scaledY) const
  {
    return (int)((scaledY + scaledSizeHalvedY) * scaledToOcclusionMultiplierY);
  }

  bool isInOcclusionBoundaries(int x, int y) const
  {
		if (x >= 0 && x < sizeX
			&& y >= 0 && y < sizeY)
			return true;
		else
			return false;
  }

  inline bool isWellInScaledBoundaries(float scaledX, float scaledY) const
  {
    // notice: excluding the exact boundary points
    if (scaledX > scaledWellMinX && scaledX < scaledWellMaxX
      && scaledY > scaledWellMinY && scaledY < scaledWellMaxY)
      return true;
    else 
      return false;
  }


	// translates camera order number to actual area number (which is a bit mask)
	// order numbers are from 0 to GRIDOCCLUSIONCULLER_MAX_AREAS 
	// all other functions take in the actual area number, the single (or multi) bit mask.
	GRIDOCCLUSIONCULLER_DATATYPE getAreaForOrderNumber(int areaOrderNumber) const
	{
		assert(areaOrderNumber >= 0 && areaOrderNumber < GRIDOCCLUSIONCULLER_MAX_AREAS);
		return (1<<areaOrderNumber);
	}

	void makeCameraArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area);
	GRIDOCCLUSIONCULLER_DATATYPE getCameraArea(int x, int y) const;

	void makeVisibleToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area);
	void makeOccludedToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area);
	bool isVisibleToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area) const;

	GRIDOCCLUSIONCULLER_DATATYPE getAllVisibilitiesForArea(int x, int y) const;
	void setAllVisibilitiesForArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE areasMask);

	void save(const char *filename) const;
	void load(const char *filename);

private:
	GRIDOCCLUSIONCULLER_DATATYPE *visibleAreasMap;
	GRIDOCCLUSIONCULLER_DATATYPE *cameraAreaMap;
	int sizeX;
	int sizeY;
	float scaledSizeX;
	float scaledSizeY;
	float scaledSizeHalvedX;
	float scaledSizeHalvedY;
	float scaledToOcclusionMultiplierX;
	float scaledToOcclusionMultiplierY;
  float scaledWellMinX;
  float scaledWellMinY;
  float scaledWellMaxX;
  float scaledWellMaxY;
};

}


#endif
