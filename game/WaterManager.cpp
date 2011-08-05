
#include "precompiled.h"

#include <assert.h>
#include "../ui/VisualObject.h"
#include "../ui/VisualObjectModel.h"

#include "../ui/Decoration.h"

#include "Water.h"
#include "WaterManager.h"

#include "../container/LinkedList.h"

namespace game
{

WaterManager::WaterManager() 
{

   waterList = new LinkedList();
   highest = 0.0f;

}

WaterManager::~WaterManager() 
{

	while (!waterList->isEmpty())
	{
		Water *w = (Water *)waterList->popLast();
		delete w;
	}
	
	delete waterList;
	
}

Water *WaterManager::createWater() 
{

	Water* water = new Water();
	
	waterList->append(water);

	return water;

}
       
void WaterManager::deleteWater(Water *water) 
{

	/*
	LinkedListIterator iter = LinkedListIterator(waterList);
	while (iter.iterateAvailable())
	{
		Water *w = (Water *)iter.iterateNext();
		if(w == water) {
			waterList->remove(w);
			delete w;
			return;
		}
	}
	*/
	waterList->remove(water);
	delete water;
	
}

Water *WaterManager::getWaterByName(const char *name) const 
{

	LinkedListIterator iter = LinkedListIterator(waterList);
	while (iter.iterateAvailable())
	{
		Water *water = (Water *)iter.iterateNext();
		if (water->name != NULL
			&& strcmp(water->name, name) == 0)
		{
			return water;
		}
	}
	return NULL;
}

float WaterManager::getWaterDepthAt(const VC3 &position) const 
{

	// optimization, only if the point is below hihghest water plane
	// proceed with the test
	if(position.y > highest) {
		return 0.0f;
	}
	
	LinkedListIterator iter = LinkedListIterator(waterList);
	while (iter.iterateAvailable())
	{
		Water *water = (Water *)iter.iterateNext();
		// if this point is in the are of this water?
		if((position.y > water->minX) && (position.x < water->maxX) 
			&& (position.z > water->minY) && (position.z < water->maxY)) 
		{
			
			// return the depth how much this point is below water plane
			// zero, if above water
			float d = 0.0f;
			if(water->height > position.y) 
			{
				d = water->height - position.y;
			}
			return d;

		}
	
	}

	// not in water at all.
	return 0.0f;
}

void WaterManager::recalculate() 
{

	// shoould we remove all updateBoundaries from Water class methods
	// and call it simple here for each Water?
	
	// recalculate highest

	LinkedListIterator iter = LinkedListIterator(waterList);
	if(iter.iterateAvailable()) {
		Water *water = (Water *)iter.iterateNext();
		highest = water->height;
	}

	while (iter.iterateAvailable())
	{
		Water *water = (Water *)iter.iterateNext();
		if(water->height > highest) {
			highest = water->height;
		}	
	}

}


} // ui


