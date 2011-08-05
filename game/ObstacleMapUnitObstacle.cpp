
#include "precompiled.h"

#include "ObstacleMapUnitObstacle.h"
#include "Game.h"
#include "GameScene.h"
#include "scaledefs.h"
#include "GameMap.h"
#include "Unit.h"
#include "../system/Logger.h"



namespace game
{
  inline bool obstmap_is_at_line(int x, int y, float angle, bool leftside, bool rightside, int lineWidth)
	{
		if (x == 0 && y == 0) return true;

		float line_dx = -sinf(angle - 3.141592f/2);
		float line_dy = -cosf(angle - 3.141592f/2);

		float line_dx_zero, line_dy_zero;
		if (fabs(line_dx) < 0.1f)
			line_dx_zero = 0;
		else 
			line_dx_zero = line_dx;
		if (fabs(line_dy) < 0.1f)
			line_dy_zero = 0;
		else 
			line_dy_zero = line_dy;
		if ((leftside && (line_dx_zero * x) >= 0 && (line_dy_zero * y) >= 0) ||
			(rightside && (line_dx_zero * x) <= 0 && (line_dy_zero * y) <= 0))
		{
			// nop
		} else {
			return false;
		}

		if (fabs(line_dx) < 0.1f)
		{
			// tripled line thickness...
			//if (x == 0) return true; else return false;
			if (lineWidth <= 1)
			{
				if (x == 0 || x == -1 || x == 1) return true; else return false;
			} else {
				if (x >= -lineWidth && x <= lineWidth) return true; else return false;
			}
		}
		if (fabs(line_dy) < 0.1f)
		{
			// tripled line thickness...
			//if (y == 0) return true; else return false;
			if (lineWidth <= 1)
			{
				if (y == 0 || y == -1 || y == 1) return true; else return false;
			} else {
				if (y >= -lineWidth && y <= lineWidth) return true; else return false;
			}
		}

		// about triple thickness... (i think?)
		//float line_y_at_x = (line_dy / line_dx) * (float)x;
		float line_y_at_x_plus1 = (line_dy / line_dx) * (float)(x + lineWidth);
		float line_y_at_x_minus1 = (line_dy / line_dx) * (float)(x - lineWidth);

		//float fy = (float)y;
		float fy_plus = (float)y + lineWidth;
		float fy_minus = (float)y - lineWidth;
		//if ((line_y_at_x <= fy && line_y_at_x_minus1 >= fy)
		//	|| (line_y_at_x >= fy && line_y_at_x_minus1 <= fy))
		if ((line_y_at_x_plus1 <= fy_plus && line_y_at_x_minus1 >= fy_minus)
			|| (line_y_at_x_plus1 >= fy_minus && line_y_at_x_minus1 <= fy_plus))
		{
			return true;
		} else {
			return false;
		}
	}


	void ObstacleMapUnitObstacle::addObstacle(Game *game, Unit *unit, 
		int radius, int height, bool lineblock, bool leftside, bool rightside, int lineWidth)
	{
		if (unit->isGhostOfFuture())
			return;

		assert(!unit->obstacleExists);

		VC3 pos = unit->getPosition();
		int obstacleX = game->gameMap->scaledToObstacleX(pos.x);
		int obstacleY = game->gameMap->scaledToObstacleY(pos.z);

		unit->obstacleExists = true;
		unit->obstacleX = obstacleX;
		unit->obstacleY = obstacleY;
		unit->obstacleAngle = unit->yAngle;

		// TODO:
		// support for other sizes still kinda preliminary (see below)
		if (radius == 1)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			if (obstacleX < 0 || obstacleY < 0
				|| obstacleX >= mapsizex || obstacleY >= mapsizey)
				return;
			game->getGameScene()->addMovingObstacle(obstacleX, obstacleY, height);
		}
		else if (radius == 2)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			if (obstacleX <= 0 || obstacleY <= 0
				|| obstacleX >= mapsizex - 1 || obstacleY >= mapsizey - 1)
				return;
			game->getGameScene()->addMovingObstacle(obstacleX, obstacleY, height);
			game->getGameScene()->addMovingObstacle(obstacleX, obstacleY - 1, height);
			game->getGameScene()->addMovingObstacle(obstacleX - 1, obstacleY, height);
			game->getGameScene()->addMovingObstacle(obstacleX, obstacleY + 1, height);
			game->getGameScene()->addMovingObstacle(obstacleX + 1, obstacleY, height);
			// corners too...
			game->getGameScene()->addMovingObstacle(obstacleX - 1, obstacleY - 1, height);
			game->getGameScene()->addMovingObstacle(obstacleX - 1, obstacleY + 1, height);
			game->getGameScene()->addMovingObstacle(obstacleX + 1, obstacleY + 1, height);
			game->getGameScene()->addMovingObstacle(obstacleX + 1, obstacleY - 1, height);
		}
		else if (radius > 2)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			int minX = obstacleX - radius;
			int maxX = obstacleX + radius;
			int minY = obstacleY - radius;
			int maxY = obstacleY + radius;
			if (minX < 0) minX = 0;
			if (minY < 0) minY = 0;
			if (maxX >= mapsizex) maxX = mapsizex - 1;
			if (maxY >= mapsizey) maxY = mapsizey - 1;
			int radiusSq = radius * radius;

			// TEMP!
			//char buf[16 * 16];
			//for (int c = 0; c < 16*16; c++) { buf[c] = ' '; }
			 
			for (int i = minY; i <= maxY; i++)
			{
				for (int j = minX; j <= maxX; j++)
				{
					int diffX = j - obstacleX;
					int diffY = i - obstacleY;
					if (diffX * diffX + diffY * diffY <= radiusSq)
					{
						if (lineblock)
						{
							if (obstmap_is_at_line(diffX, diffY, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth))
// (this hack removed)
//								|| obstmap_is_at_line(diffX + 1, diffY, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth)
//								|| obstmap_is_at_line(diffX, diffY + 1, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth))
							{
								game->getGameScene()->addDoorObstacle(j, i, height);
								//if (diffX >= -7 && diffX <= 7)
									//if (diffY >= -7 && diffY <= 7)
										//buf[diffX + 8 + (diffY + 8) * 16] = 'X';
							//} else {
								//if (diffX >= -7 && diffX <= 7)
									//if (diffY >= -7 && diffY <= 7)
										//buf[diffX + 8 + (diffY + 8) * 16] = '.';
							}
						} else {
							// TODO: non-line obstacles should check overlapping too?
							game->getGameScene()->addMovingObstacle(j, i, height);
						}
					}
				}
			}

			//for (int d = 0; d < 16; d++)
			//{
				//buf[15 + d * 16] = '\n';
			//}
			//buf[15 + 15 * 16] = '\0';
			//Logger::getInstance()->error("ADD");
			//Logger::getInstance()->error(buf);

		}
	}	



	void ObstacleMapUnitObstacle::removeObstacle(Game *game, Unit *unit, 
		int radius, int height, bool lineblock, bool leftside, bool rightside, int lineWidth)
	{
		if (unit->isGhostOfFuture())
			return;

//		assert(unit->obstacleExists);

		unit->obstacleExists = false;

		int obstacleX = unit->obstacleX;
		int obstacleY = unit->obstacleY;

		// TODO:
		// support for other sizes still kinda preliminary (see below)
		if (radius == 1)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			if (obstacleX < 0 || obstacleY < 0
				|| obstacleX >= mapsizex || obstacleY >= mapsizey)
				return;
			game->getGameScene()->removeMovingObstacle(obstacleX, obstacleY, height);
		}
		else if (radius == 2)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			if (obstacleX <= 0 || obstacleY <= 0
				|| obstacleX >= mapsizex - 1 || obstacleY >= mapsizey - 1)
				return;
			game->getGameScene()->removeMovingObstacle(obstacleX, obstacleY, height);
			game->getGameScene()->removeMovingObstacle(obstacleX, obstacleY - 1, height);
			game->getGameScene()->removeMovingObstacle(obstacleX - 1, obstacleY, height);
			game->getGameScene()->removeMovingObstacle(obstacleX, obstacleY + 1, height);
			game->getGameScene()->removeMovingObstacle(obstacleX + 1, obstacleY, height);
			// corners too...
			game->getGameScene()->removeMovingObstacle(obstacleX - 1, obstacleY - 1, height);
			game->getGameScene()->removeMovingObstacle(obstacleX - 1, obstacleY + 1, height);
			game->getGameScene()->removeMovingObstacle(obstacleX + 1, obstacleY + 1, height);
			game->getGameScene()->removeMovingObstacle(obstacleX + 1, obstacleY - 1, height);
		}
		else if (radius > 2)
		{
			int mapsizex = game->gameMap->getObstacleSizeX();
			int mapsizey = game->gameMap->getObstacleSizeY();
			int minX = obstacleX - radius;
			int maxX = obstacleX + radius;
			int minY = obstacleY - radius;
			int maxY = obstacleY + radius;
			if (minX < 0) minX = 0;
			if (minY < 0) minY = 0;
			if (maxX >= mapsizex) maxX = mapsizex - 1;
			if (maxY >= mapsizey) maxY = mapsizey - 1;
			int radiusSq = radius * radius;

			// TEMP!
			//char buf[16 * 16];
			//for (int c = 0; c < 16*16; c++) { buf[c] = ' '; }

			for (int i = minY; i <= maxY; i++)
			{
				for (int j = minX; j <= maxX; j++)
				{
					int diffX = j - obstacleX;
					int diffY = i - obstacleY;
					if (diffX * diffX + diffY * diffY <= radiusSq)
					{
						if (lineblock)
						{
							if (obstmap_is_at_line(diffX, diffY, UNIT_ANGLE_TO_RAD(unit->obstacleAngle), leftside, rightside, lineWidth))
// (this hack removed)
//								|| obstmap_is_at_line(diffX + 1, diffY, UNIT_ANGLE_TO_RAD(unit->obstacleAngle), leftside, rightside, lineWidth)
//								|| obstmap_is_at_line(diffX, diffY + 1, UNIT_ANGLE_TO_RAD(unit->obstacleAngle), leftside, rightside, lineWidth))
							{
								game->getGameScene()->removeDoorObstacle(j, i, height);
								//if (diffX >= -7 && diffX <= 7)
									//if (diffY >= -7 && diffY <= 7)
										//buf[diffX + 8 + (diffY + 8) * 16] = 'X';
							//} else {
								//if (diffX >= -7 && diffX <= 7)
									//if (diffY >= -7 && diffY <= 7)
										//buf[diffX + 8 + (diffY + 8) * 16] = '.';
							}
						} else {
							game->getGameScene()->removeMovingObstacle(j, i, height);
						}
					}
				}
			}

			//for (int d = 0; d < 16; d++)
			//{
				//buf[15 + d * 16] = '\n';
			//}
			//buf[15 + 15 * 16] = '\0';
			//Logger::getInstance()->error("REMOVE");
			//Logger::getInstance()->error(buf);
		}
	}



	void ObstacleMapUnitObstacle::moveObstacle(Game *game, Unit *unit,
		int x, int y, int radius, int height, bool lineblock, bool leftside, bool rightside, int lineWidth)
	{
		if (unit->isGhostOfFuture())
			return;

//		assert(unit->obstacleExists);

		if (unit->obstacleX != x || unit->obstacleY != y)
		{
			removeObstacle(game, unit, radius, height, lineblock, leftside, rightside, lineWidth);

			unit->obstacleExists = true;
			unit->obstacleX = x;
			unit->obstacleY = y;
			unit->obstacleAngle = unit->yAngle;

			// TODO:
			// support for other sizes still kinda preliminary (see below)
			if (radius == 1)
			{
				int mapsizex = game->gameMap->getObstacleSizeX();
				int mapsizey = game->gameMap->getObstacleSizeY();
				if (x < 0 || y < 0
					|| x >= mapsizex || y >= mapsizey)
					return;
				game->getGameScene()->addMovingObstacle(x, y, height);
			}
			else if (radius == 2)
			{
				int mapsizex = game->gameMap->getObstacleSizeX();
				int mapsizey = game->gameMap->getObstacleSizeY();
				if (x <= 0 || y <= 0
					|| x >= mapsizex - 1 || y >= mapsizey - 1)
					return;
				game->getGameScene()->addMovingObstacle(x, y, height);
				game->getGameScene()->addMovingObstacle(x, y - 1, height);
				game->getGameScene()->addMovingObstacle(x - 1, y, height);
				game->getGameScene()->addMovingObstacle(x, y + 1, height);
				game->getGameScene()->addMovingObstacle(x + 1, y, height);
				// corners too...
				game->getGameScene()->addMovingObstacle(x - 1, y - 1, height);
				game->getGameScene()->addMovingObstacle(x - 1, y + 1, height);
				game->getGameScene()->addMovingObstacle(x + 1, y + 1, height);
				game->getGameScene()->addMovingObstacle(x + 1, y - 1, height);
			}
			else if (radius > 2)
			{
				int mapsizex = game->gameMap->getObstacleSizeX();
				int mapsizey = game->gameMap->getObstacleSizeY();
				int minX = x - radius;
				int maxX = x + radius;
				int minY = y - radius;
				int maxY = y + radius;
				if (minX < 0) minX = 0;
				if (minY < 0) minY = 0;
				if (maxX >= mapsizex) maxX = mapsizex - 1;
				if (maxY >= mapsizey) maxY = mapsizey - 1;
				int radiusSq = radius * radius;

				//unit->obstacleOverlaps = false;
				int overLapAmount = 0;

				// TEMP!
				//char buf[16 * 16];
				//for (int c = 0; c < 16*16; c++) { buf[c] = ' '; }

				for (int i = minY; i <= maxY; i++)
				{
					for (int j = minX; j <= maxX; j++)
					{
						int diffX = j - x;
						int diffY = i - y;
						if (diffX * diffX + diffY * diffY <= radiusSq)
						{
							if (lineblock)
							{
								if (obstmap_is_at_line(diffX, diffY, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth))
//									|| obstmap_is_at_line(diffX + 1, diffY, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth)
//									|| obstmap_is_at_line(diffX, diffY + 1, UNIT_ANGLE_TO_RAD(unit->yAngle), leftside, rightside, lineWidth))
								{
									// does this (door) overlap some other moving unit?
									if (game->gameMap->isMovingObstacle(j, i)
										&& game->gameMap->getObstacleHeight(j, i) > 0)
									{
										//unit->obstacleOverlaps = true;
										overLapAmount++;
									}
									game->getGameScene()->addDoorObstacle(j, i, height);
									//if (diffX >= -7 && diffX <= 7)
										//if (diffY >= -7 && diffY <= 7)
											//buf[diffX + 8 + (diffY + 8) * 16] = 'X';
								//} else {
									//if (diffX >= -7 && diffX <= 7)
										//if (diffY >= -7 && diffY <= 7)
											//buf[diffX + 8 + (diffY + 8) * 16] = '.';
								}
							} else {
								game->getGameScene()->addMovingObstacle(j, i, height);
							}
						}
					}
				}

				//  if (overLapAmount >= 1)
				// HACK: at least 4 blocks need to overlap!
				// HACK: more hack hack...
				// in case of 90 deg doors only!!				
				// FIXME: really needs some proper code!
				if (overLapAmount >= 4
					|| ((int(UNIT_ANGLE_TO_RAD(unit->obstacleAngle) + 0.5f) % 90) != 0
					&& overLapAmount >= 2))
				{
					unit->obstacleOverlaps = true;
				} else {
					unit->obstacleOverlaps = false;
				}


				//for (int d = 0; d < 16; d++)
				//{
					//buf[15 + d * 16] = '\n';
				//}
				//buf[15 + 15 * 16] = '\0';
				//Logger::getInstance()->error("MOVEADD");
				//Logger::getInstance()->error(buf);
			}
		}
	}

}

