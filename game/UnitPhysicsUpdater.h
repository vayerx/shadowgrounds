
#ifndef UNITPHYSICSUPDATER_H
#define UNITPHYSICSUPDATER_H

namespace game
{
	class Unit;
	class GamePhysics;
	class Game;

	class UnitPhysicsUpdater
	{
	public:
		static void createPhysics(Unit *unit, GamePhysics *physics, Game *game);
		static void deletePhysics(Unit *unit, GamePhysics *physics);

		static void updatePhysics(Unit *unit, GamePhysics *physics, int gameTickTimer);

		static void startStats();
		static void endStats();

		static char *getStatusInfo();
		static void deleteStatusInfo(char *buf);
	};
}

#endif
