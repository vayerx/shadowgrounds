
#include "HideMap.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../system/Logger.h"
#include "../filesystem/input_stream_wrapper.h"


// NOTICE: these must match the maxHiddeness
#define HIDEMAP_VALUE_MASK 31

#define HIDEMAP_TYPE_MASK_VEGETATION 32
#define HIDEMAP_TYPE_MASK_TREE 64
#define HIDEMAP_TYPE_MASK_SOLID 128
#define HIDEMAP_MASK_ALL 255

using namespace frozenbyte;

namespace game
{
	const int HideMap::maxHiddeness = 31;

	HideMap::HideMap(int sizeX, int sizeY)
	{
		assert(maxHiddeness == HIDEMAP_VALUE_MASK);

		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->hidemap = new unsigned char[sizeX * sizeY];
		for (int i = 0; i < sizeX * sizeY; i++)
		{
			hidemap[i] = 0;
		}
	}


	HideMap::~HideMap()
	{
		if (hidemap != NULL)
		{
			delete [] hidemap;
		}
	}


	void HideMap::setHiddenessType(int x, int y, HIDDENESS_TYPE hidType)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);

		// don't cleat the existing, add this type on top of existing
		// except if we're setting the NONE type...
		if (hidType == HIDDENESS_TYPE_NONE)
		{
			hidemap[x + y * sizeX] &= HIDEMAP_VALUE_MASK;
			return;
		}
		if (hidType == HIDDENESS_TYPE_VEGETATION)
		{
			hidemap[x + y * sizeX] |= HIDEMAP_TYPE_MASK_VEGETATION;
			return;
		}
		if (hidType == HIDDENESS_TYPE_TREE)
		{
			hidemap[x + y * sizeX] |= HIDEMAP_TYPE_MASK_TREE;
			return;
		}
		if (hidType == HIDDENESS_TYPE_SOLID)
		{
			hidemap[x + y * sizeX] |= HIDEMAP_TYPE_MASK_SOLID;
			return;
		}
	}


	void HideMap::setHiddeness(int x, int y, int amount)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);

		assert(amount >= 0 && amount <= HIDEMAP_VALUE_MASK);

		hidemap[x + y * sizeX] &= (HIDEMAP_MASK_ALL - HIDEMAP_VALUE_MASK);
		hidemap[x + y * sizeX] |= amount;
	}


	void HideMap::addHiddenessToArea(int x, int y, int radius, 
		int amount, HIDDENESS_TYPE hidType)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);
		assert(radius > 0);
		assert(amount > 0 && amount <= HIDEMAP_VALUE_MASK);

		// TODO: circle radius, not rectangle!
		int x1 = x - radius;
		int x2 = x + radius;
		int y1 = y - radius;
		int y2 = y + radius;
		if (x1 < 0) x1 = 0;
		if (x2 > sizeX-1) x2 = sizeX-1;
		if (y1 < 0) y1 = 0;
		if (y2 > sizeY-1) y2 = sizeY-1;

		for (int ty = y1; ty <= y2; ty++)
		{
			for (int tx = x1; tx <= x2; tx++)
			{
				setHiddenessType(tx, ty, hidType);
				if (getHiddenessAt(tx, ty) < amount)
					setHiddeness(tx, ty, amount);
			}
		}
	}


	int HideMap::getHiddenessAt(int x, int y)
	{
		assert(x >= 0 && x < sizeX);
		assert(y >= 0 && y < sizeY);

		return (hidemap[x + y * sizeX] & HIDEMAP_VALUE_MASK);
	}


	HideMap::HIDDENESS_TYPE HideMap::getHiddenessTypeAt(int x, int y)
	{
		// TODO...
		// which one of the types should be "topmost"?
		// which hiddeness type do we possibly prefer?

		if (hidemap[x + y * sizeX] & HIDEMAP_TYPE_MASK_VEGETATION)
		{
			return HIDDENESS_TYPE_VEGETATION;
		}
		if (hidemap[x + y * sizeX] & HIDEMAP_TYPE_MASK_TREE)
		{
			return HIDDENESS_TYPE_TREE;
		}
		if (hidemap[x + y * sizeX] & HIDEMAP_TYPE_MASK_SOLID)
		{
			return HIDDENESS_TYPE_SOLID;
		}
		return HIDDENESS_TYPE_NONE;
	}


	bool HideMap::load(const char *filename)
	{
		assert(filename != NULL);

		bool success = false;

		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		if (f != NULL)
		{
			// can't read the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < sizeY; i++)
			{
				got += filesystem::fb_fread(&hidemap[i * sizeX], sizeof(unsigned char) * sizeX, 1, f);
			}
			if (got != sizeY)
			{
				Logger::getInstance()->error("HideMap::load - Error reading file.");
				Logger::getInstance()->debug(filename);
			} else {
				success = true;
			}
			filesystem::fb_fclose(f);
		} else {
			Logger::getInstance()->error("HideMap::load - Could not open file.");
			Logger::getInstance()->debug(filename);
		}
		return success;
	}


	bool HideMap::save(const char *filename)
	{
		assert(filename != NULL);

		bool success = false;

		FILE *f = fopen(filename, "wb");
		if (f != NULL)
		{
			// can't write the file in one block? too big? chop to pieces...
			int got = 0;
			for (int i = 0; i < sizeY; i++)
			{
				got += fwrite(&hidemap[i * sizeX], sizeof(unsigned char) * sizeX, 1, f);
			}
			if (got != sizeY)
			{
				Logger::getInstance()->error("HideMap::save - Error writing file.");
				Logger::getInstance()->debug(filename);
			} else {
				success = true;
			}
			fclose(f);
		} else {
			Logger::getInstance()->error("HideMap::save - Could not open file.");
			Logger::getInstance()->debug(filename);
		}

		return success;
	}



}

