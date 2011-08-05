
#include "precompiled.h"

#include "Tool.h"
#include "../util/Debug_MemoryManager.h"

#define TOOL_SLOTS 0

namespace game
{

  Tool::Tool()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = TOOL_SLOTS;
    slotTypes = NULL;
    slotPositions = NULL;
    maxDamage = 0;
    maxHeat = 0;
		for (int dmg = 0; dmg < DAMAGE_TYPES_AMOUNT; dmg++)
		{
			resistance[dmg] = 0;
			damagePass[dmg] = 0;
			damageAbsorb[dmg] = 0;
		}
  }

  Tool::Tool(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Tool::~Tool()
  {
    // nop
  }

}
