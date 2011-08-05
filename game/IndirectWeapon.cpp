
#include "precompiled.h"

#include "IndirectWeapon.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  IndirectWeapon::IndirectWeapon()
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"));
  }

  IndirectWeapon::IndirectWeapon(int id)
  {
    setPartTypeId(id);
  }

  IndirectWeapon::~IndirectWeapon()
  {
    // nop
  }

}
