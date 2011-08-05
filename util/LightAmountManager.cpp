
#include "precompiled.h"

#include "LightAmountManager.h"

#include "SpotLightCalculator.h"
#include "LightMap.h"

// bad dependency... (util -> game)
// on the other hand, this class should not be in util at all,
// because it is a game specific class, not a common utility class.
// -jpk
#include "../game/GameMap.h"

#include <vector>

using namespace std;
using namespace boost;

namespace util {

LightAmountManager *LightAmountManager::instance = NULL;

struct LightAmountManager::Data
{
	mutable vector<weak_ptr<SpotLightCalculator> > spots;
	const game::GameMap *gameMap;
	const IStorm3D_Terrain *terrain;


	void add(boost::weak_ptr<SpotLightCalculator> light)
	{
		for(unsigned int i = 0; i < spots.size(); ++i)
			assert(spots[i].lock() != light.lock());

		spots.push_back(light);
	}


	void setMaps(const game::GameMap *gameMap, const IStorm3D_Terrain *terrain)
	{
		this->gameMap = gameMap;
		this->terrain = terrain;
	}


	float getDynamicLightAmount(const VC3 &position, ui::IVisualObjectData *& visualData, float height) const
	{
		float result = 0;

		vector<weak_ptr<SpotLightCalculator> >::iterator it = spots.begin();
		while(it != spots.end())
		{
			shared_ptr<SpotLightCalculator> spot = it->lock();
			if(!spot)
			{
				it = spots.erase(it);
				continue;
			}

			// ToDo: choose brightest light

			float current = spot->getLightAmount(position, *terrain, height);
			result += current;

			if(current > 0)
				visualData = spot->getData();

			++it;
		}

		return result;
	}

	float getStaticLightAmount(const VC3 &position, float height) const
	{
		int ox = gameMap->scaledToObstacleX(position.x);
		int oy = gameMap->scaledToObstacleY(position.z);
		unsigned char lAmount = gameMap->lightMap->getLightAmount(ox, oy);

		float ret = float(lAmount) / 255.0f;

		return ret;
	}

};


LightAmountManager *LightAmountManager::getInstance()
{
	if (instance == NULL)
	{
		instance = new LightAmountManager();
	}
	return instance;
}


void LightAmountManager::cleanInstance()
{
	if (instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
}


LightAmountManager::LightAmountManager()
:	data(new Data())
{
	// just to be sure...
	data->gameMap = 0;
	data->terrain = 0;
}

LightAmountManager::~LightAmountManager()
{
}

void LightAmountManager::add(boost::weak_ptr<SpotLightCalculator> light)
{
	data->add(light);
}

void LightAmountManager::setMaps(const game::GameMap *gameMap, const IStorm3D_Terrain *terrain)
{
	data->setMaps(gameMap, terrain);
}

float LightAmountManager::getDynamicLightAmount(const VC3 &position, ui::IVisualObjectData *& visualData, float height) const
{
	assert(data->terrain != 0);
	return data->getDynamicLightAmount(position, visualData, height);
}

float LightAmountManager::getLightAmount(const VC3 &position, float height) const
{
	ui::IVisualObjectData *visualData;
	float staticLight = data->getStaticLightAmount(position, height);
	float dynamicLight = data->getDynamicLightAmount(position, visualData, height);
	if (dynamicLight > staticLight)
		return dynamicLight;
	else
		return staticLight;
}

float LightAmountManager::getStaticLightAmount(const VC3 &position, float height) const
{
	assert(data->gameMap != 0);
	return data->getStaticLightAmount(position, height);
}

} // util
