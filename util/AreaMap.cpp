
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "AreaMap.h"

namespace util
{

	class AreaMapImpl
	{
		private:

			AreaMapImpl(int sizeX, int sizeY)
			{
				this->sizeX = sizeX;
				this->sizeY = sizeY;
				this->areamap = new AREAMAP_DATATYPE[sizeX * sizeY];
				for (int i = 0; i < sizeX * sizeY; i++)
				{
					areamap[i] = 0;
				}
			}

			~AreaMapImpl()
			{
				delete [] areamap;
			}

			int sizeX;
			int sizeY;
			AREAMAP_DATATYPE *areamap;

		friend class AreaMap;
	};


	AreaMap::AreaMap(int sizeX, int sizeY)
	{
		impl = new AreaMapImpl(sizeX, sizeY);
	}

	AreaMap::~AreaMap()
	{
		delete impl;
	}

	// NOTICE: returns the unshifted value, you should shift that
	// if you want to get it between 0-n.
	int AreaMap::getAreaValue(int x, int y, int areaMask) const
	{
		return (impl->areamap[x + y * impl->sizeX] & areaMask);
	}

	bool AreaMap::isAreaValue(int x, int y, int areaMask, int value) const
	{
		if ((impl->areamap[x + y * impl->sizeX] & areaMask) == value)
			return true;
		else
			return false;
	}

	bool AreaMap::isAreaAnyValue(int x, int y, int areaMask) const
	{
		if ((impl->areamap[x + y * impl->sizeX] & areaMask) != 0)
			return true;
		else
			return false;
	}

	void AreaMap::setAreaValue(int x, int y, int areaMask, int value)
	{
		impl->areamap[x + y * impl->sizeX] = 
			((impl->areamap[x + y * impl->sizeX] & (~areaMask)) | value);
	}

	/*
	// there's no point in this, as it is actually the same as setAreaValue, after the buggy setAreaValue has 
	// been fixed. --jpk ;)
	void AreaMap::resetAreaValue(int x, int y, int areaMask, int value)
	{
		impl->areamap[x + y * impl->sizeX] = ((impl->areamap[x + y * impl->sizeX] & (~areaMask)) | value);
	}
	*/

	void AreaMap::fillAreaValue(int areaMask, int value)
	{
		unsigned int size = impl->sizeX * impl->sizeY;
		for(unsigned int i = 0; i < size; i++)
		{
			impl->areamap[i] = ((impl->areamap[i] & (~areaMask)) | value);
		}
	}

	bool AreaMap::isAreaAnyValue(int index, int areaMask) const
	{
		if ((impl->areamap[index] & areaMask) != 0)
			return true;
		else
			return false;
	}

	bool AreaMap::isAreaValue(int index, int areaMask, int value) const
	{
		if ((impl->areamap[index] & areaMask) == value)
			return true;
		else
			return false;
	}

	AREAMAP_DATATYPE *AreaMap::getInternalBuffer()
	{
		return impl->areamap;
	}


}

