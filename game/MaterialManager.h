
#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include <DatatypeDef.h>
#include "../game/materials.h"

namespace game
{
	class GameMap;

	class MaterialManager
	{
		public:
			// returns true if position is on top of given material.
			static bool isMaterialUnderPosition(GameMap *gameMap,
				const VC3 &position, int material);

			// return material under given position 
			static int getMaterialUnderPosition(GameMap *gameMap,
				const VC3 &position);
	};
}

#endif


