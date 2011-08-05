
#ifndef PLAYERPARTSMANAGER_H
#define PLAYERPARTSMANAGER_H

namespace game
{
	class Game;
	class Part;

	class PlayerPartsManager
	{
	public:
		PlayerPartsManager(Game *game);

		~PlayerPartsManager();

		bool allowPartType(int player, char *partIdString);

		Part *addStoragePart(int player, char *partIdString);

	private:
		Game *game;
	};
}

#endif


