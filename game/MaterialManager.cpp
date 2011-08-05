
#include "precompiled.h"

#include "MaterialManager.h"
#include "GameMap.h"
#include "../util/AreaMap.h"
#include "../game/areamasks.h"

namespace game
{
	int MaterialManager::getMaterialUnderPosition(GameMap *gameMap,
		const VC3 &position)
	{
		util::AreaMap *areaMap = gameMap->getAreaMap();
		assert (areaMap != NULL);

		int ox = gameMap->scaledToObstacleX(position.x);
		int oy = gameMap->scaledToObstacleY(position.z);

		int materialArea = (areaMap->getAreaValue(ox, oy, AREAMASK_MATERIAL) >> AREASHIFT_MATERIAL);
		assert(materialArea >= 0 && materialArea < MATERIAL_PALETTE_AMOUNT);

		int material = game::getMaterialByPalette(materialArea);
		assert(material >= 0 && material < MATERIAL_AMOUNT);

		return material;
	}


	bool MaterialManager::isMaterialUnderPosition(GameMap *gameMap,
		const VC3 &position, int material)
	{
		int mat = MaterialManager::getMaterialUnderPosition(gameMap, position);
		if (mat == material)
			return true;
		else
			return false;
	}

}


