
#ifndef UNITVISIBLITYCHECKER_H
#define UNITVISIBLITYCHECKER_H

#include "../container/LinkedList.h"

// visibility of one unit is checked in this many passes.
// (one pass to apply changes when all units done.)
// so value 2 means 2 game ticks = 20ms for one unit.
// thus, if we have 100 units, it takes 2 seconds for visibilities to 
// to be updated.
#define VISIBILITY_CHECK_IN_PASSES 1

// NOTICE! rate of checks per seconds depends on GAME_TICKS_PER_SECOND
// (if 50Hz, checking all units takes twice as long as it would take
// for 100Hz)


// TODO: visibility checker may not work properly if units are 
// spawned in the middle of combat (= while in mission). 
// Probably all new units will be checked during the last pass.
// This may cause cpu usage spikes.
// Or possibly they are not checked at all.
// FIXME: Check and fix that!


namespace game
{
  class Game;
  class Unit;

  class UnitVisibilityChecker
  {
  public:
    UnitVisibilityChecker(game::Game *game);
    ~UnitVisibilityChecker();

    void restart();
    void runCheck();

		void setUpdateEnabled(bool enabled);

		void unitSeesUnit(Unit *unit, Unit *other, float distance);

  private:
    bool runCheckImpl();

    game::Game *game;
    LinkedListIterator *unitIterator;
    LinkedListIterator *otherUnitIterator;
    int totalUnits;
    int unitsInPass;
    int passCount;
    game::Unit *currentUnit;
		bool updateDisabled;
  };

}

#endif

