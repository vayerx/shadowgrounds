
#include "precompiled.h"

#include "WeaponObject.h"
#include "Weapon.h"
#include "AmmoPack.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  WeaponObject::WeaponObject()
  {
    maxAmmoAmount = 0;
    ammoAmount = 0;
  }


  SaveData *WeaponObject::getSaveData() const
  {
    // TODO
    return NULL;
  }


	const char *WeaponObject::getStatusInfo() const 
	{
		static std::string weapon_status_info_buf;
		weapon_status_info_buf = std::string("WeaponObject");
		//weapon_status_info_buf += PARTTYPE_ID_STRING_TO_INT(this->partType->getPartTypeId());
		//weapon_status_info_buf += ")";
		return weapon_status_info_buf.c_str();
	}


	int WeaponObject::getReloadPrice()
  {
    if (ammoAmount < maxAmmoAmount)
    {
      // WARNING: unsafe cast!
      Weapon *w = (Weapon *)this->getType();
      AmmoPack *ap = w->getAmmoType();
      if (ap != NULL)
        return (((maxAmmoAmount - ammoAmount) * ap->getPrice()) / ap->getAmount());
    }    
    return 0;
  }


  void WeaponObject::reload()
  {
    ammoAmount = maxAmmoAmount;
  }


}

