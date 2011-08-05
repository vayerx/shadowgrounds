
#include "precompiled.h"

#include "UnitInventory.h"

#include "Unit.h"
#include "Game.h"
#include "scripting/GameScripting.h"
#include "Item.h"
#include "ItemType.h"
#include "ItemManager.h"
#include "../system/Logger.h"
#include <assert.h>


namespace game
{
	bool UnitInventory::giveUnitItem(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		int itemId = game->itemManager->getItemIdByName(itemName);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				assert(unit->getItem(num) != NULL);
				unit->getItem(num)->addCount();
			} else {
				Item *item = new Item(itemId);
				int freeSlot = 0;
				for (int i = 0; i < UNIT_MAX_ITEMS; i++)
				{
					if (unit->getItem(i) == NULL)
					{
						freeSlot = i;
						break;
					}
				}
				unit->addItem(freeSlot, item);
			}
			return true;
		} else {
			Logger::getInstance()->error("UnitInventory::giveUnitItem - giveUnitItem item by given name not found.");
			return false;
		}		
	}


	bool UnitInventory::removeUnitItem(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		int itemId = game->itemManager->getItemIdByName(itemName);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				assert(unit->getItem(num) != NULL);
				if (unit->getItem(num)->getCount() > 1)
				{
					unit->getItem(num)->decreaseCount();
				} else {
					Item *item = unit->removeItem(num);
					if (num == unit->getSelectedItem())
						unit->setSelectedItem(-1);
					if (item != NULL)
					{
						delete item;
					} else {
						Logger::getInstance()->warning("GameScripting::process - Unit item remove failed.");
						assert(!"UnitInventory::removeUnitItem - Unit item remove failed.");
					}
				}
				return true;
			}
		} else {
			Logger::getInstance()->error("UnitInventory::removeUnitItem - Item by given name not found.");
		}
		return false;
	}


	bool UnitInventory::useSelectedUnitItem(Game *game, Unit *unit)
	{
		assert(unit != NULL);

		if (unit->getSelectedItem() != -1)
		{
			// assuming the following call always succeeds (unless something is buggy)
			useUnitItemImpl(game, unit, unit->getSelectedItem());
			return true;
		} else {
			return false;
		}
	}


	bool UnitInventory::useUnitItem(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		int itemTypeId = game->itemManager->getItemIdByName(itemName);
		if (itemTypeId == -1)
		{
			Logger::getInstance()->warning("UnitInventory::useUnitItem - No item type found with given name.");
			return false;
		}

		int num = unit->getItemNumberByTypeId(itemTypeId);
		if (num != -1)
		{
			// assuming the following call always succeeds (unless something is buggy)
			useUnitItemImpl(game, unit, num);
			return true;
		} else {
			// unit did not have an item of that item type.
			return false;
		}
	}


	int UnitInventory::getUnitItemCount(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		int itemId = game->itemManager->getItemIdByName(itemName);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				assert(unit->getItem(num) != NULL);
				return unit->getItem(num)->getCount();
			}
		} else {
			Logger::getInstance()->error("UnitInventory::getUnitItemCount - Item by given name not found.");
		}

		return 0;
	}


	void UnitInventory::getUnitItems(Game *game, Unit *unit, std::vector<std::string> &itemNames)
	{
		assert(unit != NULL);
		for(unsigned int i = 0; i < UNIT_MAX_ITEMS; i++)
		{
			Item *item = unit->getItem(i);
			if(item != NULL)
			{
				ItemType *it = game->itemManager->getItemTypeById(item->getItemTypeId());
				if(it != NULL)
				{
					itemNames.push_back(it->getName());
				}
			}
		}
	}
	

	void UnitInventory::setUnitItemCount(Game *game, Unit *unit, const char *itemName, int amount)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		// TODO: proper implementation for this!!!
		int failcount = 0;
		while (true)
		{
			if (getUnitItemCount(game, unit, itemName) < amount)
			{
				giveUnitItem(game, unit, itemName);
			}
			else if (getUnitItemCount(game, unit, itemName) > amount)
			{
				removeUnitItem(game, unit, itemName);
			} else {
				break;
			}
			failcount++;
			if (failcount > 1000) 
			{
				Logger::getInstance()->error("UnitInventory::setUnitItemCount - Failed to set proper item count (internal error).");
				break;
			}
		}

		return;
	}

	
	bool UnitInventory::selectUnitItem(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		unit->setSelectedItem(-1);

		int itemId = game->itemManager->getItemIdByName(itemName);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				unit->setSelectedItem(num);
				return true;
			}
		} else {
			Logger::getInstance()->error("UnitInventory::selectUnitItem - Item by given name not found.");
		}
		return false;
	}


	bool UnitInventory::isSelectedUnitItem(Game *game, Unit *unit, const char *itemName)
	{
		assert(unit != NULL);
		assert(itemName != NULL);

		int itemId = game->itemManager->getItemIdByName(itemName);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				if (unit->getSelectedItem() == num)
				{
					return true;
				}
			}
		} else {
			Logger::getInstance()->error("UnitInventory::isSelectedUnitItem - Item by given name not found.");
		}
		return false;
	}


	void UnitInventory::deselectUnitItem(Game *game, Unit *unit)
	{
		unit->setSelectedItem(-1);
	}


	void UnitInventory::useUnitItemImpl(Game *game, Unit *unit, int itemNumber)
	{
		assert(itemNumber != -1);

		if (unit->getItem(itemNumber) != NULL)
		{
			if (unit->getItem(itemNumber)->getCount() > 1)
			{
				unit->getItem(itemNumber)->decreaseCount();
				Item *item = unit->getItem(itemNumber);
				game->gameScripting->runItemUseScript(unit, item);
			} else {
				Item *item = unit->removeItem(itemNumber);
				if (unit->getSelectedItem() == itemNumber)
					unit->setSelectedItem(-1);
				if (item != NULL)
				{
					game->gameScripting->runItemUseScript(unit, item);
					delete item;
				} else {
					Logger::getInstance()->warning("UnitInventory::useUnitItemImpl - Unit item remove failed.");
					assert(!"UnitInventory::useUnitItemImpl - Unit item remove failed.");
				}
			}
		} else {
			Logger::getInstance()->error("UnitInventory::useUnitItemImpl - Requested unit item was null.");
			assert(!"UnitInventory::useUnitItemImpl - Requested unit item was null.");
		}
	}


}

