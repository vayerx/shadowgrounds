
#ifndef POSITIONSCRIPTING_H
#define POSITIONSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class PositionScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void PositionScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif


