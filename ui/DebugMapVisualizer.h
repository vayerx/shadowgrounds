
#ifndef DEBUGMAPVISUALIZER_H
#define DEBUGMAPVISUALIZER_H

namespace game
{
	class GameMap;
	class GameScene;
}

namespace ui
{
	class DebugMapVisualizer
	{
	public:
		static void visualizeObstacleMap(game::GameMap *gameMap, game::GameScene *gameScene, const VC3 &cameraPosition);
	};
}

#endif
