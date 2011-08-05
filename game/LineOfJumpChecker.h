
#ifndef LINEOFJUMPCHECKER_H
#define LINEOFJUMPCHECKER_H

#include <DatatypeDef.h>

namespace game
{
	class Unit;
	class Game;

	class LineOfJumpChecker
	{
		public:
			static bool hasLineOfJump(Unit *unit, const VC3 &position, Game *game,
				const VC3 &targetPosition, float targetAreaRange);
	};
}

#endif

