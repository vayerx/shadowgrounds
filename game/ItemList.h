
#ifndef ITEMLIST_H
#define ITEMLIST_H

#include "GameObject.h"

class LinkedList;

namespace game
{
	class Item;

	/**
	 * A class holding game items.
	 * (Those collectable items that are currently in the map)
	 * 
	 * @version 1.0, 18.8.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see Item
	 * @see Game
	 */

	class ItemList : public GameObject
	{
	public:
		ItemList();
		~ItemList();

		virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

		int getAllItemAmount();

		LinkedList *getAllItems();

		void addItem(Item *item);
		void removeItem(Item *item);

	private:
		LinkedList *allItems;
	};

}

#endif

