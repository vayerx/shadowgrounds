
#include "precompiled.h"

#include "DistanceFloodfill.h"

#include <assert.h>
#include <stdlib.h>

//#define DISTANCE_FLOODFILL_MAX_RANGE 255

// optimized for this game...
#define DISTANCE_FLOODFILL_MAX_RANGE 128


namespace util
{

  DistanceFloodfillByteArrayMapper::DistanceFloodfillByteArrayMapper(int sizeX, int sizeY, unsigned char *buf)
  {
    assert(buf != NULL);
    assert(sizeX > 0);
    assert(sizeY > 0);
    this->buf = buf;
    this->sizeX = sizeX;
    this->sizeY = sizeY;
  }


  unsigned char DistanceFloodfillByteArrayMapper::getByte(int x, int y)
  {
    assert(x >= 0 && x < sizeX);
    assert(y >= 0 && y < sizeY);
    return buf[x + y * sizeX];
  }


  void DistanceFloodfillByteArrayMapper::setByte(int x, int y, unsigned char value)
  {
    assert(x >= 0 && x < sizeX);
    assert(y >= 0 && y < sizeY);
    buf[x + y * sizeX] = value;
  }


  void DistanceFloodfill::fillRanges(unsigned char areaByte, 
    int mapSizeX, int mapSizeY, unsigned char *map,
		unsigned char *rangeMap,
    bool areaIsBlocking, bool corners, IDistanceFloodfillMonitor *monitor)
  {
		int progress = 0;

		int sh = 0;
	  int i = mapSizeX;
		while (i > 1)
		{
			i /= 2;
			sh++;
		}
		if ((1 << sh) != mapSizeX)
		{
			// unoptimized...

			DistanceFloodfillByteArrayMapper mapper = 
				DistanceFloodfillByteArrayMapper(mapSizeX, mapSizeY, map);
			DistanceFloodfillByteArrayMapper rangeMapper = 
				DistanceFloodfillByteArrayMapper(mapSizeX, mapSizeY, rangeMap);

			fillRanges(areaByte, mapSizeX, mapSizeY, &mapper, 
				&rangeMapper, areaIsBlocking, corners, monitor);

			return;
		}

		// optimized... (only for 2^n sizes)

    if (areaIsBlocking)
    {

      // the areabyte is the only byte that we WILL NOT flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
				bool linedone = true;
        for (int x = 0; x < mapSizeX; x++)
        {
					unsigned char r = rangeMap[x + (y << sh)];
					unsigned char rp = r + 1;
          if (r < DISTANCE_FLOODFILL_MAX_RANGE)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (map[x + ((y - 1) << sh)] != areaByte
                && rangeMap[x + ((y - 1) << sh)] > rp)
              {
                rangeMap[x + ((y - 1) << sh)] = rp;
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && map[x - 1 + ((y - 1) << sh)] != areaByte
                  && rangeMap[x - 1 + ((y - 1) << sh)] > rp)
                {
                  rangeMap[x - 1 + ((y - 1) << sh)] = rp;
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && map[x + 1 + ((y - 1) << sh)] != areaByte
                  && rangeMap[x + 1 + ((y - 1) << sh)] > rp)
                {
                  rangeMap[x + 1 + ((y - 1) << sh)] = rp;
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (map[x + ((y + 1) << sh)] != areaByte
                && rangeMap[x + ((y + 1) << sh)] > rp)
              {
                rangeMap[x + ((y + 1) << sh)] = rp;
              }
              if (corners)
              {
                if (x > 0 && map[x - 1 + ((y + 1) << sh)] != areaByte
                  && rangeMap[x - 1 + ((y + 1) << sh)] > rp)
                {
                  rangeMap[x - 1 + ((y + 1) << sh)] = rp;
                }
                if (x < mapSizeX-1 && map[x + 1 + ((y + 1) << sh)] != areaByte
                  && rangeMap[x + 1 + ((y + 1) << sh)] > rp)
                {
                  rangeMap[x + 1 + ((y + 1) << sh)] = rp;
                }
              }
            }
            if (x > 0 && map[x - 1 + (y << sh)] != areaByte
              && rangeMap[x - 1 + (y << sh)] > rp)
            {
              rangeMap[x - 1 + (y << sh)] = rp;
              moveLeft = true;
            }
            if (x < mapSizeX-1 && map[x + 1 + (y << sh)] != areaByte
              && rangeMap[x + 1 + (y << sh)] > rp)
            {
              rangeMap[x + 1 + (y << sh)] = rp;
            }
            if (moveUp) 
            {
							linedone = false;
              y--;
              if (moveLeft) 
                x -= 2;
              else
                x--;
            } else {
              if (moveLeft) 
                x -= 2;
            }
          }
        }
				if (linedone)
				{
					if (progress < y)
					{
						progress = y;
						if (monitor != NULL)
							monitor->distanceFloodfillProgress(progress);
					}
				}
      }

    } else {

      // the areabyte is the only byte we WILL flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
				bool linedone = true;
        for (int x = 0; x < mapSizeX; x++)
        {
					unsigned char r = rangeMap[x + (y << sh)];
					unsigned char rp = r + 1;
          if (r < DISTANCE_FLOODFILL_MAX_RANGE)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (map[x + ((y - 1) << sh)] == areaByte
                && rangeMap[x + ((y - 1) << sh)] > rp)
              {
                rangeMap[x + ((y - 1) << sh)] = rp;
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && map[x - 1 + ((y - 1) << sh)] == areaByte
                  && rangeMap[x - 1 + ((y - 1) << sh)] > rp)
                {
                  rangeMap[x - 1 + ((y - 1) << sh)] = rp;
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && map[x + 1 + ((y - 1) << sh)] == areaByte
                  && rangeMap[x + 1 + ((y - 1) << sh)] > rp)
                {
                  rangeMap[x + 1 + ((y - 1) << sh)] = rp;
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (map[x + ((y + 1) << sh)] == areaByte
                && rangeMap[x + ((y + 1) << sh)] > rp)
              {
                rangeMap[x + ((y + 1) << sh)] = rp;
              }
              if (corners)
              {
                if (x > 0 && map[x - 1 + ((y + 1) << sh)] == areaByte
                  && rangeMap[x - 1 + ((y + 1) << sh)] > rp)
                {
                  rangeMap[x - 1 + ((y + 1) << sh)] = rp;
                }
                if (x < mapSizeX-1 && map[x + 1 + ((y + 1) << sh)] == areaByte
                  && rangeMap[x + 1 + ((y + 1) << sh)] > rp)
                {
                  rangeMap[x + 1 + ((y + 1) << sh)] = rp;
                }
              }
            }
            if (x > 0 && map[x - 1 + (y << sh)] == areaByte
              && rangeMap[x - 1 + (y << sh)] > rp)
            {
              rangeMap[x - 1 + (y << sh)] = rp;
              moveLeft = true;
            }
            if (x < mapSizeX-1 && map[x + 1 + (y << sh)] == areaByte
              && rangeMap[x + 1 + (y << sh)] > rp)
            {
              rangeMap[x + 1 + (y << sh)] = rp;
            }
            if (moveUp) 
            {
							linedone = false;
              y--;
              if (moveLeft) 
                x -= 2;
              else
                x--;
            } else {
              if (moveLeft) 
                x -= 2;
            }
          }
        }
				if (linedone)
				{
					if (progress < y)
					{
						progress = y;
						if (monitor != NULL)
							monitor->distanceFloodfillProgress(progress);
					}
				}
      }

    }

  }


  void DistanceFloodfill::fillRanges(unsigned char areaByte, 
    int mapSizeX, int mapSizeY, IDistanceFloodfillByteMapper *mapper,
		IDistanceFloodfillByteMapper *rangeMapper,
    bool areaIsBlocking, bool corners, IDistanceFloodfillMonitor *monitor)
  {
		int progress = 0;

    if (areaIsBlocking)
    {

      // the areabyte is the only byte that we WILL NOT flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
				bool linedone = true;
        for (int x = 0; x < mapSizeX; x++)
        {
					unsigned char r = rangeMapper->getByte(x, y);
          if (r < 255)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (mapper->getByte(x, y - 1) != areaByte
                && rangeMapper->getByte(x, y - 1) > r + 1)
              {
                rangeMapper->setByte(x, y - 1, r + 1);
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y - 1) != areaByte
                  && rangeMapper->getByte(x - 1, y - 1) > r + 1)
                {
                  rangeMapper->setByte(x - 1, y - 1, r + 1);
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y - 1) != areaByte
                  && rangeMapper->getByte(x + 1, y - 1) > r + 1)
                {
                  rangeMapper->setByte(x + 1, y - 1, r + 1);
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (mapper->getByte(x, y + 1) != areaByte
                && rangeMapper->getByte(x, y + 1) > r + 1)
              {
                rangeMapper->setByte(x, y + 1, r + 1);
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y + 1) != areaByte
                  && rangeMapper->getByte(x - 1, y + 1) > r + 1)
                {
                  rangeMapper->setByte(x - 1, y + 1, r + 1);
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y + 1) != areaByte
                  && rangeMapper->getByte(x + 1, y + 1) > r + 1)
                {
                  rangeMapper->setByte(x + 1, y + 1, r + 1);
                }
              }
            }
            if (x > 0 && mapper->getByte(x - 1, y) != areaByte
              && rangeMapper->getByte(x - 1, y) > r + 1)
            {
              rangeMapper->setByte(x - 1, y, r + 1);
              moveLeft = true;
            }
            if (x < mapSizeX-1 && mapper->getByte(x + 1, y) != areaByte
              && rangeMapper->getByte(x + 1, y) > r + 1)
            {
              rangeMapper->setByte(x + 1, y, r + 1);
            }
            if (moveUp) 
            {
							linedone = false;
              y--;
              if (moveLeft) 
                x -= 2;
              else
                x--;
            } else {
              if (moveLeft) 
                x -= 2;
            }
          }
        }
				if (linedone)
				{
					if (progress < y)
					{
						progress = y;
						if (monitor != NULL)
							monitor->distanceFloodfillProgress(progress);
					}
				}
      }

    } else {

      // the areabyte is the only byte we WILL flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
				bool linedone = true;
        for (int x = 0; x < mapSizeX; x++)
        {
					unsigned char r = rangeMapper->getByte(x, y);
          if (r < 255)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (mapper->getByte(x, y - 1) == areaByte
                && rangeMapper->getByte(x, y - 1) > r + 1)
              {
                rangeMapper->setByte(x, y - 1, r + 1);
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y - 1) == areaByte
                  && rangeMapper->getByte(x - 1, y - 1) > r + 1)
                {
                  rangeMapper->setByte(x - 1, y - 1, r + 1);
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y - 1) == areaByte
                  && rangeMapper->getByte(x + 1, y - 1) > r + 1)
                {
                  rangeMapper->setByte(x + 1, y - 1, r + 1);
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (mapper->getByte(x, y + 1) == areaByte
                && rangeMapper->getByte(x, y + 1) > r + 1)
              {
                rangeMapper->setByte(x, y + 1, r + 1);
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y + 1) == areaByte
                  && rangeMapper->getByte(x - 1, y + 1) > r + 1)
                {
                  rangeMapper->setByte(x - 1, y + 1, r + 1);
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y + 1) == areaByte
                  && rangeMapper->getByte(x + 1, y + 1) > r + 1)
                {
                  rangeMapper->setByte(x + 1, y + 1, r + 1);
                }
              }
            }
            if (x > 0 && mapper->getByte(x - 1, y) == areaByte
              && rangeMapper->getByte(x - 1, y) > r + 1)
            {
              rangeMapper->setByte(x - 1, y, r + 1);
              moveLeft = true;
            }
            if (x < mapSizeX-1 && mapper->getByte(x + 1, y) == areaByte
              && rangeMapper->getByte(x + 1, y) > r + 1)
            {
              rangeMapper->setByte(x + 1, y, r + 1);
            }
            if (moveUp) 
            {
							linedone = false;
              y--;
              if (moveLeft) 
                x -= 2;
              else
                x--;
            } else {
              if (moveLeft) 
                x -= 2;
            }
          }
        }
				if (linedone)
				{
					if (progress < y)
					{
						progress = y;
						if (monitor != NULL)
							monitor->distanceFloodfillProgress(progress);
					}
				}
      }

    }
  }

}
