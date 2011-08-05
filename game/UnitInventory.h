
#ifndef UNITINVENTORY_H
#define UNITINVENTORY_H


namespace game
{
	class Game;
	class Unit;
	class Item;

	class UnitInventory
	{
		public:
			// returns true on success, if an item was added
			static bool giveUnitItem(Game *game, Unit *unit, const char *itemName);

			// returns true on success, if an item was removed
			static bool removeUnitItem(Game *game, Unit *unit, const char *itemName);

			// returns true on success, if an item was used
			static bool useSelectedUnitItem(Game *game, Unit *unit);

			// returns true on success, if an item was used
			static bool useUnitItem(Game *game, Unit *unit, const char *itemName);

			// returns item count or zero if unit has no such item - or if item by that name does not exist (error)
			static int getUnitItemCount(Game *game, Unit *unit, const char *itemName);

			// returns items unit is holding
			static void getUnitItems(Game *game, Unit *unit, std::vector<std::string> &itemNames);

			// sets item count to given value (and removes/adds items if necessary, if zero/nonzero)
			static void setUnitItemCount(Game *game, Unit *unit, const char *itemName, int amount);

			// returns true on success
			static bool selectUnitItem(Game *game, Unit *unit, const char *itemName);

			// should always succeed, thus no return value)
			static void deselectUnitItem(Game *game, Unit *unit);

			// returns true if selected item is of given type name
			static bool isSelectedUnitItem(Game *game, Unit *unit, const char *itemName);

		private:
			static void useUnitItemImpl(Game *game, Unit *unit, int itemNumber);

	};

}


#endif

