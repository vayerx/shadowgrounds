
#include "precompiled.h"

#include <stdio.h>
#include "GridOcclusionCuller.h"
#include "../system/Logger.h"

namespace util
{

void gridocc_check_singlebit_area_bits(GRIDOCCLUSIONCULLER_DATATYPE area)
{
	int bitsSet = 0;
	for (int i = 0; i < GRIDOCCLUSIONCULLER_MAX_AREAS; i++)
	{
		if ((area & (1<<i)) != 0)
			bitsSet++;
	}
	if (bitsSet != 1)
	{
		assert(!"gridocc_check_area_bits - area number is incorrect. (must be occlusion area bit mask with one bit set)");
	}
}


GridOcclusionCuller::GridOcclusionCuller(float scaledSizeX, float scaledSizeY, int sizeX, int sizeY)
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->scaledSizeX = scaledSizeX;
	this->scaledSizeY = scaledSizeY;
	this->scaledSizeHalvedX = scaledSizeX / 2;
	this->scaledSizeHalvedY = scaledSizeY / 2;
  this->scaledToOcclusionMultiplierX = (float)sizeX / scaledSizeX;
  this->scaledToOcclusionMultiplierY = (float)sizeY / scaledSizeY;

  float scaleX = scaledSizeX / (float)sizeX;
  float scaleY = scaledSizeY / (float)sizeY;
  this->scaledWellMinX = -(scaledSizeHalvedX - scaleX);
  this->scaledWellMinY = -(scaledSizeHalvedY - scaleY);
  this->scaledWellMaxX = scaledSizeHalvedX - scaleX;
  this->scaledWellMaxY = scaledSizeHalvedY - scaleY;

	visibleAreasMap = new GRIDOCCLUSIONCULLER_DATATYPE[sizeX * sizeY];
	cameraAreaMap = new GRIDOCCLUSIONCULLER_DATATYPE[sizeX * sizeY];
	for (int i = 0; i < sizeX * sizeY; i++)
	{
		visibleAreasMap[i] = GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK;
		cameraAreaMap[i] = GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK;
	}
}

GridOcclusionCuller::~GridOcclusionCuller()
{
	delete [] visibleAreasMap;
	visibleAreasMap = NULL;
	delete [] cameraAreaMap;
	cameraAreaMap = NULL;
}

void GridOcclusionCuller::makeCameraArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area)
{
	// only one bit must be on!
#ifndef NDEBUG
	gridocc_check_singlebit_area_bits(area);
#endif
	cameraAreaMap[x + y * sizeX] = area;
}

GRIDOCCLUSIONCULLER_DATATYPE GridOcclusionCuller::getCameraArea(int x, int y) const
{
	return cameraAreaMap[x + y * sizeX];
}

void GridOcclusionCuller::makeVisibleToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area)
{
#ifndef NDEBUG
	gridocc_check_singlebit_area_bits(area);
#endif
	visibleAreasMap[x + y * sizeX] |= area;
}

void GridOcclusionCuller::makeOccludedToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area)
{
#ifndef NDEBUG
	gridocc_check_singlebit_area_bits(area);
#endif
	visibleAreasMap[x + y * sizeX] &= (GRIDOCCLUSIONCULLER_ALL_AREAS_MASK ^ area);
}

bool GridOcclusionCuller::isVisibleToArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE area) const
{
#ifndef NDEBUG
	gridocc_check_singlebit_area_bits(area);
#endif
	if (visibleAreasMap[x + y * sizeX] & area)
		return true;
	else
		return false;
}

GRIDOCCLUSIONCULLER_DATATYPE GridOcclusionCuller::getAllVisibilitiesForArea(int x, int y) const
{
	return visibleAreasMap[x + y * sizeX];
}

void GridOcclusionCuller::setAllVisibilitiesForArea(int x, int y, GRIDOCCLUSIONCULLER_DATATYPE areasMask)
{
	visibleAreasMap[x + y * sizeX] = areasMask;
}

void GridOcclusionCuller::save(const char *filename) const
{
	FILE *f = fopen(filename, "wb");
	if (f != NULL)
	{
		int wrote1 = fwrite(cameraAreaMap, sizeX*sizeY*sizeof(GRIDOCCLUSIONCULLER_DATATYPE), 1, f);
		int wrote2 = fwrite(visibleAreasMap, sizeX*sizeY*sizeof(GRIDOCCLUSIONCULLER_DATATYPE), 1, f);
		if (wrote1 != 1 || wrote2 != 1)
		{
			Logger::getInstance()->error("GridOcclusionCuller::save - Error writing to file.");
			Logger::getInstance()->debug(filename);
		}
		fclose(f);
	} else {
		Logger::getInstance()->error("GridOcclusionCuller::save - Could not open file for writing.");
		Logger::getInstance()->debug(filename);
	}
}

void GridOcclusionCuller::load(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (f != NULL)
	{
		// TODO: assert that filesize == 2*sizeX*sizeY*sizeof(GRIDOCCLUSIONCULLER_DATATYPE)
		int read1 = fread(cameraAreaMap, sizeX*sizeY*sizeof(GRIDOCCLUSIONCULLER_DATATYPE), 1, f);
		int read2 = fread(visibleAreasMap, sizeX*sizeY*sizeof(GRIDOCCLUSIONCULLER_DATATYPE), 1, f);
		if (read1 != 1 || read2 != 1)
		{
			Logger::getInstance()->error("GridOcclusionCuller::load - Error reading from file.");
			Logger::getInstance()->debug(filename);
		}
		fclose(f);
	} else {
		Logger::getInstance()->error("GridOcclusionCuller::load - Could not open file for reading.");
		Logger::getInstance()->debug(filename);
	}
}

}
