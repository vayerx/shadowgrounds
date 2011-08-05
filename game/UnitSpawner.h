
#ifndef UNITSPAWNER_H
#define UNITSPAWNER_H

namespace game
{
	class Game;
	class Unit;
	class UnitType;

	class UnitSpawner
	{
		public:
			/**
			 * Spawns an unit into combat. (The unit must have already been
			 * created, this just spawns them into the combat field -
			 * or in other words activates the unit.)
			 * The created unit should have been placed to appropriate
			 * unitlist before this call.
			 * If the unit has already been spawned, will first retire it 
			 * and then respawn it.
			 */
			static void spawnUnit(Game *game, Unit *unit);

			/**
			 * Retires a unit from the battlefield. 
			 * In case of a computer opponent owned unit, just dispose
			 * of it. Human player controlled units will be kept.
			 * (May affect unit's parts and stuff - some things still TODO)
			 */
			static void retireUnit(Game *game, Unit *unit);		

			/**
			 * Retires a unit from the battlefield and deletes it,
			 * regardless of whether it is a human or computer opponent owned unit.
			 * (Call this before quitting to make sure player units
			 * get properly destroyed)
			 */
			static void deleteUnit(Game *game, Unit *unit);		

			static void markAliveHostileUnits(Game *game);

			static void respawnMarkedHostileUnitsAlive(Game *game);

			// for recycling the destroyed units into new spawns...

			// returns null if no recyclable units found
			static Unit *findReusableUnit(Game *game, UnitType *unitType, int playerSide);

			// effectively, retires the unit, then spawns it back.
			static void reuseUnit(Game *game, Unit *unit);

		private:
			// a hack to get respawning work for computer units...
		  static bool spawner_dont_delete_unit;

			// HACK: ...
			friend class UnitScripting;
			friend class CharacterSelectionWindow;
	};

}

#endif

