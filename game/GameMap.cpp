
#include "precompiled.h"

#include <string.h>
#include <stdio.h>

#include <IStorm3D_Terrain.h>

#include "GameMap.h"
#include "DHLocaleManager.h"
#include "SimpleOptions.h"
#include "options/options_precalc.h"
#include "CoverMap.h"
//#include "HideMap.h"
#include "../ui/VisualObjectModel.h"
#include "../ui/VisualObject.h"
#include "../ui/LoadingMessage.h"
#include "../system/FileTimestampChecker.h"
#include "../system/Logger.h"
#include "../util/ColorMap.h"
#include "../util/LightMap.h"
#include "../util/AI_PathFind.h"
#include "../util/AreaMap.h"
#include "areamasks.h"
#include "../util/Floodfill.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/rle_packed_file_wrapper.h"

#include "../util/Debug_MemoryManager.h"


using namespace ui;
using namespace frozenbyte;


namespace game
{
	// to assist in "BlockedHeightArea" floodfilling, implements the 
	// required mapper interface.
	class BlockedHeightAreaFillMapper : public util::IFloodfillByteMapper
	{
	private:
		GameMap *data;
		int height;

	public:
		// in this case, 2 means "has blocked this height area"
		// 1 means "height are to be blocked"
		// 0 means "some other height (do nothing here)"
		BlockedHeightAreaFillMapper(GameMap *data, int height)
		{
			this->height = height;
			this->data = data;
		}

		virtual unsigned char getByte(int x, int y)
		{
			if (data->getHeightmapHeightAt(x,y) != height)
			{
				return 0;
			}

			int ox = x * GAMEMAP_PATHFIND_ACCURACY;
			int oy = y * GAMEMAP_PATHFIND_ACCURACY;

			for (int ty = 0; ty < GAMEMAP_PATHFIND_ACCURACY; ty++)
			{
				for (int tx = 0; tx < GAMEMAP_PATHFIND_ACCURACY; tx++)
				{
					if (data->getObstacleHeight(ox + tx, oy + ty) <= 200)
					{
						// please, do floodfill me.
						return 1;
					}
				}
			}

			// this has already been floodfilled 
			// (or just otherwise fully blocked)
			return 0;
		}

		virtual void setByte(int x, int y, unsigned char value)
		{
			int ox = x * GAMEMAP_PATHFIND_ACCURACY;
			int oy = y * GAMEMAP_PATHFIND_ACCURACY;

			int obstSizeX = data->getObstacleSizeX();
			int obstSizeY = data->getObstacleSizeY();

			for (int ty = -1; ty < GAMEMAP_PATHFIND_ACCURACY+1; ty++)
			{
				for (int tx = -1; tx < GAMEMAP_PATHFIND_ACCURACY+1; tx++)
				{
					if (ox + tx >= 0 && ox + tx < obstSizeX
						&& oy + ty >= 0 && oy + ty < obstSizeY)
					{
						// HACK: just using some strange number for the height :)
						// in this case, 200 seems to be somewhat nice...
						if (data->getObstacleHeight(ox + tx, oy + ty) <= 200)
						{
							data->addObstacleHeight(ox + tx, oy + ty, 201, AREAVALUE_OBSTACLE_TERRAINOBJECT);
							assert(data->getObstacleHeight(ox + tx, oy + ty) > 200);
						}
					}
				}
			}
		}

	};

 
  GameMap::GameMap()
  {

    // TODO: INITIALIZE THE REST OF THE VARIABLES.

    sizeX = 0;
    sizeY = 0;
    scaledSizeX = 0;
    scaledSizeY = 0;
    scaleX = 1;
    scaleY = 1;
    scaleHeight = 1;
    heightMap = NULL;
    terrain = NULL;
    obstacleHeightMap = NULL;
		coverMap = NULL;
		coverFilename = NULL;
//		hideMap = NULL;
//		hideMapLoaded = false;
		obstacleAndAreaMapLoaded = false;
		areaMap = NULL;
		colorMap = 0;
		lightMap = 0;
		precalcedPathfindHeightMap = NULL;
  } 

  GameMap::~GameMap()
  {
    // WARNING: this is a shared data buffer!
    // created by Terrain, but deleted here!!!
    // (that is, if setData is used)
    if (heightMap != NULL)
    {
      //delete [] heightMap;
    }

    if (obstacleHeightMap != NULL)
    {
      delete [] obstacleHeightMap;
    }
    if (coverMap != NULL)
    {
      delete coverMap;
    }
//    if (hideMap != NULL)
//    {
//      delete hideMap;
//    }
    if (areaMap != NULL)
    {
      delete areaMap;
    }
	
    delete lightMap;
    delete colorMap;
  }

  WORD *GameMap::getObstacleHeightMap()
  {
    return obstacleHeightMap;
  }

  void GameMap::applyObstacleHeightChanges()
  {
    terrain->recreateCollisionMap();
  }

  // a hack to get storm terrain height calculation here
  // otherwise the interpolation between height blocks will be different
  void GameMap::setTerrain(IStorm3D_Terrain *terrain)
  {
    this->terrain = terrain;
  }

  SaveData *GameMap::getSaveData() const
  {
    // TODO: return heightMap...
    return NULL;
  }

  const char *GameMap::getStatusInfo() const
  {
		return "GameMap";
	}

  void GameMap::setData(WORD *heightMap, WORD *doubledMap, VC2I size, VC2 scaledSize, float scaledHeight,
		const char *vegeFilename)
  {
    // shared buffer! (now owned by this object)
    this->heightMap = heightMap;
    
    // 2x resolution. shared. (owned by storm)
    this->pathfindHeightMap = doubledMap;

    sizeX = size.x;
    sizeY = size.y;
    sizeHeight = 65536;
    scaleX = scaledSize.x / (float)sizeX;
    scaleY = scaledSize.y / (float)sizeY;
    scaleHeight = scaledHeight / (float)65536;
    scaledSizeX = sizeX * scaleX;
    scaledSizeY = sizeY * scaleY;
    scaledSizeHeight = (sizeHeight * scaleHeight);

    pathfindSizeX = size.x * GAMEMAP_HEIGHTMAP_MULTIPLIER * GAMEMAP_PATHFIND_ACCURACY;
    pathfindSizeY = size.y * GAMEMAP_HEIGHTMAP_MULTIPLIER * GAMEMAP_PATHFIND_ACCURACY;

    scaledSizeHalvedX = scaledSizeX / 2;
    scaledSizeHalvedY = scaledSizeY / 2;
    sizeHalvedX = sizeX / 2;
    sizeHalvedY = sizeY / 2;
    pathfindSizeHalvedX = pathfindSizeX / 2;
    pathfindSizeHalvedY = pathfindSizeY / 2;

    scaledMinX = -scaledSizeHalvedX;
    scaledMinY = -scaledSizeHalvedY;
    scaledMaxX = scaledSizeHalvedX;
    scaledMaxY = scaledSizeHalvedY;
    scaledWellMinX = -(scaledSizeHalvedX - scaleX);
    scaledWellMinY = -(scaledSizeHalvedY - scaleY);
    scaledWellMaxX = scaledSizeHalvedX - scaleX;
    scaledWellMaxY = scaledSizeHalvedY - scaleY;

    scaledToHeightmapMultiplierX = 1 / scaleX;
    scaledToHeightmapMultiplierY = 1 / scaleY;

    scaledToPathfindMultiplierX = (float)pathfindSizeX / scaledSizeX;
    scaledToPathfindMultiplierY = (float)pathfindSizeY / scaledSizeY;
    pathfindToScaledMultiplierX = 1 / scaledToPathfindMultiplierX;
    pathfindToScaledMultiplierY = 1 / scaledToPathfindMultiplierY;

    if (obstacleHeightMap != NULL)
    {
      delete [] obstacleHeightMap;
      obstacleHeightMap = NULL;
    }
    obstacleHeightMap = new WORD[pathfindSizeX * pathfindSizeY];
    for (int i = 0; i < pathfindSizeX * pathfindSizeY; i++)
      obstacleHeightMap[i] = 0;

    if (coverMap != NULL)
    {
      delete coverMap;
    }
		coverMap = new CoverMap(pathfindSizeX, pathfindSizeY);

//    if (hideMap != NULL)
//    {
//      delete hideMap;
//    }
//		hideMap = new HideMap(pathfindSizeX, pathfindSizeY);

    if (areaMap != NULL)
    {
      delete areaMap;
    }
		areaMap = new util::AreaMap(pathfindSizeX, pathfindSizeY);

		if (coverFilename != NULL)
		{
			delete [] coverFilename;
			coverFilename = NULL;
		}
		if (vegeFilename != NULL)
		{
			coverFilename = new char[strlen(vegeFilename) + 1];
			strcpy(coverFilename, vegeFilename);

			if(strlen(vegeFilename) > 9)
			{
				char *colorFilename = new char[strlen(vegeFilename) + 10];
				strncpy(colorFilename, vegeFilename, strlen(vegeFilename) - 9);
				colorFilename[strlen(vegeFilename) - 9] = '\0';
				//strcat(colorFilename, "lightmap.bin");
				strcat(colorFilename, "color.bin");

				if (colorMap != NULL)
					delete colorMap;
				colorMap = new util::ColorMap(this, colorFilename, scaleX, scaleY, scaledSizeX, scaledSizeY);
				delete[] colorFilename;

				if (lightMap != NULL)
					delete lightMap;
				lightMap = new util::LightMap(colorMap, pathfindSizeX, pathfindSizeY, scaledSizeX, scaledSizeY);
			}
		}

		// the coverFilename is actually vegeFilename...
		// not sure about the logic there. ;)
		// (but can't just change that, cos we're relying on it)

		// attempt to load hidemap
//		loadHideMap();
	}


  void GameMap::createCoverMap()
  {
		// now either create the covermap and save it to disk, or load
		// directly from disk if it's up-to-date...
		char *bin_filename = new char[strlen(coverFilename) + 16];
		strcpy(bin_filename, coverFilename);
		if (strlen(coverFilename) > 10)
		{
			strcpy(&bin_filename[strlen(coverFilename) - 4-5], "cover.bin");
		} else {
			strcpy(&bin_filename[strlen(coverFilename)], "cover.bin");
			assert(0); // umm, bin filename sucked?
		}

		bool upToDate = FileTimestampChecker::isFileNewerOrSameThanFile(
			bin_filename, coverFilename);

		// if auto is off, make it up to date.
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_COVER_BIN_RECREATE))
		{
			if (!upToDate)
			{
				Logger::getInstance()->warning("GameMap::createCoverMap - Covermap is not up to date, but it will not be recreated (auto recreate off).");
				upToDate = true;
			}
		}

		// if force is on, treat as being out-of-date
		if (SimpleOptions::getBool(DH_OPT_B_FORCE_COVER_BIN_RECREATE))
		{
			Logger::getInstance()->debug("GameMap::createCoverMap - Covermap is recreated (force recreate on).");
			upToDate = false;
		}

		bool loaded = false;
		if (upToDate)
		{
			loaded = coverMap->load(bin_filename);
		}
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_COVER_BIN_RECREATE))
		{
			loaded = true; // skip recreate even if load fails!
		} else {
			if (upToDate && !loaded)
			{
				Logger::getInstance()->warning("GameMap::setData - Failed to load covermap (will attempt to recreate it).");
			}
		}
		if (!loaded)
		{
#ifndef PROJECT_SURVIVOR
			LoadingMessage::showLoadingMessage(getLocaleGuiString("gui_loading_regen_bin_convermap"));
#endif

			coverMap->create(obstacleHeightMap, pathfindHeightMap);
			bool saved = coverMap->save(bin_filename);
			if (!saved)
			{
				Logger::getInstance()->warning("GameMap::setData - Failed to save covermap.");
			}
		}
		delete [] bin_filename;
  }


	/*
  void GameMap::loadHideMap()
  {
		// either load the hidemap from disk or if the file is not 
		// up-to-date, set not-loaded flag.

		// NOTE: copy&pasted to saveHideMap below.
		char *bin_filename = new char[strlen(coverFilename) + 16];
		strcpy(bin_filename, coverFilename);

		if (strlen(coverFilename) > 10)
		{
			strcpy(&bin_filename[strlen(coverFilename) - 4-5], "hide.bin");
		} else {
			strcpy(&bin_filename[strlen(coverFilename)], "hide.bin");
			assert(0); // umm, bin filename sucked?
		}

		bool upToDate = FileTimestampChecker::isFileNewerOrSameThanFile(
			bin_filename, coverFilename);

		// if auto is off, make it up to date.
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_HIDE_BIN_RECREATE))
		{
			if (!upToDate)
			{
				Logger::getInstance()->warning("GameMap::loadHideMap - Hidemap is not up to date, but it will not be recreated (auto recreate off).");
				upToDate = true;
			}
		}

		// if force is on, treat as being out-of-date
		if (SimpleOptions::getBool(DH_OPT_B_FORCE_HIDE_BIN_RECREATE))
		{
			Logger::getInstance()->debug("GameMap::loadHideMap - Hidemap is recreated (force recreate on).");
			upToDate = false;
		}

		bool loaded = false;
		if (upToDate)
		{
			loaded = hideMap->load(bin_filename);
		}
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_HIDE_BIN_RECREATE))
		{
			loaded = true; // skip recreate even if load fails!
		} else {
			if (upToDate && !loaded)
			{
				Logger::getInstance()->warning("GameMap::loadHideMap - Failed to load hidemap (will attempt to recreate it).");
			}
		}
		if (!loaded)
		{
			LoadingMessage::showLoadingMessage(getLocaleGuiString("gui_loading_regen_bin_hidemap"));
		}
		delete [] bin_filename;

		hideMapLoaded = loaded;
	}
	*/


	/*
  bool GameMap::isHideMapLoaded()
  {
		// tells if we've succeeded to load the hidemap
		// this method should be called to find out if the hidemap
		// should be created while creating the map (terrain+buildings)...
		return hideMapLoaded;
	}
	*/


	/*
  void GameMap::saveHideMap()
  {
		// save the hidemap, if it was not originally loaded, but rather
		// created

		// NOTE: copy&pasted from loadHideMap above
		char *bin_filename = new char[strlen(coverFilename) + 16];
		strcpy(bin_filename, coverFilename);
		if (strlen(coverFilename) > 10)
		{
			strcpy(&bin_filename[strlen(coverFilename) - 4-5], "hide.bin");
		} else {
			strcpy(&bin_filename[strlen(coverFilename)], "hide.bin");
			assert(0); // umm, bin filename sucked?
		}

		if (!hideMapLoaded)
		{
			bool saved = hideMap->save(bin_filename);
			if (!saved)
			{
				Logger::getInstance()->warning("GameMap::saveHideMap - Failed to save hidemap.");
			}
		}
	}
	*/


  void GameMap::loadObstacleAndAreaMap(frozenbyte::ai::PathFind *pathfinder)
  {
		// either load the hidemap from disk or if the file is not 
		// up-to-date, set not-loaded flag.

		// NOTE: copy&pasted to saveHideMap below.
		char *bin_filename = new char[strlen(coverFilename) + 32];
		strcpy(bin_filename, coverFilename);

		if (strlen(coverFilename) > 10)
		{
			strcpy(&bin_filename[strlen(coverFilename) - 4-5], "obstacle.bin");
		} else {
			strcpy(&bin_filename[strlen(coverFilename)], "obstacle.bin");
			assert(0); // umm, bin filename sucked?
		}

		bool upToDate = FileTimestampChecker::isFileNewerOrSameThanFile(
			bin_filename, coverFilename);

		// if auto is off, make it up to date.
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_OBSTACLE_BIN_RECREATE))
		{
			if (!upToDate)
			{
				Logger::getInstance()->warning("GameMap::loadObstacleAndAreaMap - Obstacle/area map is not up to date, but it will not be recreated (auto recreate off).");
				upToDate = true;
			}
		}

		// if force is on, treat as being out-of-date
		if (SimpleOptions::getBool(DH_OPT_B_FORCE_OBSTACLE_BIN_RECREATE))
		{
			Logger::getInstance()->debug("GameMap::loadObstacleAndAreaMap - Obstacle/area map is recreated (force recreate on).");
			upToDate = false;
		}

		bool loaded = false;
		if (upToDate)
		{
			loaded = this->loadObstacleAndAreaImpl(bin_filename, pathfinder);
		}
		if (!SimpleOptions::getBool(DH_OPT_B_AUTO_OBSTACLE_BIN_RECREATE))
		{
			loaded = true; // skip recreate even if load fails!
		} else {
			if (upToDate && !loaded)
			{
				Logger::getInstance()->warning("GameMap::loadObstacleAndAreaMap - Failed to load obstacle/area map (will attempt to recreate it).");
			}
		}
#ifndef PROJECT_SURVIVOR
		if (!loaded)
		{
			LoadingMessage::showLoadingMessage(getLocaleGuiString("gui_loading_regen_bin_obstaclemap"));
		}
#endif
		delete [] bin_filename;

		obstacleAndAreaMapLoaded = loaded;
	}


  bool GameMap::isObstacleAndAreaMapLoaded()
  {
		// tells if we've succeeded to load the hidemap
		// this method should be called to find out if the hidemap
		// should be created while creating the map (terrain+buildings)...
		return obstacleAndAreaMapLoaded;
	}


  void GameMap::saveObstacleAndAreaMap(frozenbyte::ai::PathFind *pathfinder)
  {
		// save the hidemap, if it was not originally loaded, but rather
		// created

		// NEW: apply the collision height map too (cannot do that much earlier)
		if (this->precalcedPathfindHeightMap != NULL)
		{
			for (int i = 0; i < sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER * sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER; i++)
			{
				this->pathfindHeightMap[i] = this->precalcedPathfindHeightMap[i];
			}
			delete [] this->precalcedPathfindHeightMap;
			this->precalcedPathfindHeightMap = NULL;
		}

		// NOTE: copy&pasted from loadHideMap above
		char *bin_filename = new char[strlen(coverFilename) + 32];
		strcpy(bin_filename, coverFilename);
		if (strlen(coverFilename) > 10)
		{
			strcpy(&bin_filename[strlen(coverFilename) - 4-5], "obstacle.bin");
		} else {
			strcpy(&bin_filename[strlen(coverFilename)], "obstacle.bin");
			assert(0); // umm, bin filename sucked?
		}

		if (!obstacleAndAreaMapLoaded)
		{
			bool saved = this->saveObstacleAndAreaImpl(bin_filename, pathfinder);
			if (!saved)
			{
				Logger::getInstance()->warning("GameMap::saveObstacleAndAreaMap - Failed to save obstacle/area map.");
			}
		}

		// NOTE: set loaded flag to false - this allows dynamic obstacle/areamap changes 
		// to take effect (when breaking terrainobjects)
		// (otherwise they would be ignored)
		obstacleAndAreaMapLoaded = false;
	}


  /*
  void GameMap::loadMap(char *filename) throw (GameMapLoadException *)
  {
    // TODO: read/solve map size from file

    FILE *f = fopen(filename, "rb");
    if (f == NULL) 
    {
      heightMap = NULL;
      sizeX = 0;
      sizeY = 0;
      throw new GameMapLoadException("GameMap::loadMap - Could not open map file");
    }

    sizeX = 512;
    sizeY = 512;
    sizeHeight = 65536;
    scaleX = 128;
    scaleY = 128;
    scaleHeight = 0.0066f;
    scaledSizeX = sizeX * scaleX;
    scaledSizeY = sizeY * scaleY;
    scaledSizeHeight = (sizeHeight * scaleHeight);

    WORD *buf = new WORD[sizeX * sizeY];

    int got = fread(buf, sizeof(WORD), sizeY * sizeY, f);

    fclose(f);

    if (got != sizeX * sizeY)
    {
      delete [] buf;
      heightMap = NULL;
      sizeX = 0;
      sizeY = 0;
      throw new GameMapLoadException("GameMap::loadMap - Could not read the whole map.");
    }

    heightMap = buf;
  }
  */

  int GameMap::getHeightmapHeightAt(int x, int y)
  {
    #ifdef _DEBUG
      if (x < 0 || y < 0 || x >= sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER || y >= sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER) abort();
    #endif
    return (int)pathfindHeightMap[x + y * sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER];
    //return (int)heightMap[x + y * sizeX];
  }

  void GameMap::setHeightmapHeightAt(int x, int y, int value)
  {
    #ifdef _DEBUG
      if (x < 0 || y < 0 || x >= sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER || y >= sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER) abort();
    #endif
    pathfindHeightMap[x + y * sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER] = value;
    //return (int)heightMap[x + y * sizeX];
  }

  /*
  int GameMap::getFixedPointHeightAt(int fixedX, int fixedY)
  {
    float x = (float)fixedX / 256.0f;
    float y = (float)fixedY / 256.0f;
    int ret = (int)(terrain->GetHeightAt(VC2(x, y)) * 256.0f);
    return ret;
  }
  */

  float GameMap::getScaledHeightAt(float scaledX, float scaledY)
  {
		// TODO: optimize, get rid of these float -> int conversions :(
		int ox = this->scaledToObstacleX(scaledX);
		int oy = this->scaledToObstacleY(scaledY);
		if (ox < 0 || oy < 0 || ox >= pathfindSizeX || oy >= pathfindSizeY)
			return 0;
		if (areaMap->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
			return terrain->getPartiallyInterpolatedHeight(VC2(scaledX, scaledY));
		else
			return terrain->getHeight(VC2(scaledX, scaledY));
  }

  /*
  int GameMap::getHeightmapSizeX()
  {
    return sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
  }

  int GameMap::getHeightmapSizeY()
  {
    return sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;
  }
  */

  int GameMap::getHeightmapMaxHeight()
  {
    return sizeHeight;
  }

  float GameMap::getScaledSizeX()
  {
    return scaledSizeX;
  }

  float GameMap::getScaledSizeY()
  {
    return scaledSizeY;
  }

  float GameMap::getScaledMaxHeight()
  {
    return scaledSizeHeight;
  }

  float GameMap::getScaleX()
  {
    return scaleX;
  }

  float GameMap::getScaleY()
  {
    return scaleY;
  }

  float GameMap::getScaleHeight()
  {
    return scaleHeight;
  }

	void GameMap::makeHeightAreaBlocked(int heightMapX, int heightMapY)
	{
		// FIXME: this implementation blocks ALL (even non-connected) areas 
		// of this height. should only block the connected area!!!

		int x = heightMapX;
		int y = heightMapY;
		int height = this->getHeightmapHeightAt(x, y);

		BlockedHeightAreaFillMapper mapper = BlockedHeightAreaFillMapper(this, height);

		int xResolution = this->sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
		int yResolution = this->sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;

		// in this case, 2 means "has blocked this height area"
		// 1 means "height are to be blocked"
		// 0 means "some other height (do nothing here)"
		util::Floodfill::fillWithByte(0, 1, xResolution, yResolution, &mapper, false, false);
	}


	bool GameMap::loadObstacleAndAreaImpl(const char *filename, frozenbyte::ai::PathFind *pathfinder)
	{
		assert(filename != NULL);
		assert(this->obstacleHeightMap != NULL);
		assert(this->areaMap != NULL);
		assert(this->precalcedPathfindHeightMap == NULL);

		bool success = false;

		//filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		RLE_PACKED_FILE *f = rle_packed_fopen(filename, "rb");

		if (f != NULL)
		{
			// can't read the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < pathfindSizeY; i++)
			{
				got += rle_packed_fread(&obstacleHeightMap[i * pathfindSizeX], sizeof(unsigned short) * pathfindSizeX, 1, f);
			}
			if (got != pathfindSizeY)
			{
				Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Error reading file (at obstaclemap).");
				Logger::getInstance()->debug(filename);
			} else {
				int got2 = 0;
				AREAMAP_DATATYPE *amap = areaMap->getInternalBuffer();
				for (int i = 0; i < pathfindSizeY; i++)
				{
					got2 += rle_packed_fread(&amap[i * pathfindSizeX], sizeof(AREAMAP_DATATYPE) * pathfindSizeX, 1, f);
				}
				if (got2 != pathfindSizeY)
				{
					Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Error reading file (at areamap).");
					Logger::getInstance()->debug(filename);
				} else {
					int got3 = 0;
					unsigned char *tmpbuf = new unsigned char[pathfindSizeX];
					for (int i = 0; i < pathfindSizeY; i++)
					{
						got3 += rle_packed_fread(tmpbuf, sizeof(signed char) * pathfindSizeX, 1, f);
						for (int xfill = 0; xfill < pathfindSizeX; xfill++)
						{
							pathfinder->setBlockingCount(xfill, i, tmpbuf[xfill]);
						}
					}
					delete [] tmpbuf;
					if (got3 != pathfindSizeY)
					{
						Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Error reading file (at pathfinder).");
						Logger::getInstance()->debug(filename);
					} else {
						int xResolution = this->sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
						int yResolution = this->sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;
						assert(xResolution >= 16 && xResolution <= 1024);
						assert(yResolution >= 16 && yResolution <= 1024);
						assert(heightMap != NULL);
						this->precalcedPathfindHeightMap = new unsigned short[xResolution * yResolution];
						int got4 = 0;
						for (int i = 0; i < yResolution; i++)
						{
							got4 += rle_packed_fread(&precalcedPathfindHeightMap[i * xResolution], sizeof(unsigned short) * xResolution, 1, f);
						}
						if (got4 != yResolution)
						{
							Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Error reading file (at heightmap).");
							Logger::getInstance()->debug(filename);
						} else {
							Logger::getInstance()->debug("GameMap::loadObstacleAndAreaImpl - Obstacle/area map successfully loaded.");
							success = true;
						}
					}
				}
			}
			if (rle_packed_was_error(f))
			{
				Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Error while reading/unpacking file.");
				Logger::getInstance()->debug(filename);
			}
			rle_packed_fclose(f);
		} else {
			Logger::getInstance()->error("GameMap::loadObstacleAndAreaImpl - Could not open file.");
			Logger::getInstance()->debug(filename);
		}

		return success;
	}


	bool GameMap::saveObstacleAndAreaImpl(const char *filename, frozenbyte::ai::PathFind *pathfinder)
	{
		assert(filename != NULL);

		bool success = false;

		//FILE *f = fopen(filename, "wb");
		RLE_PACKED_FILE *f = rle_packed_fopen(filename, "wb");

		if (f != NULL)
		{
			// can't write the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < pathfindSizeY; i++)
			{
				got += rle_packed_fwrite(&obstacleHeightMap[i * pathfindSizeX], sizeof(unsigned short) * pathfindSizeX, 1, f);
			}
			if (got != pathfindSizeY)
			{
				Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Error writing file (at obstaclemap).");
				Logger::getInstance()->debug(filename);
			} else {
				int got2 = 0;
				assert(areaMap != NULL);
				AREAMAP_DATATYPE *amap = areaMap->getInternalBuffer();
				assert(amap != NULL);
				for (int i = 0; i < pathfindSizeY; i++)
				{
					got2 += rle_packed_fwrite(&amap[i * pathfindSizeX], sizeof(AREAMAP_DATATYPE) * pathfindSizeX, 1, f);
				}
				if (got2 != pathfindSizeY)
				{
					Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Error writing file (at areamap).");
					Logger::getInstance()->debug(filename);
				} else {
					int got3 = 0;
					assert(pathfinder != NULL);

					unsigned char *tmpbuf = new unsigned char[pathfindSizeX];
					for (int i = 0; i < pathfindSizeY; i++)
					{
						for (int xfill = 0; xfill < pathfindSizeX; xfill++)
						{
							tmpbuf[xfill] = pathfinder->getBlockingCount(xfill, i);
						}
						got3 += rle_packed_fwrite(tmpbuf, sizeof(signed char) * pathfindSizeX, 1, f);
					}
					delete [] tmpbuf;

					if (got3 != pathfindSizeY)
					{
						Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Error writing file (at pathfinder).");
						Logger::getInstance()->debug(filename);
					} else {

						// --- TEMP: write heightmap to own file ---
						/*
						FILE *fhe = fopen("heightmap_temp.raw", "wb");
						if (fhe != NULL)
						{
							int xResolution = this->sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
							int yResolution = this->sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;
							int gotfoo = 0;
							for (int i = 0; i < yResolution; i++)
							{
								gotfoo += fwrite(&pathfindHeightMap[i * xResolution], sizeof(unsigned short) * xResolution, 1, fhe);
							}
							fclose(fhe);
						}
						*/
						// --- TEMP: end of temp ---

						int xResolution = this->sizeX * GAMEMAP_HEIGHTMAP_MULTIPLIER;
						int yResolution = this->sizeY * GAMEMAP_HEIGHTMAP_MULTIPLIER;
						int got4 = 0;
						for (int i = 0; i < yResolution; i++)
						{
							got4 += rle_packed_fwrite(&pathfindHeightMap[i * xResolution], sizeof(unsigned short) * xResolution, 1, f);
						}
						if (got4 != yResolution)
						{
							Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Error writing file (at heightmap).");
							Logger::getInstance()->debug(filename);
						} else {
							Logger::getInstance()->debug("GameMap::saveObstacleAndAreaImpl - Obstacle/area map successfully saved.");
							success = true;
						}
					}
				}
			}
			if (rle_packed_was_error(f))
			{
				Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Error while writing/packing file.");
				Logger::getInstance()->debug(filename);
			}
			rle_packed_fclose(f);
		} else {
			Logger::getInstance()->error("GameMap::saveObstacleAndAreaImpl - Could not open file.");
			Logger::getInstance()->debug(filename);
		}

		return success;
	}

}
