
#include "precompiled.h"

#include "CoverFinder.h"

#include <stdlib.h>
#include "CoverMap.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{
	bool CoverFinder::findCover(CoverMap *covermap, int x, int y,
		int *destx, int *desty, int fromx, int fromy)
	{
		/*
		CoverMap::COVER_DIRECTION dir 
			= covermap->getNearestCoverDirection(upathPosX, upathPosY);

		if (dir == CoverMap::COVER_DIRECTION_N)
		{
			gsd->position.z += distScaled;
		}
		if (dir == CoverMap::COVER_DIRECTION_S)
		{
			gsd->position.z -= distScaled;
		}
		if (dir == CoverMap::COVER_DIRECTION_W)
		{
			gsd->position.x -= distScaled;
		}
		if (dir == CoverMap::COVER_DIRECTION_E)
		{
			gsd->position.x += distScaled;
		}
		*/

		int tx = x;
		int ty = y;
		int dx = x;
		int dy = y;

		int sizex = covermap->getSizeX();
		int sizey = covermap->getSizeY();
		int d = covermap->getDistanceToNearestCover(tx, ty);

		// track to nearest obstacle...
		while (true)
		{
			int prevd = d;
			if (ty > 0 && tx > 0 && ty < sizey - 1 && tx < sizex - 1)
			{
				if (covermap->getDistanceToNearestCover(tx, ty - 1) < d)
					ty--;
				else if (covermap->getDistanceToNearestCover(tx, ty + 1) < d)
					ty++;
				else if (covermap->getDistanceToNearestCover(tx - 1, ty) < d)
					tx--;
				else if (covermap->getDistanceToNearestCover(tx + 1, ty) < d)
					tx++;
				else if (covermap->getDistanceToNearestCover(tx + 1, ty + 1) < d)
					{ tx++; ty++; }
				else if (covermap->getDistanceToNearestCover(tx + 1, ty - 1) < d)
					{ tx++; ty--; }
				else if (covermap->getDistanceToNearestCover(tx - 1, ty - 1) < d)
					{ tx--; ty--; }
				else if (covermap->getDistanceToNearestCover(tx - 1, ty + 1) < d)
					{ tx--; ty++; }
			}
			d = covermap->getDistanceToNearestCover(tx, ty);
			if (prevd == d)
				break;
			if (d == 0) 
				break;
		}

		// direction mask to get cover from...
		int primaryCoverMask = 0;
		int secondaryCoverMask = 0;

		if (fromx < tx)
		{
			if (abs(fromy - ty) <= abs(fromx - tx))
				primaryCoverMask = CoverMap::COVER_FROM_WEST_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_WEST_MASK;
		}
		else if (fromx > tx)
		{
			if (abs(fromy - ty) <= abs(fromx - tx))
				primaryCoverMask = CoverMap::COVER_FROM_EAST_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_EAST_MASK;
		}
		if (fromy < ty)
		{
			if (abs(fromx - tx) <= abs(fromy - ty))
				primaryCoverMask = CoverMap::COVER_FROM_NORTH_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_NORTH_MASK;
		}
		else if (fromy > ty)
		{
			if (abs(fromx - tx) <= abs(fromy - ty))
				primaryCoverMask = CoverMap::COVER_FROM_SOUTH_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_SOUTH_MASK;
		}

		// found our obstacle, now find out best cover side near it
		if (d == 0)
		{
      int x1 = tx - 3; if (x1 < 0) x1 = 0;
      int x2 = tx + 3; if (x2 > sizex - 1) x2 = sizex - 1;
      int y1 = ty - 3; if (y1 < 0) y1 = 0;
      int y2 = ty + 3; if (y2 > sizey - 1) y2 = sizey - 1;
			// TODO
			// damn, would need random here... =/
			// but don't want to use rand()...
			// should use GameRandom...
			// could swap the check pass directions based on that
			// so we would not always prefer the upper left corner.
			for (int i = y1; i < y2; i++)
			{
				for (int j = x1; j < x2; j++)
				{
					if (i != tx || j != ty)
					{
						if (covermap->isCoveredFromAll(primaryCoverMask, j, i)
							&& covermap->getDistanceToNearestCover(j, i) == 1)
						{
							// this we like very much, cover from primary direction
							// take it!
						  dx = j;
							dy = i;
							i = y2;
							break;
						}
						if (covermap->isCoveredFromAll(secondaryCoverMask, j, i))
						{
							// well, if we won't find a primary direction cover, 
							// this will do...
							dx = j;
							dy = i;
						}
					}
				}
			}
		}

		*destx = dx;
		*desty = dy;

		return true;
	}


	bool CoverFinder::isCoveredFrom(CoverMap *covermap, int x, int y,
		int fromx, int fromy)
	{
		// direction mask to get cover from...
		int primaryCoverMask = 0;
		int secondaryCoverMask = 0;

		// must be next to obstacle to be covered...
		if (covermap->getDistanceToNearestCover(x, y) != 1)
			return false;

		if (fromx < x)
		{
			if (abs(fromy - y) <= abs(fromx - x))
				primaryCoverMask = CoverMap::COVER_FROM_WEST_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_WEST_MASK;
		}
		else if (fromx > x)
		{
			if (abs(fromy - y) <= abs(fromx - x))
				primaryCoverMask = CoverMap::COVER_FROM_EAST_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_EAST_MASK;
		}
		if (fromy < y)
		{
			if (abs(fromx - x) <= abs(fromy - y))
				primaryCoverMask = CoverMap::COVER_FROM_NORTH_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_NORTH_MASK;
		}
		else if (fromy > y)
		{
			if (abs(fromx - x) <= abs(fromy - y))
				primaryCoverMask = CoverMap::COVER_FROM_SOUTH_MASK;
			else
			  secondaryCoverMask = CoverMap::COVER_FROM_SOUTH_MASK;
		}

		bool covered = covermap->isCoveredFromAny(
			(primaryCoverMask | secondaryCoverMask), x, y);

		// blocked won't do for cover...
		// already checked that dist == 1
		//if (covermap->getDistanceToNearestCover(x, y) == 0)
		//	covered = false;

		return covered;
	}

}

