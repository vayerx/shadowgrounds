
#ifndef INDIRECTWEAPON_H
#define INDIRECTWEAPON_H

#include "Weapon.h"

namespace game
{

  class IndirectWeapon : public Weapon
  {
  public:
    IndirectWeapon();
    IndirectWeapon(int id);
    virtual ~IndirectWeapon();
  };

}

#endif

