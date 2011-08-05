
#ifndef LIGHTSCRIPTING_H
#define LIGHTSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	/** 
	 * Just the GameScripting commands that deal with lights.
	 * (To prevent the GameScripting growing totally enormous.)
	 */
	class LightScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif



