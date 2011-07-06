
#ifndef SYNCSCRIPTING_H
#define SYNCSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class SyncScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void SyncScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


