
#include "precompiled.h"

#include "ConnectionChecker.h"
#include "GameScene.h"
#include "GameMap.h"
#include "../util/Floodfill.h"

#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


#define CONNCHK_UNCONNECTED 0
#define CONNCHK_WALL 1
#define CONNCHK_CONNECTED 2

namespace game
{

	ConnectionChecker::ConnectionChecker(GameScene *gameScene, int centerX, int centerY, int size)
	{
		this->size = size;
		mapData = new unsigned char[size * size * 4];

		// first create walls
		for (int y = -size; y < size; y++)
		{
			for (int x = -size; x < size; x++)
			{				
				int maptype = CONNCHK_UNCONNECTED;
				if (!gameScene->getGameMap()->inPathfindBoundaries(centerX + x, centerY + y))
				{
					maptype = CONNCHK_WALL;
				} else {
					if (gameScene->isBlocked(centerX + x, centerY + y, 0))
						maptype = CONNCHK_WALL;
				}
				mapData[x + size + (y + size) * size*2] = maptype;
			}
		}

		// center always connected
		mapData[size + size * size*2] = CONNCHK_CONNECTED;

		// then floodfill connected areas
		util::Floodfill::fillWithByte(
			CONNCHK_CONNECTED, CONNCHK_UNCONNECTED, size*2, size*2, mapData);
	}


	ConnectionChecker::~ConnectionChecker()
	{
		if (mapData != NULL)
		{
			delete [] mapData;
			mapData = NULL;
		}
	}


	bool ConnectionChecker::isCenterConnectedTo(int x, int y)
	{
		if (x < -size || x >= size || y < -size || y >= size)
		{
			// not within area
			return false;
		}

		if (mapData[x + size + (y + size) * size*2] == CONNCHK_CONNECTED)
			return true;
		else
			return false;
	}	

}


