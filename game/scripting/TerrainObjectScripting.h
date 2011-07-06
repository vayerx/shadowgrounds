
#ifndef TERRAINOBJECTSCRIPTING_H
#define TERRAINOBJECTSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class TerrainObjectScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void TerrainObjectScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif


