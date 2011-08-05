
#ifndef UNITSELECTIONS_H
#define UNITSELECTIONS_H


namespace game
{
  class Unit;
  class UnitList;


  class IUnitSelectionListener
  {
  public:
	  virtual ~IUnitSelectionListener() {}
    virtual void unitSelectionEvent(Unit *unit) = 0;
  };


  class UnitSelections
  {
  private:
    int unitsSelected; 
    IUnitSelectionListener *listener;
    int player;
    UnitList *unitList;

  public:
    UnitSelections(UnitList *unitList, int player);

    void reset();
     
    inline int getUnitsSelected() { return unitsSelected; }

    void setListener(IUnitSelectionListener *listener);

    void selectUnit(Unit *unit, bool selected);
    void selectAllUnits(bool selected);

  };

}

#endif

