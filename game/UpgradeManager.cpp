
#include "precompiled.h"

#include "UpgradeManager.h"

#include "Game.h"
#include "scripting/GameScripting.h"
#include "UpgradeType.h"
#include "ItemManager.h"
#include "Item.h"
#include "ItemType.h"
#include "../convert/str2int.h"
#include "../util/SimpleParser.h"
#include "../util/ScriptManager.h"
#include "../system/Logger.h"

#define MAX_UPGRADE_TYPES 128


namespace game
{
	UpgradeType *upgradeTypes = NULL;


	UpgradeManager::UpgradeManager(Game *game)
	{
		this->game = game;

		if (upgradeTypes == NULL)
			loadUpgradeTypes();
	}


	UpgradeManager::~UpgradeManager()
	{
		assert(upgradeTypes != NULL);

		unloadUpgradeTypes();
	}


	Item *UpgradeManager::getUnitUpgradeItem(Unit *unit, int *itemNumber)
	{
		int itemId = game->itemManager->getItemIdByName("upgradepart");
		assert(itemId != -1);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				if (itemNumber != NULL)
					*itemNumber = num;
				return unit->getItem(num);
			}
		}
		return NULL;
	}


	int UpgradeManager::getUpgradePartsAmount(Unit *unit)
	{
		int num = 0;
		Item *item = getUnitUpgradeItem(unit, &num);

		if (item != NULL)
		{
			return item->getCount();
		} else {
			return 0;
		}
	}


	bool UpgradeManager::canAfford(Unit *unit, int upgradeId, int *costPending)
	{
		int num = 0;
		Item *item;
		if(isCharacterUpgrade(upgradeId)) item = getUnitCharacterPartItem(unit, &num);
		else item = getUnitUpgradeItem(unit, &num);

		if (item != NULL)
		{
			int tmp = 0;
			if (costPending != NULL)
				tmp += *costPending;

			if (item->getCount() - tmp >= upgradeTypes[upgradeId].getCost())
			{
				if (costPending != NULL)
					*costPending += upgradeTypes[upgradeId].getCost();

				return true;
			}
		}

		return false;
	}


	bool UpgradeManager::canUpgrade(Unit *unit, int upgradeId, int *costPending)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		int tmp = 0;
		if (costPending != NULL)
			tmp = *costPending;

		if (!canAfford(unit, upgradeId, &tmp))
			return false;

		int ret = game->gameScripting->runOtherScript(upgradeTypes[upgradeId].getScript(), "can_upgrade", unit, VC3(0,0,0));

		if (ret != 0)
		{
			if (costPending != NULL)
				*costPending = tmp;

			return true;
		} else {
			return false;
		}
	}


	bool UpgradeManager::isUpgraded(Unit *unit, int upgradeId)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		int ret = game->gameScripting->runOtherScript(upgradeTypes[upgradeId].getScript(), "is_upgraded", unit, VC3(0,0,0));

		if (ret != 0)
			return true;
		else
			return false;
	}


	int UpgradeManager::getUpgradedAmount(Unit *unit)
	{
		int upgAmount = 0;
		for (int i = 0; i < MAX_UPGRADE_TYPES; i++)
		{
			int ret = game->gameScripting->runOtherScript(upgradeTypes[i].getScript(), "is_upgraded", unit, VC3(0,0,0));

			if (ret != 0)
				upgAmount++;
		}
		return upgAmount;
	}


	void UpgradeManager::upgrade(Unit *unit, int upgradeId)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		if (!canUpgrade(unit, upgradeId))
		{
			assert(!"UpgradeManager::upgrade - Upgrade called even though it cannot be done?");
			return;
		}

		int num = 0;
		Item *item;
		if(isCharacterUpgrade(upgradeId)) item = getUnitCharacterPartItem(unit, &num);
		else item = getUnitUpgradeItem(unit, &num);

		assert(item != NULL);

		if (item->getCount() > upgradeTypes[upgradeId].getCost())
		{
			item->setCount(item->getCount() - upgradeTypes[upgradeId].getCost());
		} else {
			Item *item = unit->removeItem(num);
			if (item != NULL)
			{
				delete item;
			} else {
				Logger::getInstance()->warning("Unit upgradepart item remove failed.");
				assert(!"Unit upgradepart item remove failed.");
			}
		}

		game->gameScripting->runOtherScript(upgradeTypes[upgradeId].getScript(), "upgrade", unit, VC3(0,0,0));
	}

	void UpgradeManager::reapplyAllUpgrades(Unit *unit)
	{
		game->gameScripting->runOtherScript("upgrader", "reapply_all", unit, VC3(0,0,0));
	}

	void UpgradeManager::unapplyAll(Unit *unit)
	{
		game->gameScripting->runOtherScript("upgrader", "unapply_all", unit, VC3(0,0,0));
	}

	void UpgradeManager::downgradeAll(Unit *unit)
	{
		game->gameScripting->runOtherScript("upgrader", "downgrade_all", unit, VC3(0,0,0));
	}


	UpgradeType *UpgradeManager::getUpgradeTypeById(int upgradeId)
	{
		if (upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES)
		{
			return &upgradeTypes[upgradeId];
		} else {
			Logger::getInstance()->error("UpgradeManager::getUpgradeTypeById - Upgrade id number out of range.");
			assert(0);
			return NULL;
		}		
	}


	// NOT TO BE USED ANYMORE (as names are no-longer unique)
	/*
	int UpgradeManager::getUpgradeIdByName(const char *upgradename)
	{
		assert(upgradename != NULL);

		if (upgradeTypes == NULL)
			loadUpgradeTypes();		

		// TODO: this is not very effective, but on the other hand
		// this is not meant to be called very often.
		for (int i = 0; i < MAX_UPGRADE_TYPES; i++)
		{
			if (upgradeTypes[i].getName() != NULL
				&& strcmp(upgradeTypes[i].getName(), upgradename) == 0)
				return i;
		}

		Logger::getInstance()->warning("UpgradeManager::getUpgradeIdByName - No upgrade type with given name.");
		Logger::getInstance()->debug(upgradename);

		return -1;
	}
	*/


	int UpgradeManager::getUpgradeIdByScript(const char *scriptname)
	{
		assert(scriptname != NULL);

		if (upgradeTypes == NULL)
			loadUpgradeTypes();		

		// TODO: this is not very effective, but on the other hand
		// this is not meant to be called very often.
		for (int i = 0; i < MAX_UPGRADE_TYPES; i++)
		{
			if (upgradeTypes[i].getScript() != NULL
				&& strcmp(upgradeTypes[i].getScript(), scriptname) == 0)
				return i;
		}

		Logger::getInstance()->warning("UpgradeManager::getUpgradeIdByScript - No upgrade type with given script.");
		Logger::getInstance()->debug(scriptname);

		return -1;
	}


	void UpgradeManager::loadUpgradeTypes()
	{
		// NOTE: not thread safe.

		Logger::getInstance()->debug("UpgradeManager::loadUpgradeTypes - About to load upgrade types.");

		// read in upgrade types
		upgradeTypes = new UpgradeType[MAX_UPGRADE_TYPES];
		util::SimpleParser sp;
#ifdef LEGACY_FILES
		if (sp.loadFile("Data/Items/upgrades.txt"))
#else
		if (sp.loadFile("data/item/upgrades.txt"))
#endif
		{
			int atId = 0;
			bool insideUpgrade = false;
			while (sp.next())
			{
				bool lineok = false;
				char *k = sp.getKey();
				if (k != NULL)
				{
					if (!insideUpgrade)
						sp.error("UpgradeManager::loadUpgradeTypes - Parse error, key=value pair outside upgrade block.");
					char *v = sp.getValue();
					// treat empty lines as null...
					if (v[0] == '\0') v = NULL;

					if (strcmp(k, "name") == 0)
					{
						// NOTE: upgrade names are no longer unique
						/*
						if (v != NULL)
						{
							for (int i = 0; i < atId; i++)
							{
								if (upgradeTypes[i].getName() != NULL
								  && strcmp(v, upgradeTypes[i].getName()) == 0)
								{
									sp.error("UpgradeManager::loadUpgradeTypes - Duplicate upgrade name.");
									break;
								}
							}
						}
						*/
						upgradeTypes[atId].setName(v);
						lineok = true;
					}
					if (strcmp(k, "script") == 0)
					{
						if (v != NULL)
						{
							for (int i = 0; i < atId; i++)
							{
								if (upgradeTypes[i].getScript() != NULL
								  && strcmp(v, upgradeTypes[i].getScript()) == 0)
								{
									sp.error("UpgradeManager::loadUpgradeTypes - Duplicate upgrade script.");
									break;
								}
							}
						}
						upgradeTypes[atId].setScript(v);
						lineok = true;
					}
					if (strcmp(k, "desc") == 0)
					{
						upgradeTypes[atId].setDescription(v);
						lineok = true;
					}
					if (strcmp(k, "part") == 0)
					{
						upgradeTypes[atId].setPart(v);
						lineok = true;
					}
					if (strcmp(k, "cost") == 0)
					{
						if (v != NULL)
						{
							upgradeTypes[atId].setCost(str2int(v));
							if (str2int_errno() != 0)
							{
								sp.error("UpgradeManager::loadUpgradeTypes - Cost parameter invalid, integer number expected.");
							}
						} else {
							sp.error("UpgradeManager::loadUpgradeTypes - Cost parameter missing.");
						}
						lineok = true;
					}
					
				} else {
					char *l = sp.getLine();
					if (strcmp(l, "upgrade") == 0)
					{
						if (insideUpgrade)
							sp.error("UpgradeManager::loadUpgradeTypes - Parse error, } expected.");
						if (atId < MAX_UPGRADE_TYPES - 1)
						{
							atId++;
						} else {
							sp.error("UpgradeManager::loadUpgradeTypes - Too many upgrades, limit reached.");
						}
						lineok = true;
					}
					if (strcmp(l, "{") == 0)
					{
						if (insideUpgrade)
							sp.error("UpgradeManager::loadUpgradeTypes - Parse error, unexpected {.");
						insideUpgrade = true;
						lineok = true;
					}
					if (strcmp(l, "}") == 0)
					{
						if (!insideUpgrade)
							sp.error("UpgradeManager::loadUpgradeTypes - Parse error, unexpected }.");
						insideUpgrade = false;
						lineok = true;
					}
				}
				if (!lineok)
				{
					sp.error("UpgradeManager::loadUpgradeTypes - Unknown command or bad key/value pair.");
				}
			}				
		}	else {
			Logger::getInstance()->error("UpgradeManager::loadUpgradeTypes - Failed to load upgrade types.");
		}		
	}


	void UpgradeManager::unloadUpgradeTypes()
	{
		delete [] upgradeTypes;
		upgradeTypes = NULL;
	}


	void UpgradeManager::getUpgradesForPart(const char *part, std::vector<int> &upgradeIds) const
	{
		for (int i = 0; i < MAX_UPGRADE_TYPES; i++)
		{
			if (part == NULL)
			{
				if (upgradeTypes[i].getPart() == NULL)
					upgradeIds.push_back(i);
			} else {
				if (upgradeTypes[i].getPart() != NULL
					&& strcmp(upgradeTypes[i].getPart(), part) == 0)
				{
					upgradeIds.push_back(i);
				}
			}
		}		
	}


	int UpgradeManager::getUpgradePartCost( int upgradeId )
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);

		return upgradeTypes[upgradeId].getCost();
	}


	bool UpgradeManager::isAvailableUpgrades(Unit *unit)
	{
		int availAmount = 0;
		for (int i = 0; i < MAX_UPGRADE_TYPES; i++)
		{
			if (upgradeTypes[i].getCost() != -1)
			{
				// no free upgrades should exist?
				assert(upgradeTypes[i].getCost() != 0);

				if (this->canUpgrade(unit, i))
				{
					availAmount++;
				}
			}
		}

		if (availAmount > 0)
			return true;
		else
			return false;
	}

	/////////////////////////////////////////////////////////////////////////
	// character parts
	//

	Item *UpgradeManager::getUnitCharacterPartItem(Unit *unit, int *itemNumber)
	{
		int itemId = game->itemManager->getItemIdByName("characterpart");
		assert(itemId != -1);
		if (itemId != -1)
		{
			int num = unit->getItemNumberByTypeId(itemId);
			if (num != -1)
			{
				if (itemNumber != NULL)
					*itemNumber = num;
				return unit->getItem(num);
			}
		}
		return NULL;
	}

	int UpgradeManager::getCharacterPartsAmount(Unit *unit)
	{
		int num = 0;
		Item *item = getUnitCharacterPartItem(unit, &num);

		if (item != NULL)
		{
			return item->getCount();
		} else {
			return 0;
		}
	}

	bool UpgradeManager::isCharacterUpgrade( int upgradeId )
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		const char *part = upgradeTypes[upgradeId].getPart();
		if(strcmp(part, "surv_napalm") == 0 ||
			strcmp(part, "surv_marine") == 0 ||
			strcmp(part, "surv_sniper") == 0) return true;
		return false;
	}

	bool UpgradeManager::runStartPendingScript(Unit *unit, int upgradeId)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		const char *script = upgradeTypes[upgradeId].getScript();
		const char *sub = "start_pending";
		util::Script *s = util::ScriptManager::getInstance()->getScript(script);
		if (s == NULL)
			return false;
		if (!s->hasSub(sub))
			return false;

		return 1 == game->gameScripting->runOtherScript(script, sub, unit, VC3(0,0,0));
	}

	bool UpgradeManager::runStopPendingScript(Unit *unit, int upgradeId)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		const char *script = upgradeTypes[upgradeId].getScript();
		const char *sub = "stop_pending";
		util::Script *s = util::ScriptManager::getInstance()->getScript(script);
		if (s == NULL)
			return false;
		if (!s->hasSub(sub))
			return false;

		return 1 == game->gameScripting->runOtherScript(script, sub, unit, VC3(0,0,0));
	}

	bool UpgradeManager::isLocked(Unit *unit, int upgradeId)
	{
		assert(upgradeId >= 0 && upgradeId < MAX_UPGRADE_TYPES);
		assert(unit != NULL);

		const char *script = upgradeTypes[upgradeId].getScript();
		const char *sub = "is_locked";
		util::Script *s = util::ScriptManager::getInstance()->getScript(script);
		if (s == NULL)
			return false;
		if (!s->hasSub(sub))
			return false;

		return 1 == game->gameScripting->runOtherScript(script, sub, unit, VC3(0,0,0));
	}
}

