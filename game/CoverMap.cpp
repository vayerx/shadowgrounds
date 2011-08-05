
#include "precompiled.h"

#include "CoverMap.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../util/DistanceFloodfill.h"
#include "../util/Debug_MemoryManager.h"
#include "../system/Logger.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/rle_packed_file_wrapper.h"

#include "../util/Debug_MemoryManager.h"

// TEMP DEBUGGING!
//#define COVERMAP_SAVE_MONITOR_IMAGE

using namespace frozenbyte;


namespace game
{

	// a class for writing progress monitoring image...
	class CoverMapMonitor : public util::IDistanceFloodfillMonitor
	{
		private:
			int sizeX;
			int sizeY;
			char *filename;
			unsigned char *rangemap;
			
		public:
			CoverMapMonitor(char *filename, unsigned char *rangemap, 
				int sizeX, int sizeY) 
			{ 
				this->sizeX = sizeX;
				this->sizeY = sizeY; 
				this->rangemap = rangemap;
				this->filename = new char[strlen(filename) + 1];
				strcpy(this->filename, filename);
			}
			virtual ~CoverMapMonitor() 
			{
				delete [] filename;
			}
			virtual void distanceFloodfillProgress(int line)
			{
				FILE *f = fopen(filename, "wb");
				if (f != NULL)
				{
					for (int i = 0; i < sizeY; i++)
					{
						fwrite(&rangemap[i * sizeX], sizeX, 1, f);
					}
					fclose(f);					
				}
			}
	};

	const int CoverMap::COVER_FROM_WEST_MASK = 1;
	const int CoverMap::COVER_FROM_EAST_MASK = 2;
	const int CoverMap::COVER_FROM_NORTH_MASK = 4;
	const int CoverMap::COVER_FROM_SOUTH_MASK = 8;
	const int CoverMap::COVER_FROM_ALL_MASK = 15;

	CoverMap::CoverMap(int sizeX, int sizeY)
	{
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		covermap = new unsigned short[sizeX * sizeY];
		for (int i = 0; i < sizeX * sizeY; i++)
		{
			covermap[i] = 0;
		}
	}


	CoverMap::~CoverMap()
	{
		delete [] covermap;
	}


	bool CoverMap::load(char *filename)
	{
		assert(filename != NULL);

		bool success = false;

		RLE_PACKED_FILE *f = rle_packed_fopen(filename, "rb");
		if (f != NULL)
		{
			// can't read the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < sizeY; i++)
			{
				got += rle_packed_fread(&covermap[i * sizeX], sizeof(unsigned short) * sizeX, 1, f);
			}
			if (got != sizeY)
			{
				Logger::getInstance()->error("CoverMap::load - Error reading file.");
				Logger::getInstance()->debug(filename);
			} else {
				success = true;
			}
			if (rle_packed_was_error(f))
			{
				Logger::getInstance()->error("CoverMap::load - Error while reading/unpacking file.");
				Logger::getInstance()->debug(filename);
			}
			rle_packed_fclose(f);
		} else {
			Logger::getInstance()->error("CoverMap::load - Could not open file.");
			Logger::getInstance()->debug(filename);
		}
		return success;
	}


	bool CoverMap::save(char *filename)
	{
		assert(filename != NULL);

		bool success = false;

		RLE_PACKED_FILE *f = rle_packed_fopen(filename, "wb");
		if (f != NULL)
		{
			// can't write the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < sizeY; i++)
			{
				got += rle_packed_fwrite(&covermap[i * sizeX], sizeof(unsigned short) * sizeX, 1, f);
			}
			if (got != sizeY)
			{
				Logger::getInstance()->error("CoverMap::save - Error writing file.");
				Logger::getInstance()->debug(filename);
			} else {
				success = true;
			}
			rle_packed_fclose(f);
		} else {
			Logger::getInstance()->error("CoverMap::save - Could not open file.");
			Logger::getInstance()->debug(filename);
		}

		return success;
	}


	void CoverMap::create(unsigned short *obstacleMap, 
		unsigned short *heightMap)
	{
		// NOTICE: heightmap unused.
		// it is half the size of the obstacle map 
		// (1/2 width, 1/2 height, 1/4 in size)
		// depends on pathfind accuracy factor (which is 2)

		unsigned char *amap = new unsigned char[sizeX * sizeY];
		unsigned char *rmap = new unsigned char[sizeX * sizeY];

		int x, y;

		for (y = 0; y < sizeY; y++)
		{
			for (x = 0; x < sizeX; x++)
			{
				if (obstacleMap[x + y * sizeX] >= 1)
				{
					amap[x + y * sizeX] = 1;
					rmap[x + y * sizeX] = 0;
				} else {
					amap[x + y * sizeX] = 0;
					rmap[x + y * sizeX] = 255;
				}
			}
		}

		util::IDistanceFloodfillMonitor *monitor = NULL;
#ifdef COVERMAP_SAVE_MONITOR_IMAGE
		CoverMapMonitor mon("monitor.raw", rmap, sizeX, sizeY);
		monitor = &mon;
#endif
		util::DistanceFloodfill::fillRanges(0, sizeX, sizeY, amap, rmap, 
			false, true, monitor);

		for (y = 0; y < sizeY; y++)
		{
			for (x = 0; x < sizeX; x++)
			{
				unsigned short m = 0;
				bool blocked_n = false;
				bool blocked_s = false;
				bool blocked_w = false;
				bool blocked_e = false;
				bool obst_nearer_to_n = false;
				bool obst_nearer_to_s = false;
				bool obst_nearer_to_w = false;
				bool obst_nearer_to_e = false;
				unsigned char r = rmap[x + y * sizeX];
				if (y > 0)
				{
					if (obstacleMap[x + (y - 1) * sizeX] >= 1)
						blocked_n = true;
					if (rmap[x + (y - 1) * sizeX] < r)
						obst_nearer_to_n = true;
				}
				if (y < sizeY - 1)
				{
					if (obstacleMap[x + (y + 1) * sizeX] >= 1)
						blocked_s = true;
					if (rmap[x + (y + 1) * sizeX] < r)
						obst_nearer_to_s = true;
				}
				if (x > 0)
				{
					if (obstacleMap[x - 1 + y * sizeX] >= 1)
						blocked_w = true;
					if (rmap[x - 1 + y * sizeX] < r)
						obst_nearer_to_w = true;
				}
				if (x < sizeX - 1)
				{
					if (obstacleMap[x + 1 + y * sizeX] >= 1)
						blocked_e = true;
					if (rmap[x + 1 + y * sizeX] < r)
						obst_nearer_to_e = true;
				}

				// obstacle sides...
				if (blocked_w) m |= (1<<12);
				if (blocked_e) m |= (1<<13);
				if (blocked_n) m |= (1<<14);
				if (blocked_s) m |= (1<<15);

				// distance to nearest obstacle...
				m |= ((unsigned short)rmap[x + y * sizeX]);

				// a bit harsh way to determine the nearest obstacle's direction...
				// TODO: a better implementation.
				unsigned short obst_dir = 0;
				if (obst_nearer_to_w) obst_dir = 6;
				if (obst_nearer_to_e) obst_dir = 2;
				if (obst_nearer_to_n) obst_dir = 0;
				if (obst_nearer_to_s) obst_dir = 4;
				m |= (obst_dir << 8);

				// set
				covermap[x + y * sizeX] = m;
			}
		}

		delete [] rmap;
		delete [] amap;
	}


	CoverMap::COVER_DIRECTION CoverMap::getNearestCoverDirection(int x, int y)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);
		int dir = (int)((covermap[x + y * sizeX] >> 8) & 7);
		// NOTICE: cast from int to enum!
		return (CoverMap::COVER_DIRECTION)(dir + 1);
	}


	bool CoverMap::isCoveredFromAll(int fromMask, int x, int y)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);
		int covered = (int)((covermap[x + y * sizeX] >> 12) & 15);
		if ((covered & fromMask) == fromMask)
			return true;
		else
			return false;
	}


	bool CoverMap::isCoveredFromAny(int fromMask, int x, int y)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);
		int covered = (int)((covermap[x + y * sizeX] >> 12) & 15);
		if ((covered & fromMask) != 0)
			return true;
		else
			return false;
	}

	void CoverMap::removeCover(int x, int y)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);

		for(int j = -4; j <= 4; ++j)
		for(int i = -4; i <= 4; ++i)
		{
			int yp = y + j;
			int xp = x + i;

			if(xp <= 0 || xp >= sizeX)
				continue;
			if(yp <= 0 || yp >= sizeX)
				continue;

			int value = covermap[xp + yp * sizeX];
			if(value || (x == xp && y == yp))
				covermap[xp + yp * sizeX]= 5;
		}
	}

}

