
#ifndef RECONCHECKER_H
#define RECONCHECKER_H

#include <DatatypeDef.h>

namespace game
{
	class Game;

	class ReconChecker
	{
		public:
			static bool isReconAvailableAtPosition(Game *game, int player, const VC3 &position);
	};

}

#endif



