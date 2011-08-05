
#include "precompiled.h"

#include "DirectWeapon.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  DirectWeapon::DirectWeapon()
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"));
  }

  DirectWeapon::DirectWeapon(int id)
  {
    setPartTypeId(id);
  }

  DirectWeapon::~DirectWeapon()
  {
    // nop
  }

}
