
#ifndef PROGRESSBARACTOR_H
#define PROGRESSBARACTOR_H

namespace game
{
	class ProgressBar;
	class Game;
	class Item;
	class Unit;

	class ProgressBarActor
	{
		public:
			ProgressBarActor(Game *game, ProgressBar *progressBar, Item *item, Unit *unit);

			void run(const VC3 &playerPosition, float playerAngle);

		private:
			Game *game;
			ProgressBar *progressBar;
			Item *item;
			Unit *unit;

	};

}

#endif

