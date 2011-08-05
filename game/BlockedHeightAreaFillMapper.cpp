
#include "precompiled.h"

#include "BlockedHeightAreaFillMapper.h"
#include "GameMap.h"


namespace game
{
	BlockedHeightAreaFillMapper::BlockedHeightAreaFillMapper(GameMap *data, int height)
	{
		this->height = height;
		this->data = data;
	}

	unsigned char BlockedHeightAreaFillMapper::getByte(int x, int y)
	{
		if (data->getHeightmapHeightAt(x,y) != height)
		{
			return 0;
		}

		int ox = x * GAMEMAP_PATHFIND_ACCURACY;
		int oy = y * GAMEMAP_PATHFIND_ACCURACY;

		for (int ty = 0; ty < GAMEMAP_PATHFIND_ACCURACY; ty++)
		{
			for (int tx = 0; tx < GAMEMAP_PATHFIND_ACCURACY; tx++)
			{
				if (data->getObstacleHeight(ox + tx, oy + ty) <= 200)
				{
					// please, do floodfill me.
					return 1;
				}
			}
		}

		// this has already been floodfilled 
		// (or just otherwise fully blocked)
		return 0;
	}

	void BlockedHeightAreaFillMapper::setByte(int x, int y, unsigned char value)
	{
		int ox = x * GAMEMAP_PATHFIND_ACCURACY;
		int oy = y * GAMEMAP_PATHFIND_ACCURACY;

		int obstSizeX = data->getObstacleSizeX();
		int obstSizeY = data->getObstacleSizeY();

		for (int ty = -1; ty < GAMEMAP_PATHFIND_ACCURACY+1; ty++)
		{
			for (int tx = -1; tx < GAMEMAP_PATHFIND_ACCURACY+1; tx++)
			{
				if (ox + tx >= 0 && ox + tx < obstSizeX
					&& oy + ty >= 0 && oy + ty < obstSizeY)
				{
					// HACK: just using some strange number for the height :)
					// in this case, 200 seems to be somewhat nice...
					if (data->getObstacleHeight(ox + tx, oy + ty) <= 200)
					{
						data->addObstacleHeight(ox + tx, oy + ty, 201, AREAVALUE_OBSTACLE_TERRAINOBJECT);
						assert(data->getObstacleHeight(ox + tx, oy + ty) > 200);
					}
				}
			}
		}
	}


}
