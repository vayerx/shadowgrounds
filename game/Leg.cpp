
#include "precompiled.h"

#include "Leg.h"

#include "../convert/str2int.h"

#include "../util/Debug_MemoryManager.h"

#define LEG_SLOTS 0

namespace game
{

  Leg::Leg()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = LEG_SLOTS;
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
    carryingCapacity = 0;
  }

  Leg::Leg(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Leg::~Leg()
  {
    // nop
  }

  bool Leg::setData(char *key, char *value)
  {
    if (strcmp(key, "carryingcapacity") == 0)
    {
      carryingCapacity = str2int(value);
      return true;
    }
    return setRootData(key, value);
  }

}
