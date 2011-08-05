
#include "precompiled.h"

#include "Arm.h"
#include "../util/Debug_MemoryManager.h"

#define ARM_SLOTS 0

namespace game
{

  Arm::Arm()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = ARM_SLOTS;
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

  Arm::Arm(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Arm::~Arm()
  {
    // nop
  }

}
