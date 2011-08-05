
#include "precompiled.h"

#include "AI_PathFind.h"
#include <cassert>
#include <queue>
#include "../game/CoverMap.h"
#include "LightMap.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include <IStorm3D_Model.h>

#include <map>
#include <math.h>

//#define AI_PATHFIND_MAX_DEPTH 16000
//#define AI_PATHFIND_HASHMAP_SIZE (2029 * 4 - 3)
const int AI_PATHFIND_MAX_DEPTH = 4000;
const int AI_PATHFIND_HASHMAP_SIZE = 16097; // 61111; // 16097;

const float AI_MAX_COST_ESTIMATE = 60.f * 8.f;
//const float AI_MAX_COST_PORTAL_ESTIMATE = 30.f * 8.f;
const float AI_MAX_COST_PORTAL_ESTIMATE = 30.f * 8.f;

#ifdef _MSC_VER
#define inline __forceinline
#pragma intrinsic(sqrt)
#endif

int pathfind_findroutes_since_last_clear = 0;
int pathfind_findroutes_failed_since_last_clear = 0;


namespace frozenbyte {
namespace ai {

/** class Path **/

Path::Path()
{
}

Path::~Path()
{
}

void Path::addPoint(int xPosition, int yPosition)
{
	assert(xPositions.size() == yPositions.size());
	
	xPositions.push_back(xPosition);
	yPositions.push_back(yPosition);
}

int Path::getSize() const
{
	assert(xPositions.size() == yPositions.size());
	return xPositions.size();
}

int Path::getPointX(int index) const
{
	assert((index >= 0) && (index < getSize()));
	return xPositions[xPositions.size() - 1 - index];
}

int Path::getPointY(int index) const
{
	assert((index >= 0) && (index < getSize()));
	return yPositions[yPositions.size() - 1 - index];
}

void Path::setPoint(int index, int xPosition, int yPosition)
{
	assert((index >= 0) && (index < getSize()));
	this->xPositions[xPositions.size() - 1 - index] = xPosition;
	this->yPositions[yPositions.size() - 1 - index] = yPosition;
}

/** class Pathblock **/

Pathblock::Pathblock()
{
	xPosition = 0;
	yPosition = 0;
	model = 0;
}

Pathblock::~Pathblock()
{
}

void Pathblock::setSize(int xSize, int ySize)
{
	this->xSize = xSize;
	this->ySize = ySize;

	// NEW: ignore this, raytrace used instead of the old block map...
	/*
	blocks.resize(xSize);
	for(int i = 0; i < xSize; ++i)
		blocks[i].resize(ySize);

	// Clear
	for(int j = 0; j < ySize; ++j)
	for(int i = 0; i < xSize; ++i)
		blocks[i][j] = 0;
	*/
}

void Pathblock::setPosition(int xPosition, int yPosition)
{
	this->xPosition = xPosition;
	this->yPosition = yPosition;
}

void Pathblock::addPortal(int xPosition, int yPosition)
{
	if(xPosition < 0 || xPosition >= xSize || yPosition < 0 || yPosition >= ySize)
	{
		Logger::getInstance()->error("addPortal - Ignored door helper which was outside building area");
		return;
	}
	
	//assert(xPosition >= 0);
	//assert(yPosition >= 0);

	portals.push_back(std::pair<int, int> (xPosition, yPosition));
}

void Pathblock::removePortal(int xPosition, int yPosition)
{
	std::pair<int, int> position(xPosition, yPosition);
	
	for(unsigned int i = 0; i < portals.size(); ++i)
	{
		if(position == portals[i])
		{
			portals.erase(portals.begin() + i);
			return;
		}
	}

	assert(!"Portal not found");
}

void Pathblock::setBlockArea(int xPosition, int yPosition)
{
	// NEW: ignore this, raytrace used instead of the old block map...
	/*
	assert((xPosition >= 0) && (xPosition < xSize));
	assert((yPosition >= 0) && (yPosition < ySize));

	blocks[xPosition][yPosition] = 1;
	*/
}

void Pathblock::setFreeArea(int xPosition, int yPosition)
{
	// NEW: ignore this, raytrace used instead of the old block map...
	/*
	assert((xPosition >= 0) && (xPosition < xSize));
	assert((yPosition >= 0) && (yPosition < ySize));

	blocks[xPosition][yPosition] = 0;
	*/
}

void Pathblock::setModel(IStorm3D_Model *model)
{
	this->model = model;
}

int Pathblock::getPositionX() const
{
	return xPosition;
}

int Pathblock::getPositionY() const
{
	return yPosition;
}

int Pathblock::getSizeX() const
{
	return xSize;
}

int Pathblock::getSizeY() const
{
	return ySize;
}

const std::vector<std::pair<int, int> > &Pathblock::getPortals() const
{
	return portals;
}

// NEW: ignore this, raytrace used instead of the old block map...
/*
const std::vector<std::vector<unsigned char> >&Pathblock::getBlocks() const
{
	return blocks;
}
*/

IStorm3D_Model *Pathblock::getModel() const
{
	return model;
}

namespace {

// struct Node, stores pathfinding info
struct Node
{
	// Indices in map coordinates
	int xIndex;
	int yIndex;

	int xParentIndex;
	int yParentIndex;

	float realCost; // Real cost to this node
	float totalCost; // Real + estimate to end

	Node()
	:	xIndex(-1),
		yIndex(-1),
		xParentIndex(-1),
		yParentIndex(-1),
		realCost(.0f),
		totalCost(.0f)
	{
	}

};

// List of nodes. We can use indices here as instances
// Reduces copy penalties a _lot_
std::vector<Node> nodes;
int usedNodes = 0;

int getNewNode()
{
	nodes.push_back(Node());
	return usedNodes++;
}

void resetNodes()
{
	usedNodes = 0;
	nodes.clear();
}

// Used on sorting in binary heap
inline bool operator < (const Node &lhs, const Node &rhs)
{
	return lhs.totalCost < rhs.totalCost;
}

inline bool operator > (const Node &lhs, const Node &rhs)
{
	return lhs.totalCost > rhs.totalCost;
}

// Equality
inline bool operator == (const Node &lhs, const Node &rhs)
{
	if(lhs.xIndex != rhs.xIndex)
		return false;
	if(lhs.yIndex != rhs.yIndex)
		return false;

	return true;
}

inline bool operator != (const Node &lhs, const Node &rhs)
{
	return !(lhs == rhs);
}

/* Custom closed hash for speed */
class ClosedHash
{
	// (key, index, key, index, ... )
	int *array; // -1 empty, -2 removed, 0+ index

public:
	ClosedHash()
	{
		array = new int[AI_PATHFIND_HASHMAP_SIZE * 2 + 8];
		clear();
	}

	~ClosedHash()
	{
		delete[] array;
	}

	inline void insert(int nodeIndex, int nodeData)
	{
		int defaultIndex = nodeIndex % AI_PATHFIND_HASHMAP_SIZE;

		for(int i = 0; ; ++i)
		{
			int index = (defaultIndex + i*i) % AI_PATHFIND_HASHMAP_SIZE;
			index <<= 1;

			assert(index >= 0);

			if(array[index] < 0)
			{
				array[index] = nodeIndex;
				array[index + 1] = nodeData;

				break;
			}

			// Array size has to be > 2*indices and prime number
			assert(i < AI_PATHFIND_HASHMAP_SIZE);
		}
	}

	inline void update(int nodeIndex, int nodeData)
	{
		int defaultIndex = nodeIndex % AI_PATHFIND_HASHMAP_SIZE;

		for(int i = 0; ; ++i)
		{
			int index = (defaultIndex + i*i) % AI_PATHFIND_HASHMAP_SIZE;
			index <<= 1;

			assert(index >= 0);

			if(array[index] == -2)
				continue;
			if(array[index] == -1)
				break;

			// Found?
			if(array[index] == nodeIndex)
			{
				array[index + 1] = nodeData;
				return;
			}

			// Array size has to be > 2*indices and prime number
			assert(i < AI_PATHFIND_HASHMAP_SIZE);
		}
	}

	inline int find(int nodeIndex)
	{
		int defaultIndex = nodeIndex % AI_PATHFIND_HASHMAP_SIZE;

		for(int i = 0; ; ++i)
		{
			int index = (defaultIndex + i*i) % AI_PATHFIND_HASHMAP_SIZE;
			index <<= 1;

			assert(index >= 0);

			if(array[index] == -2)
				continue;
			if(array[index] == -1)
				break;

			// Compare here
			if(array[index] == nodeIndex)
				return array[index + 1];

			// Array size has to be > 2*indices and prime number
			assert(i < AI_PATHFIND_HASHMAP_SIZE);
		}

		return -1;
	}

	inline void erase(int nodeIndex)
	{
		int defaultIndex = nodeIndex % AI_PATHFIND_HASHMAP_SIZE;

		for(int i = 0; ; ++i)
		{
			int index = (defaultIndex + i*i) % AI_PATHFIND_HASHMAP_SIZE;
			index <<= 1;

			assert(index >= 0);
			assert(array[index] != -1);

			// Compare here
			if(array[index] == nodeIndex)
			{
				array[index] = -2;
				break;
			}

			// Array size has to be > 2*indices and prime number
			assert(i < AI_PATHFIND_HASHMAP_SIZE);
		}
	}

	void clear()
	{
		for(int i = 0; i < AI_PATHFIND_HASHMAP_SIZE << 1; i += 2)
			array[i] = -1;
	}
};

/* Custom binary heap */
class PriorityQueue
{
	// Container
	int *array;
	int arraySize;
	int arrayPosition;

	// Maps map indices to array index
	ClosedHash indexHash;
	// Map node indices to array indices
	//int *indexArray;

public:
	PriorityQueue()
	{
		arraySize = AI_PATHFIND_MAX_DEPTH * 10 + 10; // extreme case
		array = new int[arraySize];

		clear();
	}

	~PriorityQueue()
	{
		delete[] array;
	}

	// Stores node on queue
	inline void push(int index, int mapIndex, int mapSizeX)
	{
		assert(find(mapIndex) < 0);

		// Set as last element
		array[arrayPosition] = index;
		indexHash.insert(mapIndex, arrayPosition);
		
		// Correct place
		slideDown(arrayPosition, mapSizeX);

		++arrayPosition;
		assert(arrayPosition < arraySize);
	}

	// Removes top node
	inline void pop(int mapSizeX)
	{
		assert(empty() == false);

		// Last node as root
		int root = array[arrayPosition - 1];

		int mapIndex = nodes[root].yIndex * mapSizeX + nodes[root].xIndex;
		//indexHash.erase(mapIndex);
		//indexHash.insert(mapIndex, 1);
		indexHash.update(mapIndex, 1);
		
		int oldMapIndex = nodes[array[1]].yIndex * mapSizeX + nodes[array[1]].xIndex;
		indexHash.erase(oldMapIndex);

		array[1] = root;

		// Remove last
		--arrayPosition;
		assert(arrayPosition >= 1);

		slideUp(1, mapSizeX);
	}

	// Returns top node
	inline int top(int mapSizeX)
	{
#ifdef _DEBUG
		int mapIndex = nodes[array[1]].yIndex * mapSizeX + nodes[array[1]].xIndex;
		int index = indexHash.find(mapIndex);
		assert(index == 1);
#endif
		return array[1];
	}
	// True if container is empty
	inline bool empty()
	{
		// Dummy node is always there
		if(arrayPosition > 1)
			return false;
		else
			return true;
	}

	// -1 if not found
	inline int find(int mapIndex)
	{
		int index = indexHash.find(mapIndex);
		if(index >= 0)
			return array[index];
		else
			return -1;
	}

	inline void erase(int mapIndex, int mapSizeX)
	{
		// Remove
		int removedIndex = indexHash.find(mapIndex);
		indexHash.erase(mapIndex);

		// Replace current with last element and slide
		int newValue = array[arrayPosition - 1];
		array[removedIndex] = newValue;

		int newMapIndex = nodes[newValue].yIndex * mapSizeX + nodes[newValue].xIndex;

		// If removing last entry, don't continue
		if(mapIndex == newMapIndex)
		{
			--arrayPosition;
			return;
		}

		indexHash.update(newMapIndex, removedIndex);

		// Remove last
		--arrayPosition;
		slideUp(removedIndex, mapSizeX);
	}

	// Clears content
	inline void clear()
	{
		indexHash.clear();
		arrayPosition = 1;
	}

private:
	// Slide given index up in the hierarchy
	inline void slideUp(int index, int mapSizeX)
	{
		int currentIndex = index;

		// While we still have childs
		while(currentIndex * 2 < arrayPosition)
		{
			// Default to left child
			int childIndex = 2 * currentIndex;
			
			// Change to right child?
			if(childIndex < arrayPosition - 1)
			if(nodes[array[childIndex + 1]] < nodes[array[childIndex]])
				++childIndex;

			// Bigger than it's smaller child?
			if(nodes[array[childIndex]] < nodes[array[currentIndex]])
			{
				int hashA = nodes[array[childIndex  ]].yIndex * mapSizeX + nodes[array[childIndex  ]].xIndex;
				int hashB = nodes[array[currentIndex]].yIndex * mapSizeX + nodes[array[currentIndex]].xIndex;

				// Update hash
				//indexHash.erase(hashA);
				//indexHash.erase(hashB);
				//indexHash.insert(hashA, currentIndex);
				//indexHash.insert(hashB, childIndex);
				indexHash.update(hashA, currentIndex);
				indexHash.update(hashB, childIndex);

				std::swap(array[childIndex], array[currentIndex]);
			}
			else
				break;
				
			currentIndex = childIndex;
		}
	}
	
	// Slide given index down on hierarchy
	inline void slideDown(int index, int mapSizeX)
	{
		assert(index >= 1);
		int currentIndex = index;

		// Until we hit the root
		while(currentIndex > 1)
		{
			// Current node still smaller than it's parent?
			if(nodes[array[currentIndex]] < nodes[array[currentIndex / 2]])
			{
				int hashA = nodes[array[currentIndex  ]].yIndex * mapSizeX + nodes[array[currentIndex  ]].xIndex;
				int hashB = nodes[array[currentIndex/2]].yIndex * mapSizeX + nodes[array[currentIndex/2]].xIndex;
				
				// Update hash
				//indexHash.erase(hashA);
				//indexHash.erase(hashB);
				//indexHash.insert(hashA, currentIndex / 2);
				//indexHash.insert(hashB, currentIndex);
				indexHash.update(hashA, currentIndex / 2);
				indexHash.update(hashB, currentIndex);

				std::swap(array[currentIndex], array[currentIndex / 2]);
			}
			else
				break;
				
			currentIndex /= 2;
		}
	}
};

} // end of nameless namespace

// class PathFind

#ifdef PATHFIND_DEBUG
unsigned char *pathfind_debug_data = NULL;
#endif

PathFind::PathFind()
:	heuristicWeight(2.0f),
	xSize(-1),
	ySize(-1),
	accuracyShift(1),
	pathfindDepth(AI_PATHFIND_MAX_DEPTH),
	xSizeHeightmap(-1),
	ySizeHeightmap(-1),
	coverAvoidDistance(0),
	coverBlockDistance(0),
	coverMap(0),
	lightMap(0),
	portalRoutesDisabled(false)
{	
}

PathFind::~PathFind()
{
}

void PathFind::disablePortalRoutes(bool disable)
{
	portalRoutesDisabled = disable;
}

void PathFind::setHeightMap(uint16 *heightValues, int xSize_, int ySize_, int accuracyFactor)
{
	xSizeHeightmap = xSize_;
	ySizeHeightmap = ySize_;

	// Convert factor to shift value (assumes powers of 2).
	// I guess there should be some better way for this :)
	if(accuracyFactor == 1)
		accuracyShift = 0;
	else if(accuracyFactor == 2)
		accuracyShift = 1;
	else if(accuracyFactor == 4)
		accuracyShift = 2;
	else if(accuracyFactor == 8)
		accuracyShift = 3;
	else
	{
		assert(!"Whoops.");
	}	
	
	xSize = accuracyFactor * xSize_;
	ySize = accuracyFactor * ySize_;

	heightMap = heightValues;

	obstacleMap.resize(xSize);

	for(int i = 0; i < xSize; ++i)
		obstacleMap[i].resize(ySize);

#ifdef PATHFIND_DEBUG
	if (pathfind_debug_data != NULL)
	{
		delete [] pathfind_debug_data;
		pathfind_debug_data = NULL;
	}
	if (pathfind_debug_data == NULL)
	{
		pathfind_debug_data = new unsigned char[xSize * ySize];
		for (int i = 0; i < xSize * ySize; i++)
		{
			pathfind_debug_data[i] = 0;
		}
	}
#endif
}

void PathFind::addObstacle(int xPosition, int yPosition)
{
	++obstacleMap[xPosition][yPosition];
}

void PathFind::removeObstacle(int xPosition, int yPosition)
{
	//assert(obstacleMap[xPosition][yPosition] > 0);
	if(obstacleMap[xPosition][yPosition] > 0)
		--obstacleMap[xPosition][yPosition];
}

void PathFind::addPathblock(const Pathblock &block)
{
	blockMap.push_back(block);
}

void PathFind::setHeuristicWeight(float value)
{
	assert(value > .99f);
	heuristicWeight = value;
}

bool PathFind::findRoute(Path *resultPath, int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference, float climbPenalty, const VC3 &startWorldCoords, const VC3 &endWorldCoords) const
{
	assert(resultPath);
	assert(resultPath->getSize() == 0);

	pathfind_findroutes_since_last_clear++;

	int startId = blockAt(xStart, yStart, startWorldCoords);
	int endId = blockAt(xEnd, yEnd, endWorldCoords);

#ifdef PATHFIND_DEBUG
	if (pathfind_debug_data != NULL)
	{
		for (int i = 0; i < xSize * ySize; i++)
		{
			pathfind_debug_data[i] = 0;
		}
	}
#endif

	// Normal query
// TEMP: Temporarily removed this check! restore it!!!!
//Logger::getInstance()->debug("TEMP - patfind doors code, restore me!!!");
	
	if(startId == endId || portalRoutesDisabled)
	{
		float costEstimate = estimateCost(xStart, yStart, xEnd, yEnd) / heuristicWeight;
		if(costEstimate > AI_MAX_COST_ESTIMATE)
		{
			Logger::getInstance()->warning("Too long path specified - discarding");
			Logger::getInstance()->warning(int2str(int(costEstimate / 8.f)));
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}
	
		return findActualRoute(resultPath, xStart, yStart, xEnd, yEnd, maxHeightDifference, climbPenalty, 0);
	}

	Logger::getInstance()->debug("PathFind::findRoute - Attempting to find path thru doors.");

	// Find route through doors
	int xStartPortalPosition = -1;
	int yStartPortalPosition = -1;
	int xEndPortalPosition = -1;
	int yEndPortalPosition = -1;

	if(startId >= 0)
	{
		int xBlockPosition = blockMap[startId].getPositionX();
		int yBlockPosition = blockMap[startId].getPositionY();
		
		const std::vector<std::pair<int, int> > &portals = blockMap[startId].getPortals();
		float smallestCost = 99999999999.f;

		if(portals.empty())
		{
			Logger::getInstance()->warning("findRoute - Building has no portals!");
			startId = -1;
		}

		// Find closest (estimate) to starting position
		for(unsigned int i = 0; i < portals.size(); ++i)
		{
			int xTargetPosition = portals[i].first + xBlockPosition;
			int yTargetPosition = portals[i].second + yBlockPosition;
			
			//float currentCost = estimateCost(xStart, yStart, xTargetPosition, yTargetPosition);
		
			// Find closest door to target point
			float currentCost = estimateCost(xTargetPosition, yTargetPosition, xEnd, yEnd);
			if(currentCost < smallestCost)
			{
				smallestCost = currentCost;

				xStartPortalPosition = xTargetPosition;
				yStartPortalPosition = yTargetPosition;
			}
		}
	}

	if(endId >= 0)
	{
		int xBlockPosition = blockMap[endId].getPositionX();
		int yBlockPosition = blockMap[endId].getPositionY();

		const std::vector<std::pair<int, int> > &portals = blockMap[endId].getPortals();
		float smallestCost = 99999999999.f;

		if(portals.empty())
		{
			Logger::getInstance()->warning("findRoute - Building has no portals!");
			endId = -1;
		}

		int xPathStart = (xStartPortalPosition >= 0) ? xStartPortalPosition : xStart;
		int yPathStart = (yStartPortalPosition >= 0) ? yStartPortalPosition : yStart;

		// estimateCost(start,portal) + estimateCost(portal,end) smallest
		for(unsigned int i = 0; i < portals.size(); ++i)
		{
			int xTargetPosition = portals[i].first + xBlockPosition;
			int yTargetPosition = portals[i].second + yBlockPosition;

			float currentCost = estimateCost(xPathStart, yPathStart, xTargetPosition, yTargetPosition);
			currentCost += estimateCost(xTargetPosition, yTargetPosition, xEnd, yEnd);

			//float currentCost = estimateCost(xTargetPosition, yTargetPosition, xEnd, yEnd);

			if(currentCost < smallestCost)
			{
				smallestCost = currentCost;

				xEndPortalPosition = xTargetPosition;
				yEndPortalPosition = yTargetPosition;
			}
		}
	}

	{
		float costEstimate = 0.f;
		if(startId >= 0)
		{
			costEstimate = estimateCost(xStart, yStart, xStartPortalPosition, yStartPortalPosition);
			if(endId >= 0)
			{
				costEstimate += estimateCost(xStartPortalPosition, yStartPortalPosition, xEndPortalPosition, yEndPortalPosition);
				costEstimate += estimateCost(xEndPortalPosition, yEndPortalPosition, xEnd, yEnd);
			}
			else
				costEstimate += estimateCost(xStartPortalPosition, yStartPortalPosition, xEnd, yEnd);
		}
		else if(endId >= 0)
		{
			costEstimate = estimateCost(xStart, yStart, xEndPortalPosition, yEndPortalPosition);
			costEstimate += estimateCost(xEndPortalPosition, yEndPortalPosition, xEnd, yEnd);
		}
		else
			costEstimate = estimateCost(xStart, yStart, xEnd, yEnd);

		costEstimate /= heuristicWeight;
		if(startId >= 0 || endId >= 0)
		{
			if(costEstimate > AI_MAX_COST_PORTAL_ESTIMATE)
			{
				Logger::getInstance()->warning("Too long path specified through portals - discarding");
				Logger::getInstance()->warning(int2str(int(costEstimate / 8.f)));

				/*
			float f = estimateCost(xStart, yStart, xEnd, yEnd) / heuristicWeight / 8.f;
			Logger::getInstance()->warning(int2str(int(f)));
			f = estimateCost(xStart, yStart, xStartPortalPosition, yStartPortalPosition) / heuristicWeight / 8.f;
			Logger::getInstance()->warning(int2str(int(f)));
			f = estimateCost(xStartPortalPosition, yStartPortalPosition, xEnd, yEnd) / heuristicWeight / 8.f;
			Logger::getInstance()->warning(int2str(int(f)));
				*/

				pathfind_findroutes_failed_since_last_clear++;
				return false;
			}
		}
		else
		{
			if(costEstimate > AI_MAX_COST_ESTIMATE)
			{
				Logger::getInstance()->warning("Too long path specified - discarding");
				Logger::getInstance()->warning(int2str(int(costEstimate / 8.f)));
				pathfind_findroutes_failed_since_last_clear++;
				return false;
			}
		}
	}

	// Ok, should clean this logic up. Sometime ;-)

	bool a = (startId >= 0) && (xStartPortalPosition == -1);
	bool b = (endId >= 0) && (xEndPortalPosition == -1);
	if(a || b)
	{
		//assert(!"Whoops");
		Logger::getInstance()->warning("Pathfind::findRoute. Unstable - no doors set?");
		return findActualRoute(resultPath, xStart, yStart, xEnd, yEnd, maxHeightDifference, climbPenalty, 0);
	}

	// Find path back to front
	bool result = false;
	if((startId >= 0) && (endId >= 0))
	{
		/*
		result = findActualRoute(resultPath, xStart, yStart, xStartPortalPosition, yStartPortalPosition, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;

		result = findActualRoute(resultPath, xStartPortalPosition, yStartPortalPosition, xEndPortalPosition, yEndPortalPosition, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;

		result = findActualRoute(resultPath, xEndPortalPosition, yEndPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;
		*/

		result = findActualRoute(resultPath, xEndPortalPosition, yEndPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty, 0);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}

		result = findActualRoute(resultPath, xStartPortalPosition, yStartPortalPosition, xEndPortalPosition, yEndPortalPosition, maxHeightDifference, climbPenalty, &blockMap[startId]);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}

		result = findActualRoute(resultPath, xStart, yStart, xStartPortalPosition, yStartPortalPosition, maxHeightDifference, climbPenalty, 0);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}
	}
	else if(startId >= 0)
	{
		/*
		result = findActualRoute(resultPath, xStart, yStart, xStartPortalPosition, yStartPortalPosition, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;

		result = findActualRoute(resultPath, xStartPortalPosition, yStartPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;
		*/

		result = findActualRoute(resultPath, xStartPortalPosition, yStartPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty, &blockMap[startId]);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}

		result = findActualRoute(resultPath, xStart, yStart, xStartPortalPosition, yStartPortalPosition, maxHeightDifference, climbPenalty, 0);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}
	}
	else // endId >= 0
	{
		/*
		result = findActualRoute(resultPath, xStart, yStart, xEndPortalPosition, yEndPortalPosition, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;

		result = findActualRoute(resultPath, xEndPortalPosition, yEndPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty);
		if(result == false)
			return false;
		*/

		result = findActualRoute(resultPath, xEndPortalPosition, yEndPortalPosition, xEnd, yEnd, maxHeightDifference, climbPenalty, 0);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}
		
		result = findActualRoute(resultPath, xStart, yStart, xEndPortalPosition, yEndPortalPosition, maxHeightDifference, climbPenalty, 0);
		if(result == false)
		{
			pathfind_findroutes_failed_since_last_clear++;
			return false;
		}
	}
	
	return true;
}

bool PathFind::isBlocked(int xPosition, int yPosition) const
{
	int obstacleCount = obstacleMap[xPosition][yPosition];
	assert(obstacleCount >= 0);

	if(obstacleCount == 0)
		return false;
	else
		return true;
}

int PathFind::getBlockingCount(int xPosition, int yPosition) const
{
	int obstacleCount = obstacleMap[xPosition][yPosition];

	assert(obstacleCount >= 0);
	return obstacleCount;
}

void PathFind::setBlockingCount(int xPosition, int yPosition, int amount)
{
	assert(amount >= 0 && amount <= 127);
	obstacleMap[xPosition][yPosition] = amount;
}


// Returns model (from pathblocks) at given point. 0 if not found
IStorm3D_Model *PathFind::getModelAt(int xPosition, int yPosition, const VC3 &worldCoords) const
{
	int index = blockAt(xPosition, yPosition, worldCoords);
	if(index >= 0)
		return blockMap[index].getModel();
	else
		return 0;
}

void PathFind::setPathfindDepthByAbsoluteValue(int depth)
{
	//assert(depth > 0 && depth <= AI_PATHFIND_MAX_DEPTH);
	pathfindDepth = depth;
}

void PathFind::setPathfindDepthByPercentage(int depth)
{
	assert(depth > 0 && depth <= 100);
	pathfindDepth = (AI_PATHFIND_MAX_DEPTH * depth) / 100;
}

bool PathFind::findActualRoute(Path *resultPath, int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference, float climbPenalty, const Pathblock *avoidBlock) const
{
	//PSDHAX
	//return false;

	// Use custom version since stl doesn't support clearing pqueue
	static PriorityQueue openList;
	openList.clear();

	// Don't reallocate each time.
	static ClosedHash closedList;
	closedList.clear();

	// Reset nodes
	resetNodes();

// Clamp to even positions
xStart += xStart % 2;
yStart += yStart % 2;
xEnd += xEnd % 2;
yEnd += yEnd % 2;

	if(xStart < 0 || xStart >= xSize || yStart < 0 || yStart >= ySize || xEnd < 0 || xEnd >= xSize || yEnd < 0 || yEnd >= ySize)
	{
		Logger::getInstance()->error("findActualRoute - Invalid endpoints specified.");
		return false;
	}

	// Just in case
	assert((xStart >= 0) && (xStart < xSize));
	assert((yStart >= 0) && (yStart < ySize));
	assert((xEnd >= 0) && (xEnd < xSize));
	assert((yEnd >= 0) && (yEnd < ySize));

	assert(resultPath);

	// Scale climb penalty to our map
	climbPenalty /= 65535.f;

	// Here´s all the magic. Begin with only start position on open
	int startNode = getNewNode();
	nodes[startNode].xIndex = xStart;
	nodes[startNode].yIndex = yStart;
	nodes[startNode].totalCost = estimateCost(xStart, yStart, xEnd, yEnd);

	openList.push(startNode, nodes[startNode].yIndex * xSize + nodes[startNode].xIndex, xSize);
	assert(openList.top(xSize) == startNode);

	// Goal
	Node endNode;
	endNode.xIndex = xEnd;
	endNode.yIndex = yEnd;

	int xMid = (xStart + xEnd) / 2;
	int yMid = (yStart + yEnd) / 2;

	float ambientLightCost = 0.0f;

	if (lightAvoidAmount > 0)
	{
		assert(lightMap != NULL);
		int lAmount1 = lightMap->getLightAmount(xStart, yStart);
		int lAmount2 = lightMap->getLightAmount(xEnd, yEnd);
		int lAmount3 = lightMap->getLightAmount(xMid, yMid);
		int lAmount = (lAmount1 + lAmount2 + lAmount3) / 3;
		if (lAmount > lightAvoidAmount)
		{
			// 0 - 255 -> 0 - 25f
			ambientLightCost = 0.1f * float(lAmount - lightAvoidAmount);
			ambientLightCost = powf(ambientLightCost, 1.5f);
		}
	}

	// cannot let it check all possibilities, takes too much time.
	// computer jams if there is no route.
	// so just added a failure counter...
	// -jpk
	int failureCount = 0;

	while(openList.empty() == false)
	{
		// Get the most promising node
		int currentNode = openList.top(xSize);
		openList.pop(xSize);
		
		assert((currentNode >= 0) && (currentNode < static_cast<int> (nodes.size())));
		int currentIndex = nodes[currentNode].yIndex * xSize + nodes[currentNode].xIndex;

		/*
		// New version. Test whether this node is on closed
		int closedIndex = closedList.find(currentNode);
		if(closedIndex >= 0)
		{
			// If closed is better, ignore
			if(nodes[closedIndex] < nodes[currentNode])
				continue;
			else
				nodes[closedIndex] = nodes[currentNode];
		}
		else
			closedList.insert(currentIndex, currentNode);
		*/

		assert(closedList.find(currentIndex) < 0);
		assert(openList.find(currentIndex) < 0);
		closedList.insert(currentIndex, currentNode);

		// Failure counter
		++failureCount;
		if(failureCount > AI_PATHFIND_MAX_DEPTH
			|| failureCount > pathfindDepth)
			return false;

		// Have we found the path?
		if(nodes[currentNode] == endNode)
		{
			// Construct path backwards to resultPath
			int loopIndex = currentIndex;		

			while(loopIndex >= 0)
			{
				int closedIndex = closedList.find(loopIndex);
				assert(closedIndex >= 0);
				
				Node &n = nodes[closedIndex];

				// Add waypoint
				resultPath->addPoint(n.xIndex, n.yIndex);

				// Update to parent
				loopIndex = n.yParentIndex * xSize + n.xParentIndex;
			}

			return true;
		}

#ifndef NDEBUG
		{
			// Make sure current's parent is here too
			Node &n = nodes[currentNode];
			int parentIndex = n.yParentIndex * xSize + n.xParentIndex;
			if(parentIndex > 0)
				assert(closedList.find(parentIndex) >= 0);
		}
#endif	

		int parentIndex = nodes[currentNode].yParentIndex * xSize + nodes[currentNode].xParentIndex;

		// Loop all neighbours
		//for(int i = nodes[currentNode].xIndex - 1; i <= nodes[currentNode].xIndex + 1; ++i)
		//for(int j = nodes[currentNode].yIndex - 1; j <= nodes[currentNode].yIndex + 1; ++j)
for(int j = nodes[currentNode].yIndex - 2; j <= nodes[currentNode].yIndex + 2; j += 2)
for(int i = nodes[currentNode].xIndex - 2; i <= nodes[currentNode].xIndex + 2; i += 2)
		{
			// Test bounds
			if((i < 0) || (i >= xSize))
				continue;
			if((j < 0) || (j >= ySize))
				continue;

			int loopIndex = j * xSize + i;
	
			// Ignore self
			if(loopIndex == currentIndex)
				continue;

			// Ignore parent
			if(loopIndex == parentIndex)
				continue;

			// Is the way blocked
			if(isBlocked(i, j) == true)
				continue;

			// Too great height difference?
			if(isMovable(nodes[currentNode].xIndex, nodes[currentNode].yIndex, i, j, maxHeightDifference) == false)
				continue;

			// Create node
			int loopNode = getNewNode();
			assert((loopNode >= 0) && (loopNode < static_cast<int> (nodes.size())));

			nodes[loopNode].xIndex = i;
			nodes[loopNode].yIndex = j;
			nodes[loopNode].xParentIndex = nodes[currentNode].xIndex;
			nodes[loopNode].yParentIndex = nodes[currentNode].yIndex;

			// Costs
			nodes[loopNode].realCost = nodes[currentNode].realCost;
			nodes[loopNode].realCost += realCost(nodes[currentNode].xIndex, nodes[currentNode].yIndex, i, j, climbPenalty);
			nodes[loopNode].totalCost = nodes[loopNode].realCost;
			nodes[loopNode].totalCost += estimateCost(i, j, xEnd, yEnd) + ambientLightCost;

			// Is it too close to cover
			if (coverAvoidDistance > 0)
			{
				assert(coverMap != NULL);
				if (coverMap->getDistanceToNearestCover(i, j) 
					< coverAvoidDistance)
				{
					int coverCost = coverAvoidDistance - coverMap->getDistanceToNearestCover(i, j);
					//int coverCost = 5;
					nodes[loopNode].realCost += coverCost;
					nodes[loopNode].totalCost += coverCost;
				}
			}

			// Light avoid?
			if (lightAvoidAmount > 0)
			{
				assert(lightMap != NULL);
				int lAmount = lightMap->getLightAmount(i, j);
				assert(lightMap != NULL);
				if (lAmount > lightAvoidAmount)
				{
					// 0 - 255 -> 0 - 25f
					float lightCost = 0.1f * float(lAmount - lightAvoidAmount);
					lightCost = powf(lightCost, 1.5f);
					nodes[loopNode].realCost += lightCost;
					nodes[loopNode].totalCost += lightCost;
				}
			}

			// If this area is to be avoided, pump up the cost ;-)
			// NEW: ignore this, raytrace used instead of the old block map...
			// TODO: some new implementation (if such required)
			/*
			if(avoidBlock)
			{
				if(blockedAt(i, j, *avoidBlock) == 1)
				{
					nodes[loopNode].realCost += 100000.f;
					nodes[loopNode].totalCost += 100000.f;
				}
			}
			*/

			int closedIndex = closedList.find(loopIndex);
			assert(closedIndex < usedNodes);
			//assert(openList.find(loopIndex) < 0);

			if(closedIndex >= 0)
			{
				Node &n = nodes[closedIndex];
				assert(openList.find(loopIndex) < 0);
				
				// If found shorter path, update to open list
				if(nodes[loopNode].totalCost < n.totalCost)
				{
					closedList.erase(loopIndex);
					openList.push(loopNode, loopIndex, xSize);
				}

				// No need to push open
				continue;
			}

			int openIndex = openList.find(loopIndex);
			assert(openIndex < usedNodes);

			if(openIndex >= 0)
			{
				Node &n = nodes[openIndex];
				assert(closedList.find(loopIndex) < 0);

				// If found shorter path, update
				if(nodes[loopNode].totalCost < n.totalCost)
				{
					//openList.erase(loopIndex, xSize);
					//openList.push(loopNode, loopIndex, xSize);
				}
			}
			else
			{
				openList.push(loopNode, loopIndex, xSize);
#ifdef PATHFIND_DEBUG
				if (pathfind_debug_data != NULL)
				{
					pathfind_debug_data[i + j * xSize] = 1;
				}
#endif
			}
		}
	}

	return false;
}

int PathFind::getHeight(int xPosition, int yPosition) const
{
	// Changed divs to shift
	return heightMap[(xPosition >> accuracyShift) + (yPosition >> accuracyShift) * xSizeHeightmap];
}

bool PathFind::isMovable(int xStart, int yStart, int xEnd, int yEnd, int maxHeightDifference) const
{
	// Check for blocked sides (thanx jpk ;-)
	int xDelta = 0;
	int yDelta = 0;

	if((xDelta = xEnd - xStart) != 0
	  && (yDelta = yEnd - yStart) != 0)
	{
		if(isBlocked(xStart + xDelta, yStart))
			return false;
		if(isBlocked(xStart, yStart + yDelta))
			return false;
	}

	// Is it too close to cover
	if (coverBlockDistance > 0)
	{
		assert(coverMap != NULL);
		if (coverMap->getDistanceToNearestCover(xEnd, yEnd) 
			< coverBlockDistance)
		{
			return false;
		}
	}

	// Calculate height delta
	uint16 h1 = getHeight(xStart, yStart);
	uint16 h2 = getHeight(xEnd, yEnd);

	int delta = h2 - h1; 
	if(delta < 0)
		delta = -delta;

	if(delta < maxHeightDifference)
		return true;

	return false;
}

float PathFind::estimateCost(int xStart, int yStart, int xEnd, int yEnd) const
{
	int xDelta = xStart - xEnd;
	int yDelta = yStart - yEnd;

	// Distance * heuristicWeight
	float result = static_cast<float> (xDelta*xDelta + yDelta*yDelta);
	result = sqrtf(result);

	return result * heuristicWeight;
}

float PathFind::realCost(int xStart, int yStart, int xEnd, int yEnd, float climbPenalty) const
{
	int xDelta = xStart - xEnd;
	int yDelta = yStart - yEnd;

	/*
	int delta = abs(xDelta) + abs(yDelta);
	if(delta == 0)
		return 0.f;
	else if(delta == 1)
		return 1.f;
	else
		return 1.41f;
	*/

	// Distance

	float result = static_cast<float> (xDelta*xDelta + yDelta*yDelta);
	result = sqrtf(result);

	// Height delta
	/*
	uint16 h1 = getHeight(xStart, yStart);
	uint16 h2 = getHeight(xEnd, yEnd);

	int heightDelta = h2 - h1;
	
	// Uphill
	if(heightDelta > 0)
		result *= climbPenalty; //result += climbPenalty * heightDelta;
	*/
	
	return result;
}

int PathFind::blockAt(int xPosition_, int yPosition_, const VC3 &worldCoords) const
{
	// Maybe use quadtree or other simpled hierarchy if this becomes too slow ..
	for(unsigned int i = 0; i < blockMap.size(); ++i)
	{
		int xPosition = blockMap[i].getPositionX();
		int yPosition = blockMap[i].getPositionY();
		int xSize = blockMap[i].getSizeX();
		int ySize = blockMap[i].getSizeY();

		// Inside this block?
		if((xPosition_ < xPosition) || (xPosition_ >= xPosition + xSize))
			continue;
		if((yPosition_ < yPosition) || (yPosition_ >= yPosition + ySize))
			continue;

		// NEW: ignore this, raytrace used instead of the old block map...
		/*
		const std::vector<std::vector<unsigned char> > &blocks = blockMap[i].getBlocks();

		// Actual pixel belongs to it?
		int xBlock = xPosition_ - xPosition;
		int yBlock = yPosition_ - yPosition;

		if(blocks[xBlock][yBlock] == 1)
			return i;
		*/
		if (blockMap[i].getModel() != NULL)
		{
			Storm3D_CollisionInfo cinfo;
			VC3 dir(0,-1,0);
			VC3 pos = worldCoords;
			pos.y = 50.0f;
			blockMap[i].getModel()->RayTrace(pos, dir, 100.0f, cinfo, true);
			if (cinfo.hit)
				return i;
		}

	}

	return -1;
}

int PathFind::blockedAt(int xPosition_, int yPosition_, const Pathblock &block) const
{
	// NEW: ignore this, raytrace used instead of the old block map...
	// TODO: some new implementation (if such required)
	/*
	int xPosition = block.getPositionX();
	int yPosition = block.getPositionY();
	int xSize = block.getSizeX();
	int ySize = block.getSizeY();

	// Inside this block?
	if((xPosition_ < xPosition) || (xPosition_ >= xPosition + xSize))
		return 0;
	if((yPosition_ < yPosition) || (yPosition_ >= yPosition + ySize))
		return 0;

	const std::vector<std::vector<unsigned char> > &blocks = block.getBlocks();

	// Actual pixel belongs to it?
	int xBlock = xPosition_ - xPosition;
	int yBlock = yPosition_ - yPosition;

	if(blocks[xBlock][yBlock] == 1)
		return 1;
	*/

	return 0;
}

void PathFind::setCoverMap(game::CoverMap *coverMap)
{
  this->coverMap = coverMap;
}

void PathFind::setCoverAvoidDistance(int coverAvoidDistance)
{
  this->coverAvoidDistance = coverAvoidDistance;
}

void PathFind::setCoverBlockDistance(int coverBlockDistance)
{
  this->coverBlockDistance = coverBlockDistance;
}

void PathFind::setLightMap(util::LightMap *lightMap)
{
  this->lightMap = lightMap;
}

void PathFind::setLightAvoidAmount(int lightAvoidAmount)
{
  this->lightAvoidAmount = lightAvoidAmount;
}


} // end of namespace ai
} // end of namespace frozenbyte
