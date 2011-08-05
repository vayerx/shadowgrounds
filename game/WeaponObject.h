
#ifndef WEAPONOBJECT_H
#define WEAPONOBJECT_H

#include "Part.h"

namespace game
{

  class WeaponObject : public Part
  {
  protected:
    int ammoAmount;
    int maxAmmoAmount;

  public:

    WeaponObject();

    virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

    int getReloadPrice();
    void reload();

    inline int getAmmoAmount() { return ammoAmount; }
    inline void setAmmoAmount(int amount) { this->ammoAmount = amount; }
    inline int getMaxAmmoAmount() { return maxAmmoAmount; }
    inline void setMaxAmmoAmount(int maxAmount) { this->maxAmmoAmount = maxAmount; }
  };

}

#endif

