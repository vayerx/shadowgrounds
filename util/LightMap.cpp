
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "LightMap.h"

#include "ColorMap.h"
#include "assert.h"

namespace util
{

	LightMap::LightMap(ColorMap *colorMap, int sizeX, int sizeY,
		float scaledSizeX, float scaledSizeY)
	{
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->scaledSizeX = scaledSizeX;
		this->scaledSizeY = scaledSizeY;
		this->colorMap = colorMap;

		// (what about inaccuracies in float calc?)
		FB_ASSERT(scaledSizeX / (float)sizeX == scaledSizeY / (float)sizeY);

		this->lightMap = new unsigned char[sizeX * sizeY];
		for (int y = 0; y < sizeY; y++)
		{
			for (int x = 0; x < sizeX; x++)
			{
			  COL c = colorMap->getUnmultipliedColor(float(x) / float(sizeX), float(y) / float(sizeY));
				int colAvg = ((int)(c.r*255.0f) + (int)(c.g*255.0f) + (int)(c.b*255.0f)) / 3;
				lightMap[x + y * sizeX] = colAvg;
			}
		}
	}


	LightMap::~LightMap()
	{
		delete [] lightMap;
	}


	unsigned char LightMap::getLightAmount(int lightX, int lightY) const
	{
		FB_ASSERT(lightX >= 0 && lightY >= 0);
		FB_ASSERT(lightX < sizeX && lightY < sizeY);

		return lightMap[lightX + lightY * sizeX];
	}


	void LightMap::modifySemiDynamicLightImpl(int lightX, int lightY, int radius, bool add, float brightness)
	{
		if(!(lightX - radius >= 0 && lightY - radius >= 0
			&& lightX + radius < sizeX && lightY + radius < sizeY))
		{
			FB_ASSERT(!"LightMap::modifySemiDynamicLightImpl - position with radius goes out of bounds.");
			return;
		}

		int maxLight = int(255.0f * brightness);

		for (int ty = lightY - radius; ty < lightY + radius + 1; ty++)
		{
			for (int tx = lightX - radius; tx < lightX + radius + 1; tx++)
			{
				int dx = tx - lightX;
				int dy = ty - lightY;
				int dSq = dx * dx + dy * dy;
				if (dSq <= radius * radius)
				{
					// linear fade, 255 (at 0 distance) -> 0 (at radius distance)
					int val = lightMap[tx + ty * sizeX];
					if (add)
					{
						val += (maxLight * (radius - (int)sqrtf((float)dSq))) / radius;
						if (val > 255) val = 255;
					} else {
						val -= (maxLight * (radius - (int)sqrtf((float)dSq))) / radius;
						if (val < 0) val = 0;
					}
					lightMap[tx + ty * sizeX] = val;
				}
			}
		}

	}


	void LightMap::addSemiDynamicLight(int lightX, int lightY, int radius, float brightness)
	{
		modifySemiDynamicLightImpl(lightX, lightY, radius, true, brightness);
	}


	void LightMap::removeSemiDynamicLight(int lightX, int lightY, int radius, float brightness)
	{
		modifySemiDynamicLightImpl(lightX, lightY, radius, false, brightness);
	}


	int LightMap::scaledToLightMapX(float scaledX) const
	{
		return (int)(((scaledX / scaledSizeX) + .5f) * (float)sizeX);
	}


	int LightMap::scaledToLightMapY(float scaledY) const
	{
		return (int)(((scaledY / scaledSizeY) + .5f) * (float)sizeY);
	}


	int LightMap::scaledToLightMapRadius(float scaledRadius) const
	{
		// NOTE: assuming x and y scaling factors to be the same!
		return (int)((scaledRadius / scaledSizeX) * (float)sizeX);
	}


}

