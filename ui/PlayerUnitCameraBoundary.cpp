
#include "precompiled.h"

#include "PlayerUnitCameraBoundary.h"

#include "../game/UnitList.h"
#include "../game/Unit.h"


#define BOUNDARY_MAX_DISTANCE_FROM_UNITS 100


using namespace game;

namespace ui
{

	PlayerUnitCameraBoundary::PlayerUnitCameraBoundary(game::UnitList *unitList, int player)
	{
		this->unitList = unitList; 
		this->player = player;
	}



	PlayerUnitCameraBoundary::~PlayerUnitCameraBoundary()
	{
		// nop
	}


	bool PlayerUnitCameraBoundary::isPositionInsideBoundaries(const VC3 &position)
	{
		VC3 vec = getVectorToInsideBoundaries(position);
		if (fabs(vec.x) > 0.001f || fabs(vec.y) > 0.001f || fabs(vec.z) > 0.001f)
		{
			return false;
		} else {
			return true;
		}
	}


	VC3 PlayerUnitCameraBoundary::getVectorToInsideBoundaries(const VC3 &position)
	{

		// TODO: proper implementation
		// should not be a crappy single rectangle like this, but rather
		// circular areas around units, somehow cleverly connected.

		// now form max and min coordinates by checking each unit's
		// position. then the area formed is grown a bit.
		// notice: coordinates x,y - x,y,z. (y=z)
		float minX, maxX;
		float minY, maxY; 
		minX = maxX = position.x;
		minY = maxY = position.z;

		// loop thru all owned active units (that are not dead)
		bool firstUnit = true;
		LinkedList *ulist = unitList->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{
				VC3 pos = u->getPosition();
				if (firstUnit)
				{ 
					minX = pos.x;
					maxX = pos.x;
					minY = pos.z;
					maxY = pos.z;
					firstUnit = false;
				} else {
					if (pos.x < minX) minX = pos.x;
					if (pos.x > maxX) maxX = pos.x;
					if (pos.z < minY) minY = pos.z;
					if (pos.z > maxY) maxY = pos.z;
				}
			}
		}
		// now grow the area.
		minX -= BOUNDARY_MAX_DISTANCE_FROM_UNITS;
		minY -= BOUNDARY_MAX_DISTANCE_FROM_UNITS;
		maxX += BOUNDARY_MAX_DISTANCE_FROM_UNITS;
		maxY += BOUNDARY_MAX_DISTANCE_FROM_UNITS;
		
		// solve the vector to keep position inside the rectangle.
		VC3 ret = VC3(0,0,0);
		if (position.x < minX) ret.x = minX - position.x;
		if (position.x > maxX) ret.x = maxX - position.x;
		if (position.z < minY) ret.z = minY - position.z;
		if (position.z > maxY) ret.z = maxY - position.z;

		return ret;
	}

}

