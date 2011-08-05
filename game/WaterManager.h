
#ifndef WATERMANAGER_H
#define WATERMANAGER_H

class LinkedList;

namespace game
{
  class Water;

  class WaterManager
  {
    public:
      WaterManager();

      ~WaterManager();

      Water *createWater();
       
      void deleteWater(Water *water);

      Water *getWaterByName(const char *name) const;

      // returns the water depth at given position (notice! depth not
      // calculated from surface, but rather from the given position's y-coord)
      // (returns water above the given position, 0 if not in water)
      float getWaterDepthAt(const VC3 &position) const;

      // recalculate highest
      void recalculate();

    private:
      LinkedList *waterList;

      // updated by the updateWater script command 
      // (which will also update the water's dimensions based on decoration)
      float highest;
  };
}

#endif



