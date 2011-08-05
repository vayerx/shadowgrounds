
#ifndef BUILDINGADDER_H
#define BUILDINGADDER_H

#include <DatatypeDef.h>

namespace game
{
	class Game;
	class GamePhysics;
	class Building;

	class BuildingAdder
	{
		public:
			static void addBuilding(Game *game, const VC3 &position, const char *filename, GamePhysics *gamePhysics);

			static void createVisualForBuilding(Game *game, Building *building);

			static void setTerrainCut(bool enableTerrainCut);
	};
}

#endif



