
#ifndef ANIMATIONSCRIPTING_H
#define ANIMATIONSCRIPTING_H

#include <DatatypeDef.h>

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;
	class Unit;

	class AnimationScripting
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


