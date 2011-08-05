
#include "precompiled.h"


#include "UnitSelections.h"

#include "Unit.h"
#include "UnitList.h"


namespace game
{

  UnitSelections::UnitSelections(UnitList *unitList, int player)
  {
    this->unitList = unitList;
    this->player = player;
    listener = NULL;
    unitsSelected = 0;
  }


  void UnitSelections::reset()
  {
    unitsSelected = 0;

    LinkedList *ulist = unitList->getOwnedUnits(player);
    LinkedListIterator iter = LinkedListIterator(ulist);
    while (iter.iterateAvailable())
    { 
      Unit *u = (Unit *)iter.iterateNext();
      if (u->isSelected())
      {
        unitsSelected++;
      }
    }
  }


  void UnitSelections::setListener(IUnitSelectionListener *listener)
  {
    this->listener = listener;
  }


  void UnitSelections::selectUnit(Unit *unit, bool selected)
  {
    assert(unit->isActive());
    //assert(!unit->isDestroyed() || !selected);

    if ((unit->isSelected() && !selected)
      || (!unit->isSelected() && selected &&
			!unit->isDestroyed() && unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS))
    {
      unit->setSelected(selected);
      if (selected)
        unitsSelected++;
      else
        unitsSelected--;

      if (listener != NULL)
      {
        listener->unitSelectionEvent(unit);
      }
    }
  }


  void UnitSelections::selectAllUnits(bool selected)
  {
    LinkedList *ulist = unitList->getOwnedUnits(player);
    LinkedListIterator iter = LinkedListIterator(ulist);
    while (iter.iterateAvailable())
    { 
      Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive())
			{
				if (u->isSelected() && !selected)
				{
					selectUnit(u, selected);
				} else {
					if (!u->isSelected() && selected
						&& !u->isDestroyed()
						&& u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					{
						selectUnit(u, selected);
					}
				}
			}
    }
  }
 
}

