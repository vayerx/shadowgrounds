
#include "precompiled.h"

#include "../convert/str2int.h"

#include "ItemPack.h"

#include "../util/Debug_MemoryManager.h"

#define ITEMPACK_SLOTS 0


namespace game
{

  ItemPack::ItemPack()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = ITEMPACK_SLOTS;
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
    amount = 0;
  }


  void ItemPack::prepareNewForInherit(PartType *partType)
  {
    this->PartType::prepareNewForInherit(partType);
    
    // WARNING!
    // TODO: should really check that the given parameter is of class!
    ItemPack *ret = (ItemPack *)partType;

    ret->amount = amount;    
  }

	void ItemPack::saveOriginals()
	{
		if (originals == NULL)
		{
			originals = new ItemPack();
		} else {
			assert(!"ItemPack::saveOriginals - Attempt to save originals multiple times.");
		}

		// FIXME: this may not be correct way to do this!
		this->prepareNewForInherit(originals);
	}


  ItemPack::ItemPack(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  ItemPack::~ItemPack()
  {
    // nop
  }

  bool ItemPack::setData(char *key, char *value)
  {
    if (atSub == PARTTYPE_SUB_NONE)
    {
      if (strcmp(key, "amount") == 0)
      {
        int val = str2int(value);
        amount = val;
        return true;
      }
    }
    return setRootData(key, value);
  }

  int ItemPack::getAmount()
  {
    return amount;
  }


}
