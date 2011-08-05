
#ifndef ALIENSPAWNER_H
#define ALIENSPAWNER_H

#include <DatatypeDef.h>

namespace game
{
	class Game;
	class AlienSpawnerImpl;

	class AlienSpawner
	{
	public:
		AlienSpawner(Game *game, int player);

		~AlienSpawner();

		void reset();

		void addSpawnPoint(const VC3 &position, const char *spawnerScript);

		void run(const VC3 &playerPosition);

		// added by Pete
		void spawnRandomAt( const std::string& spawnName );

		void enableSpawnerScript(const char *spawnerScript);
		void disableSpawnerScript(const char *spawnerScript);

		void setSpawnRate(int msecInterval);

		void enable();
		void disable();

		void setNextSpawnDelay(int msecInterval);

	private:
		AlienSpawnerImpl *impl;

	};
}

#endif

