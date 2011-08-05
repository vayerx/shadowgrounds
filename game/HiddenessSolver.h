
#ifndef HIDDENESSSOLVER_H
#define HIDDENESSSOLVER_H

#include <DatatypeDef.h>

namespace game
{
	class GameMap;

	class HiddenessSolver
	{
		public:
			/**
			 * Solve the hiddeness of endpoint when looking at it from
			 * the startpoint.
			 * @return a factor for hiddeness, between 0.0f - 1.0f.
			 */
			static float solveHiddenessFactorBetween(GameMap *gameMap, 
				const VC3 &startPos, const VC3 &endPos);
	};
}

#endif


