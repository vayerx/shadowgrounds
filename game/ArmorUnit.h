
#ifndef ARMORUNIT_H
#define ARMORUNIT_H

#include "Unit.h"

namespace game
{

  /**
   * "Tactical Combat Armor" type unit.
   * Usually, all human player units are of this type.
   *
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @version 1.0, 7.7.2002 
   * @see Unit
   * @see ArmorUnitActor
   */

  class ArmorUnit : public Unit
  {
  public:

    /** 
     * Creates a new armor unit for the given player.
     * After creating the unit, you probably want to store it to
     * UnitList.
     * @param owner  int, the number of the player owning this unit or 
     * NO_UNIT_OWNER if no owner. Player number must be withing allowed
     * range (0 .. ABS_MAX_PLAYERS-1) or the object will be considered 
     * invalid.
     */
    ArmorUnit(int player);
  };

}

#endif

