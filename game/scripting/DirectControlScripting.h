
#ifndef DIRECTCONTROLSCRIPTING_H
#define DIRECTCONTROLSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class DirectControlScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void DirectControlScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


