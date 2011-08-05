
#ifndef CONNECTIONCHECKER_H
#define CONNECTIONCHECKER_H

namespace game
{
	class GameScene;

	/**
	 * A simple connection checker utility to check whether some coordinate
	 * is connected to given center coordinate of the area.
	 * (Connected means that there is a path between the two points 
	 * within the given sized area)
	 * Basically, this can be used to get rough estimate of whether some
	 * two points are in the same room, or at the same side of some wall.
	 *
	 * @version 1.0, 18.6.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see Floodfill
	 * @see UnitFormation
	 */
  class ConnectionChecker
	{
	public:
		ConnectionChecker(GameScene *gameScene, int centerX, int centerY, int size);

		~ConnectionChecker();

		// x and y center based - (0,0) being center
		bool isCenterConnectedTo(int x, int y);

	private:
		unsigned char *mapData;
		int size;

	};
}

#endif

