
#include "precompiled.h"

#include "SidewaysUnit.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  SidewaysUnit::SidewaysUnit(int player)
  {
		setOwner(player);
    //unitTypeId = getUnitTypeByName("Sideways");
  }

}

