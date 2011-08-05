
#ifndef CHECKPOINTCHECKER_H
#define CHECKPOINTCHECKER_H

#include "Checkpoints.h"

namespace game
{
  class Game;

	class CheckpointChecker
	{
		public:
			static bool isEveryUnitNearCheckpoint(Game *game, float range,
				int player);

			static bool isAnyUnitNearCheckpoint(Game *game, float range,
				int player);

			static bool isPositionNearCheckpoint(Game *game, float range,
				VC3 &position);

	};
}

#endif



