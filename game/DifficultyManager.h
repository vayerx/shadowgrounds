
#ifndef DIFFICULTYMANAGER_H
#define DIFFICULTYMANAGER_H

namespace game
{
	class Game;

	class DifficultyManager
	{
		public:
			DifficultyManager(Game *game);

			~DifficultyManager();

			void run();

			float getPlayerDamageRatio();

			//int getDamageAmountLevel();

		private:
			Game *game;
			int ticksUntilUpdate;
			int damageAmountLevel;
	};

}

#endif

