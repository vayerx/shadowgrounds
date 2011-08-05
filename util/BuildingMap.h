#ifndef INCLUDED_BUILDINGMAP_H
#define INCLUDED_BUILDINGMAP_H

#include <vector>

// value used in floormap to tell that the block has no floor height
#define BUILDINGMAP_NO_FLOOR_BLOCK -126


#include "../util/Floodfill.h"

// Forward declarations
class IStorm3D_Model;

namespace frozenbyte {

// Forward declaration
struct BuildingMapData;

class BuildingMap
{
	BuildingMapData *data;

	// Not implemented
	BuildingMap &operator = (const BuildingMap &rhs);
	BuildingMap (const BuildingMap &);

public:	
	//BuildingMap(const char *fileName, IStorm3D_Model *model, int rotationDegrees=0);
	BuildingMap(const char *fileName, IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ);
	~BuildingMap();

	// Odd sizes on maps. Model origo on center. Resolution is 0.5 units (meters)

	// 1 -> Blocked, 0 -> free
	const std::vector<std::vector<unsigned char> > &getObstacleMap() const;
	
	// Height in units (meters). 
	// Terrain should be set to height on models origo IFF height from here is not 0
	const std::vector<std::vector<unsigned char> > &getHeightMap() const;

	const std::vector<std::vector<char> > &getFloorHeightMap() const;

	const std::vector<std::pair<int, int> > &getDoors() const;

	// returns true if this buildingmap uses a floormap, else false
	bool hasFloorHeightMap() const;

	// Returns the height scale for this buildingmap.
	// (in reality this is a constant same for all buildingmaps)
	// divide the heightmap values by this value to get real values.
	// -jpk
	float getHeightScale() const;
	float getMapResolution() const;
};



} // end of namespace frozenbyte

#endif
