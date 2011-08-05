
#include "precompiled.h"

#include "ItemList.h"

#include "Item.h"
#include "gamedefs.h"
#include "../container/LinkedList.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{

	ItemList::ItemList()
	{
		allItems = new LinkedList();
	}

	// NOTE, does not delete the items inside this but just the list of them
	ItemList::~ItemList()
	{
		while (!allItems->isEmpty())
		{
			allItems->popLast();
		}
		delete allItems;
	}

	SaveData *ItemList::getSaveData() const
	{
		// TODO
		return NULL;
	}

	const char *ItemList::getStatusInfo() const
	{
		return "ItemList";
	}

	int ItemList::getAllItemAmount()
	{
		// TODO, optimize, not thread safe! (caller may not be using iterator)
		int count = 0;
		allItems->resetIterate();
		while (allItems->iterateAvailable())
		{
			allItems->iterateNext();
			count++;
		}
		return count;
	}

	LinkedList *ItemList::getAllItems()
	{
		return allItems;
	}

	void ItemList::addItem(Item *item)
	{
		allItems->append(item);
	}

	// does not delete the projectile, just removes it from the list
	void ItemList::removeItem(Item *item)
	{
		allItems->remove(item);
	}

}

