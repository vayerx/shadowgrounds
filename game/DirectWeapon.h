
#ifndef DIRECTWEAPON_H
#define DIRECTWEAPON_H

#include "Weapon.h"

namespace game
{

  class DirectWeapon : public Weapon
  {
  public:
    DirectWeapon();
    DirectWeapon(int id);
    virtual ~DirectWeapon();
  };

}

#endif

