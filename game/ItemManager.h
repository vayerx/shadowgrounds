
#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <DatatypeDef.h>

class LinkedList;

namespace game
{
	class Game;
	class Item;
	class ItemType;
	class IItemListener;
	class Unit;

	struct ItemSpawnerGroup;

	struct ItemSpawner
	{
		int item_id;
		std::string item_name;
		// time in ticks to wait until respawn
		int respawn_time;
		// gameTimer value for next spawn
		int next_respawn;
		// last spawned item (if any)
		Item *spawned_item;
		VC3 position;
		float radius;
		ItemSpawnerGroup *group;
	};

	struct ItemSpawnerGroup
	{
		std::string name;
		bool active;
		// all spawners in this group
		std::vector<ItemSpawner *> spawners;
		// spawners currently waiting to spawn
		std::vector<ItemSpawner *> spawnQueue;
	};

	class ItemManager
	{
		public:
			ItemManager(Game *game);

			~ItemManager();
			
			// added by Pete
			void setListener( IItemListener* listener );

			ItemType *getItemTypeById(int itemId);

			Item *createNewItem(int itemId, const VC3 &position);

			void deleteItem(Item *i);

			// added by Pete
			void enablePhysics( Item* item, int itemType );

			void changeItemVisual(Item *i, const char *modelFilename);

			static int getItemIdByName(const char *itemname);

			void run();

			void prepareForRender();

			Item *getNearestItemOfType(const VC3 &position, ItemType *itemType);

			void disableItem(Item *i, int disableTime);

			//void setExecuteUnit(Unit *unit);

			bool doItemExecute(Unit *unit);

			// permanently delete all items that have been disabled for more than 24 hours
			// (those items are actully "deleted" but have a possibility for respawn if
			// player should die and respawn)
			void deleteAllLongTimeDisabledItems();

			// re-enable ("respawn") all time disabled items. permanently disabled won't
			// be re-enabled
			void reEnableAllTimeDisabledItems();

		public:
			ItemSpawner *createSpawner(const char *group_name, int item_id, const std::string &item_name, int respawn_time, const VC3 &position, float radius);
			void clearAllSpawners(void);

			ItemSpawnerGroup *getSpawnerGroup(const char *group_name);
			ItemSpawnerGroup *createSpawnerGroup(const char *group_name);

			void setSpawnerGroupActive(ItemSpawnerGroup *group, bool active);

		private:
			static void loadItemTypes();

			static void unloadItemTypes();

			void queueForSpawn(ItemSpawner *spawner, bool instantly);
			void spawnItem(ItemSpawner *spawner);
			void runSpawners();

			Game *game;
			IItemListener* listener;
			//Unit *executeUnit;


			
			std::vector<ItemSpawnerGroup *> spawnerGroups;

	};
}

#endif


