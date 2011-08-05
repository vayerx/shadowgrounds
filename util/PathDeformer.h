
#ifndef PATHDEFORMER_H
#define PATHDEFORMER_H

namespace game
{
	class GameMap;
}

namespace frozenbyte
{
  namespace ai
	{
		class PathFind;
		class Path;
	}
}

class IStorm3D_Terrain;

namespace util
{
	class PathDeformer
	{
		public:
			/**
			 * Attempts to deform the given path (modify a bit) to
			 * make the path avoid light.
			 * 
			 */
			static bool deformForDarkness(
				frozenbyte::ai::PathFind *pathfinder, frozenbyte::ai::Path *path,
				int pathIndex,
				IStorm3D_Terrain *terrain, game::GameMap *gameMap, float height);
	};
}


#endif

