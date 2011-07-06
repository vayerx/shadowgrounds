
#ifndef ITEMSCRIPTING_H
#define ITEMSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;
	class Item;

	extern Item *itemScriptItem;

	class ItemScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void ItemScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game, bool *pause);
	};
}

#endif


