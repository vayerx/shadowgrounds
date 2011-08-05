#ifndef INCLUDED_COLOR_MAP_H
#define INCLUDED_COLOR_MAP_H

#include <DatatypeDef.h>
#include <boost/scoped_ptr.hpp>

namespace game {

class GameMap;

} // game

namespace util {

struct ColorMapData;

class ColorMap
{
	boost::scoped_ptr<ColorMapData> data;

public:
	ColorMap(game::GameMap *gameMap, const char *fileName, float scaleX, float scaleY, float scaledSizeX, float scaledSizeY);
	~ColorMap();

	enum Area
	{
		Indoor,
		Outdoor
	};

	void setMultiplier(Area area, float value);
	void setMultiplier(Area area, const COL &color);

	const COL &getMultiplier(Area area) const;

	// Normalized coordinates. 
	// Result should be filtered(?)
	COL getColor(float x, float y) const;
	COL getColorAtScaled(float scaledX, float scaledY) const;

	// getting without using the multiplier (the original values)
	COL getUnmultipliedColor(float x, float y) const;
	COL getUnmultipliedColorAtScaled(float scaledX, float scaledY) const;
};

} // util

#endif
