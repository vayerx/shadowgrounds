
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "Floodfill.h"

#include <assert.h>
#include <stdlib.h>


namespace util
{

  FloodfillByteArrayMapper::FloodfillByteArrayMapper(int sizeX, int sizeY, unsigned char *buf)
  {
    assert(buf != NULL);
    assert(sizeX > 0);
    assert(sizeY > 0);
    this->buf = buf;
    this->sizeX = sizeX;
    this->sizeY = sizeY;
  }


  unsigned char FloodfillByteArrayMapper::getByte(int x, int y)
  {
    assert(x >= 0 && x < sizeX);
    assert(y >= 0 && y < sizeY);
    return buf[x + y * sizeX];
  }


  void FloodfillByteArrayMapper::setByte(int x, int y, unsigned char value)
  {
    assert(x >= 0 && x < sizeX);
    assert(y >= 0 && y < sizeY);
    buf[x + y * sizeX] = value;
  }


  void Floodfill::fillWithByte(unsigned char fillByte, unsigned char areaByte, 
    int mapSizeX, int mapSizeY, unsigned char *map,
    bool areaIsBlocking, bool corners)
  {
    FloodfillByteArrayMapper mapper = 
      FloodfillByteArrayMapper(mapSizeX, mapSizeY, map);

    fillWithByte(fillByte, areaByte, mapSizeX, mapSizeY, &mapper, 
      areaIsBlocking, corners);
  }


  void Floodfill::fillWithByte(unsigned char fillByte, unsigned char areaByte, 
    int mapSizeX, int mapSizeY, IFloodfillByteMapper *mapper,
    bool areaIsBlocking, bool corners)
  {
    if (areaIsBlocking)
    {

      // the areabyte is the only byte that we WILL NOT flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
        for (int x = 0; x < mapSizeX; x++)
        {
          if (mapper->getByte(x, y) == fillByte)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (mapper->getByte(x, y - 1) != areaByte
                && mapper->getByte(x, y - 1) != fillByte)
              {
                mapper->setByte(x, y - 1, fillByte);
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y - 1) != areaByte
                  && mapper->getByte(x - 1, y - 1) != fillByte)
                {
                  mapper->setByte(x - 1, y - 1, fillByte);
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y - 1) != areaByte
                  && mapper->getByte(x + 1, y - 1) != fillByte)
                {
                  mapper->setByte(x + 1, y - 1, fillByte);
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (mapper->getByte(x, y + 1) != areaByte
                && mapper->getByte(x, y + 1) != fillByte)
              {
                mapper->setByte(x, y + 1, fillByte);
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y + 1) != areaByte
                  && mapper->getByte(x - 1, y + 1) != fillByte)
                {
                  mapper->setByte(x - 1, y + 1, fillByte);
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y + 1) != areaByte
                  && mapper->getByte(x + 1, y + 1) != fillByte)
                {
                  mapper->setByte(x + 1, y + 1, fillByte);
                }
              }
            }
            if (x > 0 && mapper->getByte(x - 1, y) != areaByte
              && mapper->getByte(x - 1, y) != fillByte)
            {
              mapper->setByte(x - 1, y, fillByte);
              moveLeft = true;
            }
            if (x < mapSizeX-1 && mapper->getByte(x + 1, y) != areaByte
              && mapper->getByte(x + 1, y) != fillByte)
            {
              mapper->setByte(x + 1, y, fillByte);
            }
            if (moveUp) 
            {
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
      }

    } else {

      // the areabyte is the only byte we WILL flood to...
      for (int y = 0; y < mapSizeY; y++)
      {
        for (int x = 0; x < mapSizeX; x++)
        {
          if (mapper->getByte(x, y) == fillByte)
          {
            bool moveLeft = false;
            bool moveUp = false;
            if (y > 0)
            {
              if (mapper->getByte(x, y - 1) == areaByte)
              {
                mapper->setByte(x, y - 1, fillByte);
                moveUp = true;
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y - 1) == areaByte)
                {
                  mapper->setByte(x - 1, y - 1, fillByte);
                  moveLeft = true;
                  moveUp = true;
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y - 1) == areaByte)
                {
                  mapper->setByte(x + 1, y - 1, fillByte);
                  moveUp = true;
                }
              }
            }
            if (y < mapSizeY-1)
            {
              if (mapper->getByte(x, y + 1) == areaByte)
              {
                mapper->setByte(x, y + 1, fillByte);
              }
              if (corners)
              {
                if (x > 0 && mapper->getByte(x - 1, y + 1) == areaByte)
                {
                  mapper->setByte(x - 1, y + 1, fillByte);
                }
                if (x < mapSizeX-1 && mapper->getByte(x + 1, y + 1) == areaByte)
                {
                  mapper->setByte(x + 1, y + 1, fillByte);
                }
              }
            }
            if (x > 0 && mapper->getByte(x - 1, y) == areaByte)
            {
              mapper->setByte(x - 1, y, fillByte);
              moveLeft = true;
            }
            if (x < mapSizeX-1 && mapper->getByte(x + 1, y) == areaByte)
            {
              mapper->setByte(x + 1, y, fillByte);
            }
            if (moveUp) 
            {
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
      }

    }
  }

}
