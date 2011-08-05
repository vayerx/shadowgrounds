
#ifndef MISSIONSCRIPTING_H
#define MISSIONSCRIPTING_H

#include <DatatypeDef.h>

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class MissionScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);

			static int calculateAliveUnits(Game *game, int player);
			static int calculateConsciousUnits(Game *game, int player);
			static int calculateAliveHostileUnits(Game *game, int player);

			static int countHostilesInRange(Game *game, const VC3 &position, int player, int range, bool consciousOnly);
			static int countFriendlysInRange(Game *game, const VC3 &position, int player, int range);
			static int countOwnedInRange(Game *game, const VC3 &position, int player, int range);

	    static bool isEveryUnitNearPosition(Game *game, int player, float range, const VC3 &position);
			static bool isAnyUnitNearPosition(Game *game, int player, float range, const VC3 &position);

			static bool playerUnitsInAction(Game *game, int player);

	};
}

#endif


