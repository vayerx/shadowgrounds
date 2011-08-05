#ifndef INCLUDED_LIGHTAMOUNTMANAGER_H
#define INCLUDED_LIGHTAMOUNTMANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Terrain;
namespace ui {
	class IVisualObjectData;
} // ui

namespace game
{
	class GameMap;
}

namespace util {

class SpotLightCalculator;

class LightAmountManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

	static LightAmountManager *instance;

public:
	LightAmountManager();
	~LightAmountManager();

	static LightAmountManager *getInstance();
	static void cleanInstance();

	void add(boost::weak_ptr<SpotLightCalculator> light);

	void setMaps(const game::GameMap *gameMap, const IStorm3D_Terrain *terrain);

	float getDynamicLightAmount(const VC3 &position, ui::IVisualObjectData *&visualData, float height) const;
	float getStaticLightAmount(const VC3 &position, float height) const;
	float getLightAmount(const VC3 &position, float height) const;
};

} // util

#endif
