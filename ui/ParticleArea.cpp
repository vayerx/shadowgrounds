
#include "precompiled.h"

#include "ParticleArea.h"
#include "../game/GameMap.h"
#include "../game/CoverMap.h"
#include "../game/GameScene.h"
#include "../game/areamasks.h"
#include "../util/AreaMap.h"
#include <IStorm3D_Scene.h>

using namespace util;
using namespace game;

namespace ui {

ParticleArea::ParticleArea(game::GameMap *gameMap_, game::GameScene *scene_)
:	gameMap(gameMap_),
	scene(scene_),
	insideCheck(true)
{
	assert(gameMap);
}

ParticleArea::~ParticleArea()
{
}

void ParticleArea::biasValues(const VC3 &position, VC3 &velocity) const
{
	float wx = position.x;
	float wy = position.z;

	if(gameMap->isWellInScaledBoundaries(wx, wy))
	{
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		CoverMap *coverMap = gameMap->getCoverMap();
		CoverMap::COVER_DIRECTION dir = coverMap->getNearestCoverDirection(x, y);
		int distance = coverMap->getDistanceToNearestCover(x, y);

		{
			AreaMap *areaMap = gameMap->getAreaMap();
			int ax = x;
			int ay = y;

			if(dir == CoverMap::COVER_DIRECTION_N)
				ay -= distance;
			else if(dir == CoverMap::COVER_DIRECTION_NE)
			{
				int df = int(distance * 0.7071f);
				ax += df;
				ay -= df;
			}
			else if(dir == CoverMap::COVER_DIRECTION_E)
				ax += distance;
			else if(dir == CoverMap::COVER_DIRECTION_SE)
			{
				int df = int(distance * 0.7071f);
				ax += df;
				ay += df;
			}
			else if(dir == CoverMap::COVER_DIRECTION_S)
				ay += distance;
			else if(dir == CoverMap::COVER_DIRECTION_SW)
			{
				int df = int(distance * 0.7071f);
				ax -= df;
				ay += df;
			}
			else if(dir == CoverMap::COVER_DIRECTION_W)
				ax -= distance;
			else if(dir == CoverMap::COVER_DIRECTION_NW)
			{
				int df = int(distance * 0.7071f);
				ax -= df;
				ay -= df;
			}

			if(areaMap->isAreaAnyValue(ax, ay, AREAMASK_OBSTACLE_UNHITTABLE) || gameMap->getObstacleHeight(ax, ay) < 20)
				return;
		}

		if(distance > 0 && distance < 8)
		{
			float blendFactor = 1.f - (distance / 8.f);
			blendFactor *= 0.04f;
			float angle = ((dir - 1) * 45.f) * PI / 180.f;
			VC3 blendVector;
			blendVector.x = -sinf(angle);
			blendVector.z = cosf(angle);

			blendVector *= velocity.GetLength();
			blendVector *= blendFactor;
			velocity *= 1.f - blendFactor;
			velocity += blendVector;
		}
	}
}

float ParticleArea::getObstacleHeight(const VC3 &position) const
{
	float wx = position.x;
	float wy = position.z;

	if(gameMap->isWellInScaledBoundaries(wx, wy))
	{
		AreaMap *areaMap = gameMap->getAreaMap();
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		int hx = x / GAMEMAP_PATHFIND_ACCURACY;
		int hy = y / GAMEMAP_PATHFIND_ACCURACY;

		int height = gameMap->getHeightmapHeightAt(hx, hy);
		if(!areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_SEETHROUGH))
			height += gameMap->getObstacleHeight(x, y);

		return height * gameMap->getScaleHeight();
		//return (gameMap->getObstacleHeight(x, y) + gameMap->getHeightmapHeightAt(hx, hy)) * gameMap->getScaleHeight();
	}

	return 0.f;
}

float ParticleArea::getBaseHeight(const VC3 &position) const
{
	float wx = position.x;
	float wy = position.z;

	if(gameMap->isWellInScaledBoundaries(wx, wy))
	{
		int hx = gameMap->scaledToHeightmapX(wx);
		int hy = gameMap->scaledToHeightmapY(wy);

		return gameMap->getHeightmapHeightAt(hx, hy) * gameMap->getScaleHeight();
	}

	return 0.f;
}

/*
bool ParticleArea::isInsideObstacle(const VC3 &position) const
{
	float wx = position.x;
	float wy = position.z;

	if(gameMap->isWellInScaledBoundaries(wx, wy))
	{
		AreaMap *areaMap = gameMap->getAreaMap();
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);
		int xh = gameMap->scaledToHeightmapX(wx);
		int yh = gameMap->scaledToHeightmapY(wy);

		int height = gameMap->getObstacleHeight(x, y) + gameMap->getHeightmapHeightAt(xh, yh);
		if(height > position.y / gameMap->getScaleHeight())
			return true;
	}

	return false;
}
*/
bool ParticleArea::isInside(const VC3 &pos) const
{
	if(!insideCheck)
		return false;

	float wx = pos.x;
	float wy = pos.z;

	if(gameMap->isWellInScaledBoundaries(wx, wy))
	{
		AreaMap *areaMap = gameMap->getAreaMap();

		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		if(areaMap->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
			return true;
	}

	return false;
}

void ParticleArea::enableInsideCheck(bool enable)
{
	insideCheck = enable;
}

} // ui
