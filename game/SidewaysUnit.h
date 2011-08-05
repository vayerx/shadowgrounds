
#ifndef SIDEWAYSUNIT_H
#define SIDEWAYSUNIT_H

#include "Unit.h"

namespace game
{

  /**
	 * The AOV sideways units.
   *
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @version 1.0, 29.10.2006 
   * @see Unit
   * @see SidewaysUnitActor
   */

  class SidewaysUnit : public Unit
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
    SidewaysUnit(int player);
  };

}

#endif

