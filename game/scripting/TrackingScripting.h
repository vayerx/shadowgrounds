
#ifndef TRACKINGSCRIPTING_H
#define TRACKINGSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class TrackingScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


