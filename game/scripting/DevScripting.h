
#ifndef DEVSCRIPTING_H
#define DEVSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class DevScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void DevScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


