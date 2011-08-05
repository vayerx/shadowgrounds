#ifndef INCLUDED_AI_PATHFIND_H
#define INCLUDED_AI_PATHFIND_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // Debug info truncate
#pragma warning(disable: 4514) // Unreferenced inline function
#endif

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

#include <DatatypeDef.h>

class IStorm3D_Model;

namespace game {
  class CoverMap;
}

namespace util
{
	class LightMap;
}

namespace frozenbyte {
namespace ai {

class PathSimplifier;

// This should be in general os_types.h or such
typedef unsigned short int uint16;

// Stores paths
class Path
{
	// Path in reverse order
	std::vector<int> xPositions;
	std::vector<int> yPositions;

public:
	Path();
	~Path();

	// Adds waypoint (to end of the list - start of the path)
	void addPoint(int xPosition, int yPosition);

	// Returns number of waypoints
	int getSize() const;
	// Waypoints are returned from vectors size()-1 to 0
	int getPointX(int index) const;
	int getPointY(int index) const;

	// New: for pathdeformer use...
	// Waypoint index should be from vectors size()-1 to 0
	void setPoint(int index, int xPosition, int yPosition);
};

// Pathblock (buildings, ..)
class Pathblock
{
	// NEW: ignore this, raytrace used instead of the old block map...
	// Space reserved (0 = free, 1 = block area
	/*
	std::vector<std::vector<unsigned char> > blocks;
	*/


	// Doors, ...
	std::vector<std::pair<int, int> > portals;

	// Dummy pointer. Link this to parent model
	IStorm3D_Model *model;

	int xPosition;
	int yPosition;
	int xSize;
	int ySize;

public:
	Pathblock();
	~Pathblock();

	// Units are in path's blocks
	void setSize(int xSize, int ySize); // Whole map is marked free after this
	void setPosition(int xPosition, int yPosition); // Upper-left corner
	
	// All internals should be accessible from these
	void addPortal(int xPosition, int yPosition);
	void removePortal(int xPosition, int yPosition);

	void setBlockArea(int xPosition, int yPosition); // All blocks which belong to this 
	void setFreeArea(int xPosition, int yPosition); // Mark as free space (might be blocked on pathfinder, thought)

	void setModel(IStorm3D_Model *model);

	// Query stuff
	
	int getPositionX() const;
	int getPositionY() const;
	
	int getSizeX() const;
	int getSizeY() const;

	const std::vector<std::pair<int, int> > &getPortals() const;
	// NEW: ignore this, raytrace used instead of the old block map...
	/*
	const std::vector<std::vector<unsigned char> > &getBlocks() const;
	*/

	IStorm3D_Model *getModel() const;
};

class PathFind
{
	// Height data should come from elsewhere
	//std::vector<std::vector<uint16> > heightMap;
	// ...using shared heightmap now
	// --jpk
	uint16 *heightMap;

	// There can be many obstacles on each point
	std::vector<std::vector<signed char> > obstacleMap;

	// Buildings (should probably work out some fancy hierarchy for finding these)
	std::vector<Pathblock> blockMap;

	// Heuristic value for search
	float heuristicWeight;

	// Dimensions
	int xSize;
	int ySize;
	
	//int accuracyFactor;
	int accuracyShift;

	int pathfindDepth;

	int xSizeHeightmap;
	int ySizeHeightmap;

	int coverAvoidDistance;
	int coverBlockDistance;
	int lightAvoidAmount;
	game::CoverMap *coverMap;
	util::LightMap *lightMap;

	bool portalRoutesDisabled;

	// Not implemented
	PathFind(const PathFind &rhs);
	PathFind &operator = (const PathFind &rhs);
		 
public:
	PathFind();
	~PathFind();

	void disablePortalRoutes(bool disable);

	// Set heightmap data. Loses all obstacles. AccuracyFactor sets multiplier for pathfind map size
	void setHeightMap(uint16 *heightValues, int xSize, int ySize, int accuracyFactor = 1);
	// Adds obstacle to given point (tree/..).
	void addObstacle(int xPosition, int yPosition);
	// Removes obstacle from given point
	void removeObstacle(int xPosition, int yPosition);

	// sets the cover map to use
	// used by vehicles to avoid getting to forests and near obstacles
	// NOTE: bad dependecy to game namespace!
	// -jpk
	void setCoverMap(game::CoverMap *coverMap);

	// sets the light map to use
	void setLightMap(util::LightMap *lightMap);

	// sets maximum depth for pathfind to given absolute value
	// NOTE: depth may be limited by internal absolute maximum limit too.
	void setPathfindDepthByAbsoluteValue(int depth);

	// sets maximum depth for pathfind to given relative value
	// (percentages of internal absolute maximum limit)
	void setPathfindDepthByPercentage(int depth);

	// sets the distance from which all covers (obstacles) are avoided.
	void setCoverAvoidDistance(int coverAvoidDistance);

	// sets the distance from which all covers (obstacles) are considered
	// to be blocking.
	void setCoverBlockDistance(int coverBlockDistance);

	// sets the amount above which light is avoided
	void setLightAvoidAmount(int lightAvoidAmount);

	// Pointers are stored
	void addPathblock(const Pathblock &block);

	// Set heuristics. Increasing value speeds up search while
	// affecting quality. Avoid setting less than one >:)
	// 1 -> optimal path
	// 1+ faster search, lower quality path
	void setHeuristicWeight(float value = 1.f);

	// Finds route between points. Returns true if path exists
	// Climb penaly means cost factor for each unit moved upwards
	//	-> cost = realCost() + heightDelta*climbPenalty
	// Eg. step from 0,0 to 1,1 always costs sqrt(2.f) (w/o climbing).
	//	-> With climbFactor 1.f
	//	-> Climbing 100 units raises cost to (sqrt(2)+100) !
	// So, scale both maxHeightDifference and climbFactor to suit your scene
	bool findRoute(Path *resultPath, int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference, float climbPenalty, const VC3 &startWorldCoords, const VC3 &endWorldCoords) const;

	// Returns true if point is blocked (obstacle)
	bool isBlocked(int xPosition, int yPosition) const;

	// Returns amount of obstacles at given point
	// (An optimization hack really) --jpk
	int getBlockingCount(int xPosition, int yPosition) const;

	// NOTE: for loading of the pathfind map...
	void setBlockingCount(int xPosition, int yPosition, int amount);

	// Returns model (from pathblocks) at given point. 0 if not found
	IStorm3D_Model *getModelAt(int xPosition, int yPosition, const VC3 &worldCoords) const;

private:
	bool findActualRoute(Path *resultPath, int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference, float climbPenalty, const Pathblock *avoidBlock) const;
	
	// Returns height at given point
	int getHeight(int xPosition, int yPosition) const;

	// Returns true if can move from start to end.
	// Points are assumed to be neighbours
	bool isMovable(int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference) const;

	// Distance costs are no longer squared

	// Returns estimated cost between points (already multipled with heuristic weight)
	float estimateCost(int xStart, int yStart, int xEnd, int yEnd) const;
	// Returns real cost between neighbours
	float realCost(int xStart, int yStart, int xEnd, int yEnd, float climbPenalty) const;

	// blockid (index), -1 = none
	int blockAt(int xPosition, int yPosition, const VC3 &worldCoords) const;
	// 0 = free, 1 = belongs to block
	int blockedAt(int xPosition, int yPosition, const Pathblock &block) const;

	friend class PathSimplifier;
};

} // end of namespace ai
} // end of namespace frozenbyte

#endif
