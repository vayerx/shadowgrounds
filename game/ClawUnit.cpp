
#include "precompiled.h"

#include "ClawUnit.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  ClawUnit::ClawUnit(int player)
  {
		setOwner(player);
    //unitTypeId = getUnitTypeByName("Claw");
  }

}

