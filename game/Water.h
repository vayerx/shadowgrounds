
#ifndef WATER_H
#define WATER_H

namespace ui
{
  class Decoration;
}

namespace game
{
  class WaterManager;

  class Water
  {
    public:
      Water();

      ~Water();

      void setName(const char *name);

      void setDecoration(ui::Decoration *decor);

      void setHeight(float height);

      void setPosition(const VC3 &position);

      // based on current position and decoration...
      void updateBoundaries();

    private:
      // boundaries (rectangle)      
      float minY;
      float maxY;
      float minX;
      float maxX;
      float height; // = position.y

      char *name;

      VC3 position;

      ui::Decoration *decoration;

      friend class WaterManager;
  };
}

#endif



