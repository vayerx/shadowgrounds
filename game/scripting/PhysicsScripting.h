
#ifndef PHYSICSSCRIPTING_H
#define PHYSICSSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class Game;
	class GameScriptData;

	class PhysicsScripting
	{
		public:			
			static void PhysicsScripting::process(util::ScriptProcess *sp, 
				int command, int intData, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif
