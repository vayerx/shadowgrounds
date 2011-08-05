
#include "precompiled.h"

#include "LineOfJumpChecker.h"

#include "Unit.h"
#include "Game.h"
#include "GameMap.h"
#include "GameScene.h"
#include "../util/LineAreaChecker.h"
#include "../system/Logger.h"



#ifdef LINEOFJUMP_DEBUG
unsigned char *lineofjump_debug_data = NULL;
#endif

namespace game
{

	bool LineOfJumpChecker::hasLineOfJump(Unit *unit, const VC3 &position, Game *game,
		const VC3 &targetPosition, float targetAreaRange)
	{
		// FIXME: proper checking, this one just checks 10 positions
		// between unit and given point - should make the checks based
		// on distance... (more distance, more checks)

		VC3 unitPos = unit->getPosition();
		VC3 endPos = position;
		game->gameMap->keepWellInScaledBoundaries(&unitPos.x, &unitPos.z);
		game->gameMap->keepWellInScaledBoundaries(&endPos.x, &endPos.z);

#ifdef LINEOFJUMP_DEBUG
	/*
	if (lineofjump_debug_data != NULL)
	{
		delete [] lineofjump_debug_data;
		lineofjump_debug_data = NULL;
	}
	*/
	if (lineofjump_debug_data == NULL)
	{
		int xSize = game->gameMap->getObstacleSizeX();
		int ySize = game->gameMap->getObstacleSizeY();
		lineofjump_debug_data = new unsigned char[xSize * ySize];
		for (int i = 0; i < xSize * ySize; i++)
		{
			lineofjump_debug_data[i] = 0;
		}
	}

	if (lineofjump_debug_data != NULL)
	{
		int xSize = game->gameMap->getObstacleSizeX();
		int ySize = game->gameMap->getObstacleSizeY();
		for (int oy = 0; oy < ySize; oy++)
		{
			for (int ox = 0; ox < xSize; ox++)
			{
				VC3 foopos = VC3(game->gameMap->obstacleToScaledX(ox), 0,
					game->gameMap->obstacleToScaledY(oy));
				foopos.y = targetPosition.y;

				if (util::LineAreaChecker::isPointInsideLineArea(foopos,
					unitPos, endPos, targetAreaRange))
				{
					lineofjump_debug_data[ox + oy * xSize] = 1;
				} else {
					lineofjump_debug_data[ox + oy * xSize] = 0;
				}
			}
		}
	}
#endif

		if (targetAreaRange > 0)
		{
			if (!util::LineAreaChecker::isPointInsideLineArea(targetPosition,
				unitPos, endPos, targetAreaRange))
			{
//Logger::getInstance()->error("POINT OUTSIDE LINE AREA");
				return false;
			}
		}

		// don't check the actual unit position (0)
		for (int i = 1; i < 10; i++)
		{
			float fact = (float)i / 10.0f;
			VC3 pos = (unitPos * fact) + (endPos * (1-fact));
			int ox = game->gameMap->scaledToObstacleX(pos.x);
			int oy = game->gameMap->scaledToObstacleY(pos.z);
			if (game->getGameScene()->isBlockedAtScaled(pos.x, pos.z, 
				game->gameMap->getScaledHeightAt(pos.x, pos.z) + 0.01f)
				&& !game->gameMap->isMovingObstacle(ox, oy))
			{
//Logger::getInstance()->error("LINE BLOCKED");
				return false;
			}
		}
		return true;
	}

}



