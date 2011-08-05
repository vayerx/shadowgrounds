
#ifndef CINEMATICSCRIPTING_H
#define CINEMATICSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class CinematicScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


