
#ifndef UPGRADEMANAGER_H
#define UPGRADEMANAGER_H

#include <DatatypeDef.h>
#include <vector>

class LinkedList;

namespace game
{
	class Game;
	class UpgradeType;
	class Unit;
	class Item;

	class UpgradeManager
	{
		public:
			UpgradeManager(Game *game);

			~UpgradeManager();

			UpgradeType *getUpgradeTypeById(int upgradeId);

			// NOTE: costPending (in/out) is the amount of "money" allocated for pending 
			// upgrades. this function uses that for checking that enough "money" is
			// available, and changes the value based on the upgrade part's cost
			// if it can be upgraded. Give NULL for ignoring the pending upgrade costs.
			bool canAfford(Unit *unit, int upgradeId, int *costPending = 0);

			// NOTE: see the above canAfford note.
			bool canUpgrade(Unit *unit, int upgradeId, int *costPending = 0);

			bool isUpgraded(Unit *unit, int upgradeId);

			bool isAvailableUpgrades(Unit *unit);

			// returns the amount of upgrades that have been bought (NOT the total cost, but the amount)
			int getUpgradedAmount(Unit *unit);

			void upgrade(Unit *unit, int upgradeId);

			//void downgrade(Unit *unit, int upgradeId);

			void reapplyAllUpgrades(Unit *unit = NULL);

			void unapplyAll(Unit *unit = NULL);

			void downgradeAll(Unit *unit = NULL);

			int getUpgradePartsAmount(Unit *unit);

			void getUpgradesForPart(const char *part, std::vector<int> &upgradeIds) const;

			// added by Pete
			int getUpgradePartCost( int upgradeId );

			/*
			// NOT TO BE USED ANYMORE (as names are no-longer unique)
			static int getUpgradeIdByName(const char *upgradename);
			*/

			static int getUpgradeIdByScript(const char *scriptname);

			bool runStartPendingScript(Unit *unit, int upgradeId);
			bool runStopPendingScript(Unit *unit, int upgradeId);

			bool isLocked(Unit *unit, int upgradeId);

		private:
			Item *getUnitUpgradeItem(Unit *unit, int *itemNumber);

			static void loadUpgradeTypes();

			static void unloadUpgradeTypes();


		// character parts
		//
		public:
			bool isCharacterUpgrade(int upgradeId);
			int getCharacterPartsAmount(Unit *unit);
		private:
			Item *getUnitCharacterPartItem(Unit *unit, int *itemNumber);


		private:

			Game *game;
	};
}

#endif


