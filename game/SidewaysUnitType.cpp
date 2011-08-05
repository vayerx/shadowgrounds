
#include "precompiled.h"

#include "SidewaysUnitType.h"
#include "SidewaysUnit.h"
#include "SidewaysUnitActor.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  SidewaysUnitType::SidewaysUnitType()
  {
    // nop
  }

  bool SidewaysUnitType::setData(char *key, char *value)
  {
    return setRootData(key, value);
  }

  bool SidewaysUnitType::setSub(char *key)
  {
    return setRootSub(key);
  }

  UnitActor *SidewaysUnitType::getActor()
  {
    return unitActorArray[UNIT_ACTOR_SIDEWAYS];
  }

  Unit *SidewaysUnitType::getNewUnitInstance(int player)
  {
    Unit *u = new SidewaysUnit(player);
    u->setUnitTypeId(unitTypeId);
    u->setUnitType(this);
    return u;
  }

}


