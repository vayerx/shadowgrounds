
#ifndef GAMECOLLISIONINFO_H
#define GAMECOLLISIONINFO_H

#include <c2_vectors.h>

#include "Part.h"
#include "Unit.h"

namespace game
{

  class GameCollisionInfo
  {
  public:
    GameCollisionInfo() 
    { 
      hit = false; 
      hitUnit = false; 
      hitGround = false; 
      hitBuilding = false; 
      hitTerrainObject = false; 
      position = VC3(0,0,0); 
      hitPlaneNormal = VC3(0,0,0); 
      unit = NULL; 
      part = NULL;
      range = 0;
			terrainInstanceId = -1;
			terrainModelId = -1;
    };
    bool hit; // hit somewhere on map or some unit
    bool hitGround; // hit to the ground
    bool hitBuilding; // hit to a building
    bool hitTerrainObject; // hit to a tree or something alike
    bool hitUnit; // hit to a unit
    VC3 position; // hit coordinates (of unit if not null, else just map)
    VC3 hitPlaneNormal; // hit plane normal direction - valid in some cases only!
    Unit *unit; // hit unit or null if did not hit a unit, can't be const
    Part *part; // hit part of the unit
		int terrainInstanceId; // mapping to terrain object model instances
		int terrainModelId; // mapping to terrain object model instances
    float range;
  };

}

#endif
