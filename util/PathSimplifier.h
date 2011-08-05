
#ifndef PATHSIMPLIFIER_H
#define PATHSIMPLIFIER_H

#include "AI_PathFind.h"

namespace frozenbyte {
namespace ai {

class PathSimplifier
{
public:
	static frozenbyte::ai::Path *getSimplifiedPath(
		frozenbyte::ai::PathFind *pathfinder,
		const frozenbyte::ai::Path *path, int simplificationLevel,
		int maxHeightDifferenct);

};

} // end of namespace ai
} // end of namespace frozenbyte

#endif

