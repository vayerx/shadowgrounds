
#ifndef STRINGSCRIPTING_H
#define STRINGSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class StringScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void StringScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif

