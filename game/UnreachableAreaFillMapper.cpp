
#include "precompiled.h"

#include "UnreachableAreaFillMapper.h"
#include "GameMap.h"

#ifdef PROJECT_AOV
#define UNREACHABLE_OBSTACLE_HEIGHT 3.7f
#else
#define UNREACHABLE_OBSTACLE_HEIGHT 2.0f
#endif


namespace game
{
	UnreachableAreaFillMapper::UnreachableAreaFillMapper(GameMap *data)
	{
		assert(data != NULL);
		this->data = data;
		this->fillmap = new unsigned char[data->getObstacleSizeX() * data->getObstacleSizeY()];

		init();
	}

	UnreachableAreaFillMapper::~UnreachableAreaFillMapper()
	{
		delete [] fillmap;
	}

	unsigned char UnreachableAreaFillMapper::getByte(int x, int y)
	{
		assert(x >= 0 && y >= 0 && x < data->getObstacleSizeX() && y < data->getObstacleSizeY());
		return fillmap[x + y * data->getObstacleSizeX()];
	}

	void UnreachableAreaFillMapper::setByte(int x, int y, unsigned char value)
	{
		assert(x >= 0 && y >= 0 && x < data->getObstacleSizeX() && y < data->getObstacleSizeY());
		fillmap[x + y * data->getObstacleSizeX()] = value;
	}

	void UnreachableAreaFillMapper::init()
	{
		int sizex = data->getObstacleSizeX();
		int sizey = data->getObstacleSizeY();

		for (int y = 0; y < sizey; y++)
		{
			for (int x = 0; x < sizex; x++)
			{
				if (data->getObstacleHeight(x, y) == 0
					|| data->isMovingObstacle(x, y))
				{
					fillmap[x + y * sizex] = UNR_VALUE_EMPTY;
				}
				else 
				{
					fillmap[x + y * sizex] = UNR_VALUE_OBSTACLE;
				}
			}
		}
	}

	void UnreachableAreaFillMapper::addReachablePoint(int x, int y)
	{
		if (x >= 0 && y >= 0 && x < data->getObstacleSizeX() && y < data->getObstacleSizeY())
		{
			fillmap[x + y * data->getObstacleSizeX()] = UNR_VALUE_REACHABLE;
		} else {
			// out of map!
			assert(!"UnreachableAreaFillMapper::addReachablePoint - Coordinates out of bounds.");
		}
	}

	void UnreachableAreaFillMapper::applyResult()
	{
		int sizex = data->getObstacleSizeX();
		int sizey = data->getObstacleSizeY();
		float heightScale = data->getScaleHeight();

		for (int y = 0; y < sizey; y++)
		{
			for (int x = 0; x < sizex; x++)
			{
				if (fillmap[x + y * sizex] == UNR_VALUE_EMPTY
					&& data->getObstacleHeight(x, y) == 0)
				{
					data->addObstacleHeight(x, y, (int)(UNREACHABLE_OBSTACLE_HEIGHT / heightScale), AREAVALUE_OBSTACLE_TERRAINOBJECT);
				}
			}
		}
	}

}
