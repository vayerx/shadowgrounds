
#ifndef ENVIRONMENTSCRIPTING_H
#define ENVIRONMENTSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	/** 
	 * Just the GameScripting commands that deal with decorations and
	 * water seperated to a different file.
	 * (To prevent the GameScripting growing totally enormous.)
	 */
	class EnvironmentScripting
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



