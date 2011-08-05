
#include "precompiled.h"

#include "Head.h"
#include "../util/Debug_MemoryManager.h"

#define HEAD_SLOTS 0

namespace game
{

  Head::Head()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = HEAD_SLOTS;
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

  Head::Head(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Head::~Head()
  {
    // nop
  }

}
