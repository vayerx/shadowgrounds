
#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <assert.h>
#include "GameObject.h"
#include "../ui/Terrain.h"
#include "../util/AreaMap.h"
#include "areamasks.h"

//#include <Storm3D_ObstacleMapDefs.h>

#include <vector>

// must equal to OBSTACLEMAP_SIZE_MULT
//#define GAMEMAP_PATHFIND_ACCURACY 4
#define GAMEMAP_PATHFIND_ACCURACY 2

// heightmap multiplier (collision heightmap : original render heightmap)
#ifdef PROJECT_SURVIVOR
#define GAMEMAP_HEIGHTMAP_MULTIPLIER 2
#else
#define GAMEMAP_HEIGHTMAP_MULTIPLIER 4
#endif


class IStorm3D_Terrain;

namespace frozenbyte
{
  namespace ai
  {
    class PathFind;
  }
}

namespace util 
{
	class ColorMap;
	class LightMap;
	class AreaMap;
}

namespace game
{
	class CoverMap;
//	class HideMap;


  class GameMapLoadException  // public std::exception 
  {
  private:
    char *msg;

  public:
    GameMapLoadException(char *errmsg) 
    { 
      if (errmsg != NULL)
      {
        msg = new char[strlen(errmsg) + 1];
        strcpy(msg, errmsg);
      } else {
        msg = NULL;
      }
    }

    ~GameMapLoadException() 
    {
      if (msg != NULL)
        delete [] msg;
    }
  };



  class GameMap : public GameObject
  {
  public:
    GameMap();
    ~GameMap();

    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    void setData(WORD *heightMap, WORD *doubledMap, VC2I size, VC2 scaledSize, float scaledHeight,
			const char *vegeFilename);

		// should call this after obstacle map is complete.
		// (after adding buildings and terrain objects)
		void createCoverMap();

    void setTerrain(IStorm3D_Terrain *terrain);

    //void loadMap(char *filename) throw (GameMapLoadException *);

    int getHeightmapHeightAt(int x, int y);
    
		// not a very nice method... does not affect rendered heightmap,
		// only the collision heightmap. =/
		void setHeightmapHeightAt(int x, int y, int value);

    float getScaledHeightAt(float scaledX, float scaledY);
    
    // get a fixed point int height 
    // (real values shifted left by 8 = 24bits.8bits fixed point value)
    // works like getScaledHeightAt, but given extra accuracy
    // returns a fixed point value also (shift right by 8 to get int value)
    //int getFixedPointHeightAt(int fixedX, int fixedY);

    //int getHeightmapSizeX();
    //int getHeightmapSizeY();
    int getHeightmapMaxHeight();

    float getScaledSizeX();
    float getScaledSizeY();
    float getScaledMaxHeight();

    float getScaleX();
    float getScaleY();

	 inline int getSizeX() const { return sizeX; };
	 inline int getSizeY() const { return sizeY; };

    // returns scale height
    float getScaleHeight();
    
    WORD *getObstacleHeightMap();

    void applyObstacleHeightChanges();

//		void loadHideMap();
//		bool isHideMapLoaded();
//		void saveHideMap();

		void loadObstacleAndAreaMap(frozenbyte::ai::PathFind *pathfinder);
		bool isObstacleAndAreaMapLoaded();
		void saveObstacleAndAreaMap(frozenbyte::ai::PathFind *pathfinder);

    // for scaled things...
    //int getScaledHeightAt(int x, int y);

  // TODO: THIS IS A TEMP HACK, REMOVE COMMENTS
  private:
    int sizeX;
    int sizeY;
    int sizeHeight;
    int pathfindSizeX;
    int pathfindSizeY;
    float scaleX;
    float scaleY;
    float scaleHeight; 
    float scaledSizeX;
    float scaledSizeY;
    float scaledSizeHeight;
    
    float scaledSizeHalvedX;
    float scaledSizeHalvedY;
    int sizeHalvedX;
    int sizeHalvedY;
    int pathfindSizeHalvedX;
    int pathfindSizeHalvedY;

    float scaledMinX;
    float scaledMinY;
    float scaledMaxX;
    float scaledMaxY;
    float scaledWellMinX;
    float scaledWellMinY;
    float scaledWellMaxX;
    float scaledWellMaxY;

    // these two are scaleX and scaleY ;)
    //float heightmapToScaledMultiplierX;
    //float heightmapToScaledMultiplierY;
    float scaledToHeightmapMultiplierX;
    float scaledToHeightmapMultiplierY;

    float pathfindToScaledMultiplierX;
    float pathfindToScaledMultiplierY;
    float scaledToPathfindMultiplierX;
    float scaledToPathfindMultiplierY;

// for efficiency
public:
    WORD *heightMap;
    WORD *pathfindHeightMap;
private:

		// to temporarily hold the pathfind heightmap loaded from binary
		// (as we cannot apply that immediately or building/terr.obj. heights would be incorrect)
		WORD *precalcedPathfindHeightMap;

    IStorm3D_Terrain *terrain;

// for efficiency
public:
    WORD *obstacleHeightMap;
private:

		CoverMap *coverMap;
		char *coverFilename;

//		HideMap *hideMap;
//		bool hideMapLoaded;

		bool obstacleAndAreaMapLoaded;

		util::AreaMap *areaMap;

public:
		util::ColorMap *colorMap;
		util::LightMap *lightMap;

		bool loadObstacleAndAreaImpl(const char *filename, frozenbyte::ai::PathFind *pathfinder);
		bool saveObstacleAndAreaImpl(const char *filename, frozenbyte::ai::PathFind *pathfinder);

  public:
		inline CoverMap *getCoverMap() 
		{
			return coverMap;
		}

		/*
		inline HideMap *getHideMap() 
		{
			return hideMap;
		}
		*/

		inline util::AreaMap *getAreaMap() 
		{
			return areaMap;
		}

	 inline IStorm3D_Terrain * getTerrain ()
	 {
		 return terrain;
	 }

    inline float configToScaledX(int x) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(x - sizeHalvedX) + 0.5f) * scaleX;
    }

    inline float configToScaledY(int y) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      // notice: y coordinate mirrored!
      return ((float)(- y + sizeHalvedY) + 0.5f) * scaleY;
    }

    inline float heightmapToScaledX(int x) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(x - sizeHalvedX * GAMEMAP_HEIGHTMAP_MULTIPLIER) + 0.5f) * (scaleX * 0.5f);
    }

    inline float heightmapToScaledY(int y) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(y - sizeHalvedY * GAMEMAP_HEIGHTMAP_MULTIPLIER) + 0.5f) * (scaleY * 0.5f);
    }

    inline int scaledToConfigX(float scaledX) const
    {
      return (int)((scaledX + scaledSizeHalvedX) * scaledToHeightmapMultiplierX);
    }

    inline int scaledToConfigY(float scaledY) const
    {
      return sizeY - (int)((scaledY + scaledSizeHalvedY) * scaledToHeightmapMultiplierY);
    }

    inline int scaledToHeightmapX(float scaledX) const
    {
      return (int)((scaledX + scaledSizeHalvedX) * scaledToHeightmapMultiplierX * GAMEMAP_HEIGHTMAP_MULTIPLIER);
    }

    inline int scaledToHeightmapY(float scaledY) const
    {
      return (int)((scaledY + scaledSizeHalvedY) * scaledToHeightmapMultiplierY * GAMEMAP_HEIGHTMAP_MULTIPLIER);
    }

    inline float pathfindToScaledX(int x) const
    {
			// TEMP
			//assert(x >= 0 && x < pathfindSizeX);
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(x - pathfindSizeHalvedX) + 0.5f) * pathfindToScaledMultiplierX;
    }

    inline float pathfindToScaledY(int y) const
    {
			// TEMP
			//assert(y >= 0 && y < pathfindSizeY);
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(y - pathfindSizeHalvedY) + 0.5f) * pathfindToScaledMultiplierY;
    }

    inline float obstacleToScaledX(int x) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(x - pathfindSizeHalvedX) + 0.5f) * pathfindToScaledMultiplierX;
    }

    inline float obstacleToScaledY(int y) const
    {
      // notice: adds 0.5f to get to the center of the block, not corner!
      return ((float)(y - pathfindSizeHalvedY) + 0.5f) * pathfindToScaledMultiplierY;
    }

    inline int scaledToPathfindX(float scaledX) const
    {
      return (int)((scaledX + scaledSizeHalvedX) * scaledToPathfindMultiplierX);
    }

    inline int scaledToPathfindY(float scaledY) const
    {
      return (int)((scaledY + scaledSizeHalvedY) * scaledToPathfindMultiplierY);
    }

    inline int scaledToObstacleX(float scaledX) const
    {
      return (int)((scaledX + scaledSizeHalvedX) * scaledToPathfindMultiplierX);
    }

    inline int scaledToObstacleY(float scaledY) const
    {
      return (int)((scaledY + scaledSizeHalvedY) * scaledToPathfindMultiplierY);
    }

		inline float getPositionOffsetFactorInsideObstacleX(float scaledX)
		{
			int obstX = scaledToObstacleX(scaledX);
			return ((scaledX + scaledSizeHalvedX) * scaledToPathfindMultiplierX) - (float)obstX;
		}

		inline float getPositionOffsetFactorInsideObstacleY(float scaledY)
		{
			int obstY = scaledToObstacleY(scaledY);
			return ((scaledY + scaledSizeHalvedY) * scaledToPathfindMultiplierY) - (float)obstY;
		}

    inline int getConfigSizeX() const
    {
      return sizeX;
    }

    inline int getConfigSizeY() const
    {
      return sizeY;
    }

    inline int getHeightmapSizeX() const
    {
      return sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
    }

    inline int getHeightmapSizeY() const
    {
      return sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;
    }

    inline int getPathfindSizeX() const
    {
      return pathfindSizeX;
    }

    inline int getPathfindSizeY() const
    {
      return pathfindSizeY;
    }

    inline int getObstacleSizeX() const
    {
      return pathfindSizeX;
    }

    inline int getObstacleSizeY() const
    {
      return pathfindSizeY;
    }

    inline bool isInScaledBoundaries(float scaledX, float scaledY) const
    {
      // notice: excluding the exact boundary points
      if (scaledX > scaledMinX && scaledX < scaledMaxX
        && scaledY > scaledMinY && scaledY < scaledMaxY)
        return true;
      else 
        return false;
    }

    inline bool isWellInScaledBoundaries(float scaledX, float scaledY) const
    {
      // notice: excluding the exact boundary points
      if (scaledX > scaledWellMinX && scaledX < scaledWellMaxX
        && scaledY > scaledWellMinY && scaledY < scaledWellMaxY)
        return true;
      else 
        return false;
    }

    inline bool inHeightmapBoundaries(int x, int y) const
    {
      if (x >= 0 && x < sizeX
        && y >= 0 && y < sizeY)
        return true;
      else 
        return false; 
    }

    inline bool inPathfindBoundaries(int x, int y) const
    {
      if (x >= 0 && x < pathfindSizeX
        && y >= 0 && y < pathfindSizeY)
        return true;
      else 
        return false; 
    }

    inline void keepWellInScaledBoundaries(float *scaledX, float *scaledY) const
    {
      if (*scaledX < scaledWellMinX) *scaledX = scaledWellMinX;
      if (*scaledX > scaledWellMaxX) *scaledX = scaledWellMaxX;
      if (*scaledY < scaledWellMinY) *scaledY = scaledWellMinY;
      if (*scaledY > scaledWellMaxY) *scaledY = scaledWellMaxY;
    }

    /*
    void addObstacleHeight(int x, int y, int height);
    void removeObstacleHeight(int x, int y, int height);
    void addMovingObstacleHeight(int x, int y, int height);
    void removeMovingObstacleHeight(int x, int y, int height);
    */
  
    inline void addObstacleHeight(int x, int y, int height, AREAMAP_DATATYPE obstacleMask)
    {
			assert((obstacleMask & AREAMASK_OBSTACLE_MOVABLE) == 0);

			//assert(obstacleHeightMap[x + y * pathfindSizeX] < OBSTACLE_MAP_MAX_HEIGHT - height);
      obstacleHeightMap[x + y * pathfindSizeX] += height;
			areaMap->setAreaValue(x, y, AREAMASK_OBSTACLE_ALL, obstacleMask);

			// TODO: clear other flags??? (seethrough, unhittable, etc.)
    }
  
    inline void removeObstacleHeight(int x, int y, int height, AREAMAP_DATATYPE obstacleMask)
    {
			assert((obstacleMask & AREAMASK_OBSTACLE_MOVABLE) == 0);

      //assert(obstacleHeightMap[x + y * pathfindSizeX] >= height);
		if (!areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_MOVABLE))
		{
		  if(obstacleHeightMap[x + y * pathfindSizeX] >= height)
			  obstacleHeightMap[x + y * pathfindSizeX] -= height;
		  else
			  obstacleHeightMap[x + y * pathfindSizeX] = 0;
		}
    } 
  
    inline bool isMovingObstacle(int x, int y) const
    {
			return areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_MOVABLE);
			/*
      if ((obstacleHeightMap[x + y * pathfindSizeX] & (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE)) 
				== (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE))
      {
				return true;
			} else {
				return false;
			}
			*/
		}

    inline bool isRoundedObstacle(int x, int y) const
    {
			return areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_ROUNDED);
		}

    inline bool isUnHittableObstacle(int x, int y) const
    {
			return areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_UNHITTABLE);
		}

		// TODO: isSeeThroughObstacle

    inline void addMovingObstacleHeight(int x, int y, int height, AREAMAP_DATATYPE obstacleMask)
    {
      // mark as moving only if no other obstacles here.
      //if ((obstacleHeightMap[x + y * pathfindSizeX] & OBSTACLE_MAP_MASK_HEIGHT) == 0)
			// New behaviour: 
			// mark as moving only if no other obstacles here or marked
			// as moving earlier.
			/*
      if ((obstacleHeightMap[x + y * pathfindSizeX] & OBSTACLE_MAP_MASK_HEIGHT) == 0
        || (obstacleHeightMap[x + y * pathfindSizeX] & (OBSTACLE_MAP_MASK_UNHITTABLE | OBSTACLE_MAP_MASK_SEETHROUGH)) == (OBSTACLE_MAP_MASK_UNHITTABLE | OBSTACLE_MAP_MASK_SEETHROUGH))
      {
        obstacleHeightMap[x + y * pathfindSizeX] |= 
					(OBSTACLE_MAP_MASK_UNHITTABLE | OBSTACLE_MAP_MASK_SEETHROUGH);
				// New behaviour: 
				// and add height only if moving obstacles here
        obstacleHeightMap[x + y * pathfindSizeX] += height;
      }
			*/

			assert((obstacleMask & AREAMASK_OBSTACLE_MOVABLE) != 0);

			if (obstacleHeightMap[x + y * pathfindSizeX] == 0
				|| areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_MOVABLE))
			{
				areaMap->setAreaValue(x, y, AREAMASK_OBSTACLE_ALL, obstacleMask);
				//areaMap->setAreaValue(x, y, AREAMASK_OBSTACLE_MOVABLE, AREAVALUE_OBSTACLE_MOVABLE_YES);
	      obstacleHeightMap[x + y * pathfindSizeX] += height;
			}
    }
  
    inline void removeMovingObstacleHeight(int x, int y, int height, AREAMAP_DATATYPE obstacleMask)
    {
      // only if a moving obstacle (no other obstacles..)
			/*
      if ((obstacleHeightMap[x + y * pathfindSizeX] & (OBSTACLE_MAP_MASK_UNHITTABLE | OBSTACLE_MAP_MASK_SEETHROUGH)) == (OBSTACLE_MAP_MASK_UNHITTABLE | OBSTACLE_MAP_MASK_SEETHROUGH))
      {
        assert((obstacleHeightMap[x + y * pathfindSizeX] & OBSTACLE_MAP_MASK_HEIGHT) >= height);
	      obstacleHeightMap[x + y * pathfindSizeX] -= height;
			}
			*/
			assert((obstacleMask & AREAMASK_OBSTACLE_MOVABLE) != 0);

			if (areaMap->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_MOVABLE))
			{
//        assert(obstacleHeightMap[x + y * pathfindSizeX] >= height);
	      obstacleHeightMap[x + y * pathfindSizeX] -= height;
			}
    }

		/*
    inline void addBuildingObstacleHeight(int x, int y, int height)
    {
      obstacleHeightMap[x + y * pathfindSizeX] += height;
			// removed the unhittable...
			// to get LOS trace behave exactly as projectile raytrace
      //obstacleHeightMap[x + y * pathfindSizeX] |= OBSTACLE_MAP_MASK_UNHITTABLE;
    }
		*/
  
    inline int getObstacleHeight(int x, int y) const
    {
			//assert(x >= 0 && y >= 0 && x < pathfindSizeX && y < pathfindSizeY);
      //return (obstacleHeightMap[x + y * pathfindSizeX] & OBSTACLE_MAP_MASK_HEIGHT);
	    return obstacleHeightMap[x + y * pathfindSizeX];
    }

		void makeHeightAreaBlocked(int heightMapX, int heightMapY);

  };

}

#endif
