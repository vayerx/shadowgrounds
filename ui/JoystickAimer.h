
#ifndef JOYSTICKAIMER_H
#define JOYSTICKAIMER_H

class Ogui;

namespace game
{
	class Unit;
	class Game;
}

namespace ui
{
	class GameController;

  class JoystickAimer
	{
		public:
			JoystickAimer(game::Unit *unit, Ogui *ogui, 
				game::Game *game, GameController *gameController,
				int clientNumber);

			void aimWithJoystick(int timeElapsed);

		private:
			float currentAimingAngle;
			GameController *gameController;
			game::Unit *unit;
			game::Game *game;
			Ogui *ogui;
			int clientNumber;
	};
}

#endif

