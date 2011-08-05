
#include "precompiled.h"

#include "../convert/str2int.h"

#include "AmmoPack.h"
#include "AmmoPackObject.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  AmmoPack::AmmoPack()
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack"));
    assert(parentType != NULL);
  }

  AmmoPack::AmmoPack(int id)
  {
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack"));
    setPartTypeId(id);
  }

  AmmoPack::~AmmoPack()
  {
    // nop
  }

  Part *AmmoPack::getNewPartInstance()
  {
    AmmoPackObject *ret = new AmmoPackObject();
    ret->setType(this);
    //ret->setAmount(this->getAmount());
    ret->setAmount(0);
    ret->setMaxAmount(this->getAmount());
    return ret;
  } 

}

