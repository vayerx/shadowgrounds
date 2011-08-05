
#include "HiddenessSolver.h"

#include "GameMap.h"
#include "HideMap.h"

namespace game
{

	float HiddenessSolver::solveHiddenessFactorBetween(
		GameMap *gameMap, const VC3 &startPos, const VC3 &endPos)
	{
		// TODO: proper implementation!
		// this one just checks a few positions between the points.
		// should check much more...

		int obstSizeX = gameMap->getObstacleSizeX();
		int obstSizeY = gameMap->getObstacleSizeY();
		int endX = gameMap->scaledToObstacleX(endPos.x);
		int endY = gameMap->scaledToObstacleY(endPos.z);
		int startX = gameMap->scaledToObstacleX(startPos.x);
		int startY = gameMap->scaledToObstacleY(startPos.z);
		int midX = (startX + endX) / 2;
		int midY = (startY + endY) / 2;
		int veryVeryNearEndX = (midX + endX * 3) / 4; // :)
		int veryVeryNearEndY = (midY + endY * 3) / 4;

		// heighest value... endpoint totally significant
		// others are not taken fully into account... their values are
		// halved. 
		int max = 0;
//		int tmp;

		// TODO: check height for midpoint and others!!!

		if (endX <= 0 || endY <= 0 || endX >= obstSizeX - 1 || endY >= obstSizeY - 1)
			return 0.0f;
		if (startX <= 0 || startY <= 0 || startX >= obstSizeX - 1 || startY >= obstSizeY - 1)
			return 0.0f;

		tmp = gameMap->getHideMap()->getHiddenessAt(endX, endY);
		if (tmp > max) max = tmp;

		tmp = gameMap->getHideMap()->getHiddenessAt(veryVeryNearEndX, veryVeryNearEndY);
		if (tmp > max) max = tmp;

		if (max < (HideMap::maxHiddeness >> 1))
		{
			int nearEndX = (midX + endX) / 2;
			int nearEndY = (midY + endY) / 2;
			int veryNearEndX = (midX + endX * 2) / 3;
			int veryNearEndY = (midY + endY * 2) / 3;
			int nearStartX = (startX + midX) / 2;
			int nearStartY = (startY + midY) / 2;

			tmp = gameMap->getHideMap()->getHiddenessAt(veryNearEndX, veryNearEndY) / 2;
			if (tmp > max) max = tmp;

			tmp = gameMap->getHideMap()->getHiddenessAt(nearEndX, nearEndY) / 2;
			if (tmp > max) max = tmp;

			tmp = gameMap->getHideMap()->getHiddenessAt(midX, midY) / 3;
			if (tmp > max) max = tmp;

			tmp = gameMap->getHideMap()->getHiddenessAt(nearStartX, nearStartY) / 4;
			if (tmp > max) max = tmp;
		}

		return ((float)max / (float)HideMap::maxHiddeness);		
	}

}
