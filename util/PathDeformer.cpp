
#include "precompiled.h"

#include "PathDeformer.h"

#include "AI_PathFind.h"
#include "LightAmountManager.h"
#include "../game/GameMap.h"
#include "../game/CoverMap.h"
#include "../ui/VisualObject.h"

// TEMP: for debugging
#include <stdio.h>
#include "../system/Logger.h"

class IStorm3D_Terrain;

namespace util
{
	bool PathDeformer::deformForDarkness(
		frozenbyte::ai::PathFind *pathfinder, frozenbyte::ai::Path *path, 
		int pathIndex,
		IStorm3D_Terrain *terrain, game::GameMap *gameMap, float height)
	{
		int pathSize = path->getSize();
		VC3 deformVec = VC3(0,0,0);
    bool deformed = false;
		int deformedAmount = 0;

		for (int i = pathIndex; i < pathSize; i++)
		{
			VC3 pos;
			VC3 otherPos;
			pos.x = gameMap->pathfindToScaledX(path->getPointX(i));
			pos.z = gameMap->pathfindToScaledY(path->getPointY(i));
			pos.y = gameMap->getScaledHeightAt(pos.x, pos.z);

			ui::IVisualObjectData *visData;
			game::CoverMap *cov = gameMap->getCoverMap();

			otherPos = pos + deformVec;
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightMid = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			lightMid -= 0.03f; // don't deform on too small changes...

			otherPos = pos + deformVec + VC3(-1.5f, 0, 0);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightLeft = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2)
				lightLeft = 1.0f;

			otherPos = pos + deformVec + VC3(1.5f, 0, 0);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightRight = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2)
				lightRight = 1.0f;

			otherPos = pos + deformVec + VC3(-3.5f, 0, 0);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightLeft2 = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2
				|| lightLeft == 1.0f)
				lightLeft2 = 1.0f;

			otherPos = pos + deformVec + VC3(3.5f, 0, 0);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightRight2 = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2
				|| lightRight == 1.0f)
				lightRight2 = 1.0f;

			otherPos = pos + deformVec + VC3(0, 0, -1.5f);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightUp = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2)
				lightUp = 1.0f;

			otherPos = pos + deformVec + VC3(0, 0, 1.5f);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightDown = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2)
				lightDown = 1.0f;

			otherPos = pos + deformVec + VC3(0, 0, -3.5f);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightUp2 = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2
				|| lightUp == 1.0f)
				lightUp2 = 1.0f;

			otherPos = pos + deformVec + VC3(0, 0, 3.5f);
			gameMap->keepWellInScaledBoundaries(&otherPos.x, &otherPos.z);
			float lightDown2 = LightAmountManager::getInstance()->getDynamicLightAmount(otherPos, visData, height);
			if (cov->getDistanceToNearestCover(gameMap->scaledToObstacleX(otherPos.x), gameMap->scaledToObstacleY(otherPos.z)) < 2
				|| lightDown == 1.0f)
				lightDown2 = 1.0f;

			if (i - pathIndex < (pathSize - pathIndex) / 2)
			{
				//char buf2[256];
				//sprintf(buf2, "%f", lightMid);
				//Logger::getInstance()->error(buf2);
				VC3 newDeformVec = deformVec;

				if (lightLeft < lightMid)
				{
					newDeformVec = deformVec + VC3(-1, 0, 0);
					lightMid = lightLeft;
					deformed = true;
				}
				if (lightRight < lightMid)
				{
					newDeformVec = deformVec + VC3(1, 0, 0);
					lightMid = lightRight;
					deformed = true;
				}
				if (lightUp < lightMid)
				{
					newDeformVec = deformVec + VC3(0, 0, -1);
					lightMid = lightUp;
					deformed = true;
				}
				if (lightDown < lightMid)
				{
					newDeformVec = deformVec + VC3(0, 0, 1);
					lightMid = lightDown;
					deformed = true;
				}
				if (lightLeft2 < lightMid - 0.1f)
				{
					newDeformVec = deformVec + VC3(-1, 0, 0);
					lightMid = lightLeft;
					deformed = true;
				}
				if (lightRight2 < lightMid - 0.1f)
				{
					newDeformVec = deformVec + VC3(1, 0, 0);
					lightMid = lightRight;
					deformed = true;
				}
				if (lightUp2 < lightMid - 0.1f)
				{
					newDeformVec = deformVec + VC3(0, 0, -1);
					lightMid = lightUp;
					deformed = true;
				}
				if (lightDown2 < lightMid - 0.1f)
				{
					newDeformVec = deformVec + VC3(0, 0, 1);
					lightMid = lightDown;
					deformed = true;
				}

				if (deformVec.x != newDeformVec.x
					|| deformVec.z != newDeformVec.z)
				{
					deformedAmount++;
				}
				deformVec = newDeformVec;
			} else {
				//if (i - pathIndex > (pathSize - pathIndex) - deformedAmount)
				//{
				//	if (deformVec.x > 0)
				//		deformVec.x -= 1.0;
				//	else if (deformVec.x < 0)
				//		deformVec.x += 1.0;
				//	if (deformVec.z > 0)
				//		deformVec.z -= 1.0;
				//	else if (deformVec.z < 0)
				//		deformVec.z += 1.0;
				//}
			}

			otherPos = pos + deformVec;
			int newpx = gameMap->scaledToPathfindX(otherPos.x);
			int newpy = gameMap->scaledToPathfindY(otherPos.z);
			//if (newpx != path->getPointX(i))
			//{
			//	Logger::getInstance()->error("WHEEE!!!");
			//}
			//char buf[256];
			//sprintf(buf, "%d,%d -> %d, %d", path->getPointX(i), path->getPointY(i), newpx, newpy);
			//Logger::getInstance()->error(buf);

			path->setPoint(i, newpx, newpy);

		}

		return deformed;
	}
}

