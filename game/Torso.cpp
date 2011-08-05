
#include "precompiled.h"

#include "Head.h"
#include "Leg.h"
#include "Arm.h"
#include "Torso.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{
  /*
  // we should make these with new when creating the torso object.
  // would leak memory, but don't care, because these objects are created 
  // only once and kept alive through the whole program run time.
  // now there's just null pointers...
  PartType *torso_slots[TORSO_SLOTS] = 
  {
    getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Head")), 
    getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm")), 
    getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm")), 
    getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg")), 
    getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg")) 
  };

  int torso_positions[TORSO_SLOTS] =
  { 
    SLOT_POSITION_HEAD,
    SLOT_POSITION_LEFT_ARM,
    SLOT_POSITION_RIGHT_ARM,
    SLOT_POSITION_LEFT_LEG,
    SLOT_POSITION_RIGHT_LEG,
  };
  */

  Torso::Torso()
  {
    image = NULL;
    parentType = &partType;
    slotAmount = 0;
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

  Torso::Torso(int id)
  {
    parentType = &partType;
    setPartTypeId(id);
  }

  Torso::~Torso()
  {
    // nop
  }

}
