
#ifndef MAPSCRIPTING_H
#define MAPSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

#include <DatatypeDef.h>

namespace game
{
	class Game;
	class GameScriptData;
	class Unit;
	class Projectile;
	class Bullet;

	/** 
	 * Hit chain script commands
	 */
	class MapScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);

			static void applyPortals(Game *game);
	};
}

#endif



