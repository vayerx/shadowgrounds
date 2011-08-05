
#ifndef GAMESCENE_H
#define GAMESCENE_H


#include "GameCollisionInfo.h"
#include "../ui/Terrain.h"

// log some stats... (will be logged at logger's debug level)
// define this in the makefile instead if you want it.
//#define DUMP_GAMESCENE_STATS

class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;

namespace frozenbyte
{
  namespace ai
  {
    class PathFind;
  }
}

namespace game
{
  class GameMap;
  class Building;

  class GameScene
  {
  public:
    GameScene(IStorm3D *storm3D, IStorm3D_Scene *stormScene,
      Terrain *terrain, GameMap *gameMap);

    ~GameScene();

    IStorm3D *getStorm3D() { return storm3D; }
    IStorm3D_Scene *getStormScene() { return stormScene; }

    // raytrace... (direction must be normalized)
    void rayTrace(const VC3 &origin, const VC3 &direction, float rayLength, 
      GameCollisionInfo &cinfo, bool accurate, bool loscheck, bool terrainOnly = false, bool terrainOnlyForReal = false);

    // pathfinding
    bool findPath(frozenbyte::ai::Path *path, float startX, float startY, 
      float endX, float endY, float maxHeightDifference, float climbPenalty,
			int coverAvoidDistance, int coverBlockDistance, int depth, int lightAvoidAmount);

    // adding and removing MOVING obstacles 
    void moveObstacle(int fromX, int fromY, int toX, int toY, int height);
    void addMovingObstacle(int x, int y, int height);
    void removeMovingObstacle(int x, int y, int height);

		// door obstacle add/remove
    void addDoorObstacle(int x, int y, int height);
    void removeDoorObstacle(int x, int y, int height);

    // querying obstacles
    bool isBlockedAtScaled(float x, float y, float height);
    bool isBlocked(int x, int y, float height);

		// WARNING: does not return the actual block amount over the
		// given height - instead returns amount of all blocks, if 
		// the given height is above all of them, else zero.
		// Remember this when you are calling this somewhere!
		// Don't think that this gives you blocks above the height. 
		// that would be a total screw up. ;)
    int getBlockingCount(int x, int y, float height);

		// WARNING: the real HACK HACK version of getBlockingCount..
    int getConditionalBlockingCountForUnit(int x, int y, float height);

		// fills the area map with terrain material
		void initTerrainMaterial(void);

		// material to fill area map with
		void setTerrainMaterial(const std::string &material);

    // add obstacles for given list of terrain obstacles
    void addTerrainObstacles(std::vector<TerrainObstacle> &obstacleList);

    // remove obstacles for given list of terrain obstacles
    void removeTerrainObstacles(std::vector<TerrainObstacle> &obstacleList);

    // add obstacle for building
    void addBuildingObstacle(Building *b, bool terrainCut);

    IStorm3D_Model *getBuildingModelAtPathfind(int x, int y);
    IStorm3D_Model *getBuildingModelAtScaled(float x, float y);

		inline GameMap *getGameMap() { return gameMap; }

		unsigned char *generateTerrainTexturing();

		frozenbyte::ai::PathFind *getPathFinder();

		IStorm3D_Terrain *getTerrain() { return stormTerrain; }

  private:
    IStorm3D *storm3D;
    IStorm3D_Scene *stormScene;
    IStorm3D_Terrain *stormTerrain;
    Terrain *terrain;
    GameMap *gameMap;
    frozenbyte::ai::PathFind *pathFinder;

    void modifyTerrainObstaclesImpl(std::vector<TerrainObstacle> &obstacleList, bool add);

#ifdef DUMP_GAMESCENE_STATS
    int statPathfindAmount;
    int statLOStraceAmount;
    int statRaytraceAmount;

    int statPathfindFailedAmount;

    int statLOStracePartialAmount;
    int statLOStraceToTerrainAmount;
    int statLOStraceToUnitAmount;
    int statLOStraceToObstacleAmount;

    int statRaytraceToTerrainAmount;
    int statRaytraceToUnitAmount;
    int statRaytraceToObstacleAmount;

    int statLongPathfindAmount;
    int statLongLOStraceAmount;
    int statLongRaytraceAmount;
    float statPathfindTime;
    float statLOStraceTime;
    float statRaytraceTime;

    int statStartTime;
#endif

  };

}

#endif



