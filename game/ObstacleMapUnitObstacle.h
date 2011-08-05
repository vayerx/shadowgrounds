
#ifndef OBSTACLEMAPUNITOBSTACLE_H
#define OBSTACLEMAPUNITOBSTACLE_H

namespace game
{
	class Game;
	class Unit;

	class ObstacleMapUnitObstacle
	{
		public:
			static void addObstacle(Game *game, Unit *unit, int radius, 
				int height, bool lineblock, bool leftside, bool rightside, int lineWidth);

			static void removeObstacle(Game *game, Unit *unit, int radius, 
				int height, bool lineblock, bool leftside, bool rightside, int lineWidth);

			static void moveObstacle(Game *game, Unit *unit, int x, int y,
				int radius, int height, bool lineblock, bool leftside, bool rightside, int lineWidth);
	};

}

#endif


