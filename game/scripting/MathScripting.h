
#ifndef MATHSCRIPTING_H
#define MATHSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class MathScripting
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

