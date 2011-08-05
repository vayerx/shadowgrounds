
#include "precompiled.h"

#include "../convert/str2int.h"

#include "Reactor.h"

#include "../util/Debug_MemoryManager.h"

#define REACTOR_SLOTS 0


namespace game
{

  Reactor::Reactor()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = REACTOR_SLOTS;
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
    energyAmount = 0;
  }

  Reactor::Reactor(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Reactor::~Reactor()
  {
    // nop
  }

  bool Reactor::setData(char *key, char *value)
  {
    if (atSub == PARTTYPE_SUB_NONE)
    {
      if (strcmp(key, "energyamount") == 0)
      {
        int val = str2int(value);
        energyAmount = val;
        return true;
      }
    }
    return setRootData(key, value);
  }

  int Reactor::getEnergyAmount()
  {
    return energyAmount;
  }


}
