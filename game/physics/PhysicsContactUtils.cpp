
#include "precompiled.h"

#include "PhysicsContactUtils.h"
#include "AbstractPhysicsObject.h"
#include "../Game.h"
#include "../UnitList.h"

namespace game
{
	void PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(Game *game, AbstractPhysicsObject *o, Unit **unit, int *terrObjModelId, int *terrObjInstanceId)
	{
		assert(game != NULL);
		assert(o != NULL);
		assert(unit != NULL);
		assert(terrObjModelId != NULL);
		assert(terrObjInstanceId != NULL);

		intptr_t id = (intptr_t)o->getCustomData();
		if (id != 0)
		{
			// bit 32 tells us if this is unit or terr.object
			if ((id & 0x80000000) != 0)
			{
				// terr.object...
				// bits 1-16 tell us the instance id number (0-65536)
				// bits 17-31 tell us the model id number (0-32768)
				int tmp = (id & 0x7fffffff);
				*terrObjModelId = ((tmp & 0x7fff0000) >> 16);
				*terrObjInstanceId = (tmp & 0xffff);
			} else {
				// unit...
				// bits 1-31 tell us the unit id number (1 - ...)
				int unitId = (id & 0x7fffffff);
				*unit = game->units->getUnitById(unitId);
				// assert(unitId != 0)
			}
		}	
	}

	int PhysicsContactUtils::calcCustomPhysicsObjectDataForUnit(Game *game, Unit *unit)
	{
		assert(game != NULL);
		assert(unit != NULL);

		if (unit != NULL)
		{
			int unitId = game->units->getIdForUnit(unit);
			assert(unitId >= 1 && unitId <= 0x7fffffff);
			return unitId;
		}

		return 0;
	}

	int PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(int terrObjModelId, int terrObjInstanceId)
	{
		assert(terrObjModelId != -1);
		assert(terrObjInstanceId != -1);
		assert(terrObjModelId >= 0 && terrObjModelId <= 0x7fff);
		assert(terrObjInstanceId >= 0 && terrObjInstanceId <= 0xffff);

		return (0x80000000 | (terrObjModelId << 16) | terrObjInstanceId);
	}

}

