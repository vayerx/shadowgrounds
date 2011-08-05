
#include "precompiled.h"

#include "AmmoPackObject.h"
#include "AmmoPack.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  
  AmmoPackObject::AmmoPackObject()
  {
    amount = 0;
    maxAmount = 0;
  }


  SaveData *AmmoPackObject::getSaveData()
  {
    // TODO
    return NULL;
  }


  int AmmoPackObject::getReloadPrice()
  {
    if (amount < maxAmount)
    {
      // WARNING: unsafe cast!
      AmmoPack *ap = (AmmoPack *)this->getType();
      return (((maxAmount - amount) * ap->getPrice()) / ap->getAmount());
    }    
    return 0;
  }


  void AmmoPackObject::reload()
  {
    amount = maxAmount;
  }

}

