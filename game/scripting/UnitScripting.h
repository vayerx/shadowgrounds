
#ifndef UNITSCRIPTING_H
#define UNITSCRIPTING_H

#include <DatatypeDef.h>

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;
	class Unit;

	class UnitScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, const char *stringData, ScriptLastValueType *lastValue,
				GameScriptData *gsd, Game *game, bool *pause);

			static Unit *findClosestHostileUnit(Game *game, VC3 &position, int player, Unit *ignore);
			static Unit *findClosestFriendlyUnit(Game *game, VC3 &position, int player, Unit *ignore);
			static Unit *findClosestOwnedUnit(Game *game, VC3 &position, int player, Unit *ignore);

			static Unit *findUnitByCharacterName(Game *game, const char *charname);
			static Unit *findClosestUnitOfType(Game *game, const VC3 &position, const char *unittype);

			static Unit *nextOwnedUnit(Game *game, const VC3 &position, int player, Unit *ignore, bool only_active = true );
			static Unit *randomOwnedUnit(Game *game, VC3 &position, int player);

			static Unit *findClosestUnitWithVariableSet(Game *game, const VC3 &position, const char *unitvar, bool varIsNonZero);
			static Unit *findClosestUnitOfTypeWithVariableSet(Game *game, const VC3 &position, const char *unittype, const char *unitvar, bool varIsNonZero);
	
			static bool findGroup(Game *game, Unit *unit);

			static void moveUnitToDirection(Unit *unit, int direction, 
				float amount);
	};
}

#endif


