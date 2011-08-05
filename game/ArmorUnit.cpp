
#include "precompiled.h"

#include "ArmorUnit.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  ArmorUnit::ArmorUnit(int player)
  {
		setOwner(player);
    //unitTypeId = getUnitTypeByName("Armor");
  }

}

