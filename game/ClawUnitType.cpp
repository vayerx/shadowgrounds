
#include "precompiled.h"

#include "ClawUnitType.h"
#include "ClawUnit.h"
#include "ClawUnitActor.h"
#include "unittypes.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  ClawUnitType::ClawUnitType()
  {
    // nop
  }

  bool ClawUnitType::setData(char *key, char *value)
  {
    return setRootData(key, value);
  }

  bool ClawUnitType::setSub(char *key)
  {
    return setRootSub(key);
  }

  UnitActor *ClawUnitType::getActor()
  {
    return unitActorArray[UNIT_ACTOR_CLAW];
  }

  Unit *ClawUnitType::getNewUnitInstance(int player)
  {
    Unit *u = new ClawUnit(player);
    u->setUnitTypeId(unitTypeId);
    u->setUnitType(this);
    return u;
  }

}


