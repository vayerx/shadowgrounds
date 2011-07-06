
#ifndef PATHSIMPLIFIER_H
#define PATHSIMPLIFIER_H

#include "AI_PathFind.h"

class PathSimplifier
{
public:
	static frozenbyte::ai::Path *getSimplifiedPath(
		frozenbyte::ai::PathFind *pathfinder,
		const frozenbyte::ai::Path *path, int simplificationLevel,
		int maxHeightDifferenct);

};

#endif

