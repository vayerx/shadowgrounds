
#include "precompiled.h"

#include "../convert/str2int.h"

#include "PowerCell.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  PowerCell::PowerCell()
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack"));
    assert(parentType != NULL);
  }

  PowerCell::PowerCell(int id)
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack"));
    setPartTypeId(id);
  }

  PowerCell::~PowerCell()
  {
    // nop
  }

}
