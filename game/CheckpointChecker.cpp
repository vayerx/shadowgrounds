
#include "precompiled.h"

#include "CheckpointChecker.h"
#include "Game.h"
#include "Unit.h"
#include "UnitList.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{
	bool CheckpointChecker::isEveryUnitNearCheckpoint(Game *game, float range,
		int player)
	{
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);

		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{
				if (!game->checkpoints->isNearCheckpoint(0, u->getPosition(), range))
					return false;
			}
		}
		return true;
	}

	
	bool CheckpointChecker::isAnyUnitNearCheckpoint(Game *game, float range,
		int player)
	{
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);

		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{
				if (game->checkpoints->isNearCheckpoint(0, u->getPosition(), range))
					return true;
			}
		}
		return false;
	}


	bool CheckpointChecker::isPositionNearCheckpoint(Game *game, float range,
		VC3 &position)
	{
    return game->checkpoints->isNearCheckpoint(0, position, range);
	}
}

