
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "ColorMap.h"
#include "AreaMap.h"
#include "../game/GameMap.h"
#include <stdio.h>
#include <boost/scoped_array.hpp>
#include "../util/Debug_MemoryManager.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"

using namespace frozenbyte;

namespace util {

struct ColorMapData
{
	game::GameMap *gameMap;

	VC2I resolution;
	COL indoorMultiplier;
	COL outdoorMultiplier;

	float scaledSizeX;
	float scaledSizeY;

	// NOTE: these scales are not the "scaled:colormap" scale - 
	// they are "scaled:heightmap" scale.
	float scaleX; 
	float scaleY;

	//float multiplier;

/*
  // 32 bit
	boost::scoped_array<DWORD> array;
*/
	boost::scoped_array<WORD> array;

	ColorMapData(game::GameMap *gameMap_, const char *fileName, float scaleX, float scaleY, float scaledSizeX, float scaledSizeY)
	:	gameMap(gameMap_),
		resolution(2048, 2048),
		indoorMultiplier(1.f, 1.f, 1.f),
		outdoorMultiplier(1.f, 1.f, 1.f)
	{
		this->scaleX = scaleX;
		this->scaleY = scaleY;
		this->scaledSizeX = scaledSizeX;
		this->scaledSizeY = scaledSizeY;

		if(!fileName)
			return;

		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(fileName);
		if(!stream.isEof())
		{
			int version = 0;
			stream >> version;

			stream >> resolution.x >> resolution.y;
			boost::scoped_array<WORD> tempArray(new WORD[resolution.x * resolution.y]);

			for(int i = 0; i < resolution.x * resolution.y; ++i)
			{
				unsigned char r = 0;
				unsigned char g = 0;
				unsigned char b = 0;

				stream >> r >> g >> b;

				WORD color = ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
				tempArray[i] = color;
			}

			if(resolution.x > 0 && resolution.y > 0)
				array.swap(tempArray);
		}

#if 0
		filesystem::FB_FILE *fp = fb_fopen(fileName, "rb");
		if(!fp)
			return;

		// 32 bit
		//boost::scoped_array<DWORD> tempArray(new DWORD[resolution.x * resolution.y]);
		boost::scoped_array<WORD> tempArray(new WORD[resolution.x * resolution.y]);

		for(int y = 0; y < resolution.y; ++y)
		for(int x = 0; x < resolution.x; ++x)
		{
			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;

			/*
			// 32 bit
			fread(&b, 1, 1, fp);
			fread(&g, 1, 1, fp);
			fread(&r, 1, 1, fp);

			DWORD color = (r << 16) | (g << 8) | b;
			tempArray[y * 2048 + x] = color;
			*/
			fread(&r, 1, 1, fp);
			fread(&g, 1, 1, fp);
			fread(&b, 1, 1, fp);

			WORD color = ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
			tempArray[y * 2048 + x] = color;
		}

		fclose(fp);
		array.swap(tempArray);
#endif
	}
};

ColorMap::ColorMap(game::GameMap *gameMap, const char *fileName, float scaleX, float scaleY, float scaledSizeX, float scaledSizeY)
{
	assert(gameMap && fileName);

	boost::scoped_ptr<ColorMapData> tempData(new ColorMapData(gameMap, fileName, scaleX, scaleY, scaledSizeX, scaledSizeY));
	data.swap(tempData);
}

ColorMap::~ColorMap()
{
}

void ColorMap::setMultiplier(ColorMap::Area area, float value)
{
	if(area == Indoor)
		data->indoorMultiplier = COL(value, value, value);
	else
		data->outdoorMultiplier = COL(value, value, value);
}

void ColorMap::setMultiplier(ColorMap::Area area, const COL &color)
{
	if(area == Indoor)
		data->indoorMultiplier = color;
	else
		data->outdoorMultiplier = color;
}

const COL &ColorMap::getMultiplier(ColorMap::Area area) const
{
	if(area == Indoor)
		return data->indoorMultiplier;
	else
		return data->outdoorMultiplier;
}


COL ColorMap::getUnmultipliedColor(float x, float y) const
{
	if(!data->array)
		return COL();

	if(x <= 0.01f || y <= 0.01f
		|| x >= 0.99f || y >= 0.99f)
	{
		return COL(0,0,0);
	}

	int mapX = int(x * float(data->resolution.x));
	int mapY = int(y * float(data->resolution.y));

	{
		if(mapX < 1)
			mapX = 1;
		if(mapY < 1)
			mapY = 1;
		//if(mapX >= data->resolution.y - 1)
		//	mapX = data->resolution.y - 2;
		//if(mapY >= data->resolution.x - 1)
		//	mapY = data->resolution.x - 2;
		if(mapX >= data->resolution.x - 1)
			mapX = data->resolution.x - 2;
		if(mapY >= data->resolution.y - 1)
			mapY = data->resolution.y - 2;
	}

	// the quick hack el cheapo interpolation...
	int mapX2 = int(x * float(data->resolution.x) + 0.5f);
	int mapY2 = mapY;

	int mapX3 = mapX;
	int mapY3 = int(y * float(data->resolution.y) + 0.5f);

	int index = mapY * data->resolution.x + mapX;
	assert(index >= 0 && index <= data->resolution.x * data->resolution.y);

	int index2 = mapY2 * data->resolution.x + mapX2;
	int index3 = mapY3 * data->resolution.x + mapX3;


	/*
	// 32 bit
	DWORD color = data->array[index];

	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	*/

	WORD color = data->array[index];
	int b = (color & 31);
	int g = ((color & 2047) >> 5);
	int r = ((color & 65535) >> 11);

	WORD color2 = data->array[index2];
	int b2 = (color2 & 31);
	int g2 = ((color2 & 2047) >> 5);
	int r2 = ((color2 & 65535) >> 11);

	WORD color3 = data->array[index3];
	int b3 = (color3 & 31);
	int g3 = ((color3 & 2047) >> 5);
	int r3 = ((color3 & 65535) >> 11);

	/*
	// 32 bit
	COL result(r / 255.f, g / 255.f, b / 255.f);
	*/
	// Interpolated 16bit
	COL result((r + r2 + r3) / 3.0f / 32.f, (g + g2 + g3) / 3.0f / 64.f, (b + b2 + b3) / 3.0f / 32.f);	

	return result;
}


COL ColorMap::getColor(float x, float y) const
{
	if(x <= 0 || x >= 0.995f || y <= 0 || y >= 0.995f)
		return COL();

	COL result = getUnmultipliedColor(x, y);

	float xw = data->scaledSizeX * (x - 0.5f);
	float yw = data->scaledSizeY * (y - 0.5f);
	int xi = data->gameMap->scaledToObstacleX(xw);
	int yi = data->gameMap->scaledToObstacleY(yw);

	if(data->gameMap->getAreaMap()->isAreaAnyValue(xi, yi, AREAMASK_INBUILDING))
		result *= data->indoorMultiplier;
	else
		result *= data->outdoorMultiplier;

	return result;
}

COL ColorMap::getColorAtScaled(float scaledX, float scaledY) const
{	 	 
	float x = scaledX / data->scaledSizeX + .5f;
	float y = scaledY / data->scaledSizeY + .5f;
	return getColor(x,y);
}

COL ColorMap::getUnmultipliedColorAtScaled(float scaledX, float scaledY) const
{	 	 
	float x = scaledX / data->scaledSizeX + .5f;
	float y = scaledY / data->scaledSizeY + .5f;
	return getUnmultipliedColor(x,y);
}

} // util
