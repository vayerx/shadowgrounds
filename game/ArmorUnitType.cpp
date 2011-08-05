
#include "precompiled.h"

#include "ArmorUnitType.h"
#include "ArmorUnit.h"
#include "ArmorUnitActor.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  ArmorUnitType::ArmorUnitType()
  {
    // nop
  }

  bool ArmorUnitType::setData(char *key, char *value)
  {
    return setRootData(key, value);
  }

  bool ArmorUnitType::setSub(char *key)
  {
    return setRootSub(key);
  }

  UnitActor *ArmorUnitType::getActor()
  {
    return unitActorArray[UNIT_ACTOR_ARMOR];
  }

  Unit *ArmorUnitType::getNewUnitInstance(int player)
  {
    Unit *u = new ArmorUnit(player);
    u->setUnitTypeId(unitTypeId);
    u->setUnitType(this);
    return u;
  }

}


