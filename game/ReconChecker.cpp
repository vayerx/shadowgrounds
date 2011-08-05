
#include "precompiled.h"

#include "ReconChecker.h"

#include "Game.h"
#include "GameScene.h"
#include "UnitList.h"
#include "Unit.h"
#include "UnitType.h"
#include "GameMap.h"
#include "../container/LinkedList.h"

namespace game
{
	bool ReconChecker::isReconAvailableAtPosition(Game *game, int player, const VC3 &position)
	{
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);

		while (iter.iterateAvailable())		
		{
			Unit *u = (Unit *)iter.iterateNext();
			
			// unit is friendly towards player?
			// (but player does not need to be friendly towards the unit ;)
			if (u->isActive() && !u->isDestroyed()				
				&& !game->isHostile(u->getOwner(), player)
				&& u->getReconValue() > 0
				&& u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
			{
				VC3 rayStartPosition = u->getPosition() + VC3(0, 2.0f, 0);
				VC3 target = position;
				float terrainHeight = game->gameMap->getScaledHeightAt(target.x, target.z);
				if (target.y < terrainHeight + 2.0f) target.y = terrainHeight + 2.0f;
				VC3 dir = target - rayStartPosition;
				float dirLen = dir.GetLength();
				dir.Normalize();

				if (dirLen > u->getUnitType()->getVisionRange() + 2.0f)
				{
					continue;
				}

				// TEMP!
				// ignore all small own units (1.5m)...
				// except the one we're trying to hit
				LinkedList *oul = game->units->getOwnedUnits(u->getOwner());
				LinkedListIterator iter = LinkedListIterator(oul);
				while (iter.iterateAvailable())
				{
					Unit *ou = (Unit *)iter.iterateNext();
					if (ou != u && ou->getUnitType()->getSize() <= 1.5f)
						ou->getVisualObject()->setCollidable(false);
				}

				// disable collision check for this unit
				u->getVisualObject()->setCollidable(false);

				GameCollisionInfo cinfo;
				game->getGameScene()->rayTrace(rayStartPosition, dir, (float)dirLen, cinfo, false, true);

				// collision check back
				u->getVisualObject()->setCollidable(true);				

				// TEMP!
				// restore them all...
				oul = game->units->getOwnedUnits(u->getOwner());
				iter = LinkedListIterator(oul);
				while (iter.iterateAvailable())
				{
					Unit *ou = (Unit *)iter.iterateNext();
					if (ou != u && ou->getUnitType()->getSize() <= 1.5f)
						ou->getVisualObject()->setCollidable(true);
				}

				if (cinfo.hit)
				{
					// 4 meters max dist.
					if (cinfo.range >= dirLen - 4.0f)
					{
						return true;
					}
				} else {
					return true;
				}
			}
		}
		return false;
	}

}
