#include <vector>

#include "precompiled.h"

#include "PathSimplifier.h"

#include <assert.h>

#include "Debug_MemoryManager.h"


namespace frozenbyte {
namespace ai {

frozenbyte::ai::Path *PathSimplifier::getSimplifiedPath(
	frozenbyte::ai::PathFind *pathfinder,
	const frozenbyte::ai::Path *path, int simplificationLevel,
	int maxHeightDifference)
{
	if (path == NULL)
	{
		assert(!"PathSimplifier - Attempt to simplify null path.");
		return NULL;
	}

	if (path->getSize() == 0) 
	{
		// TODO: ???
		//Logger::getInstance()->debug("PathSimplifier::getSimplifiedPath - Encountered a path with no points.");
		return NULL;
	}

	assert(pathfinder != NULL);

	// anything less would be useless.
	assert(simplificationLevel >= 2);

	// construct a new path (notice, direction will be inverted)
	frozenbyte::ai::Path *inv = new frozenbyte::ai::Path();

	int psize = path->getSize();
	inv->addPoint(path->getPointX(0), path->getPointY(0));
	int i;
	for (i = 1; i < psize - 1; i++)
	{
		int sx = path->getPointX(i - 1);
		int sy = path->getPointY(i - 1);
		int dx = path->getPointX(i);
		int dy = path->getPointY(i);
		int dx2 = 0;
		int dy2 = 0;
		bool canSimplify = false;
		int simpAmount = 0;

		// if not yet at the very end of the path, try to simplify
		if (i < psize - 2)
		{
			simpAmount = simplificationLevel;
			if (i + simpAmount > psize - 1)
				simpAmount = (psize - 1) - i;

			dx2 = path->getPointX(i + simpAmount);
			dy2 = path->getPointY(i + simpAmount);
			canSimplify = true;

			// create a direct line between the simplification points
			// and check the line for obstacles..
			int dist = abs(dx2 - sx);
			if (abs(dy2 - sy) > dist) dist = abs(dy2 - sy);
			if (dist < 1) dist = 1;

			// just some values next to the first ones.
			int lastx = sx - 1;
			int lasty = sy - 1;

			// double line resolution to be sure
			for (int j = 0; j <= dist * 2; j++)
			{
				// line point to check for obstacles
				int tx = ((sx * (dist * 2 - j)) + (dx2 * j)) / (dist * 2);
				int ty = ((sy * (dist * 2 - j)) + (dy2 * j)) / (dist * 2);

				// make sure the line is properly done (one block moves)
				if (abs(tx - lastx) > 1 || abs(ty - lasty) > 1)
				{
					assert(0);
				}

				// don't try to simplify near edges 
				// (because the below checks presume that there is some border area)
				if (tx <= 0 || ty <= 0 
					|| tx >= pathfinder->xSize - 1 || ty >= pathfinder->ySize - 1)
				{
					canSimplify = false;
					break;
				}

				// TODO: a better check...
				// (now this check will not allow to simplify when moving
				// in narrow corridors, even when it actually could be simplified)

				if (tx != lastx || ty != lasty)
				{
					if (!pathfinder->isMovable(tx, ty, tx + 1, ty, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx + 1, ty + 1, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx + 1, ty - 1, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx - 1, ty, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx - 1, ty + 1, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx - 1, ty - 1, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx, ty + 1, maxHeightDifference)
						|| !pathfinder->isMovable(tx, ty, tx, ty - 1, maxHeightDifference))
					{
						// some part of the direct line was blocked, cannot simplify
						canSimplify = false;
						break;
					}
				}

				lastx = tx;
				lasty = ty;
			}
		}

		if (canSimplify)
		{			
			inv->addPoint(dx2, dy2);
			i += simpAmount;
		} else {
			inv->addPoint(dx, dy);
		}
	}

	// need to invert the path's direction for the final result,
	// because the constructed path above is inverted.
	frozenbyte::ai::Path *ret = new frozenbyte::ai::Path();

	int invsize = inv->getSize();
	for (i = 0; i < invsize; i++)
	{
		ret->addPoint(inv->getPointX(i), inv->getPointY(i));
	}

	delete inv;

	return ret;
}

} // end of namespace ai
} // end of namespace frozenbyte


