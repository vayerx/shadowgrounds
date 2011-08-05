
#include "precompiled.h"

#include "AlienSpawner.h"

#include "Game.h"
#include "scripting/GameScripting.h"
#include "GameRandom.h"
#include "../system/Logger.h"

#define ALIENSPAWNER_MAX_SPAWNPOINTS 256
#define ALIENSPAWNER_MAX_SCRIPTS 32

#define ALIENSPAWNER_MIN_SPAWN_RANGE 20
#define ALIENSPAWNER_MAX_SPAWN_RANGE 40
#define ALIENSPAWNER_SPAWN_RANGE_RAND 5

namespace game
{
	
	class AlienSpawnerPointImpl
	{
	public:
		AlienSpawnerPointImpl()
		{
			position = VC3(0,0,0);
			spawnerScriptId = 0;
		}

		VC3 position;
		int spawnerScriptId;
	};



	class AlienSpawnerImpl
	{
	public:
		AlienSpawnerImpl(Game *game, int player)
		{
			this->game = game;
			this->player = player;

			reset();
		}

		~AlienSpawnerImpl()
		{
		}

		void reset()
		{
			this->currentTime = 0;
			this->spawnInterval = GAME_TICKS_PER_SECOND;
			this->enabled = false;
			this->nextSpawnDelay = 0;

			{
				for (int i = 0; i < ALIENSPAWNER_MAX_SCRIPTS; i++)
				{
					spawnerScripts[i] = "";
					scriptEnabled[i] = false;
				}
				spawnerScripts[0] = "_none_";
			}
			{
				for (int i = 0; i < ALIENSPAWNER_MAX_SCRIPTS; i++)
				{
					spawnPoint[i] = AlienSpawnerPointImpl();
				}
			}
		}

		void addSpawnPoint(const VC3 &position, const char *spawnerScript)
		{
			int scriptId = 0;

			// seek id for the spawner script string or assing a new id...
			{
				for (int i = 0; i < ALIENSPAWNER_MAX_SCRIPTS; i++)
				{
					if (strcmp(spawnerScripts[i].c_str(), spawnerScript) == 0)
					{
						scriptId = i;
						break;
					}
					if (spawnerScripts[i] == "")
					{
						spawnerScripts[i] = spawnerScript;
						scriptEnabled[i] = false;
						scriptId = i;
						break;
					}
				}
			}

			// add the given spawnpoint...
			if (scriptId != 0)
			{				
				bool foundPos = false;
				for (int i = 0; i < ALIENSPAWNER_MAX_SPAWNPOINTS; i++)
				{
					if (spawnPoint[i].spawnerScriptId == 0)
					{
						spawnPoint[i].spawnerScriptId = scriptId;
						spawnPoint[i].position = position;
						foundPos = true;
						break;
					}
				}
				if (!foundPos)
				{
					Logger::getInstance()->warning("AlienSpawnerImpl::addSpawnPoint - Too many spawnpoints added.");
				}
			} else {
				Logger::getInstance()->warning("AlienSpawnerImpl::addSpawnPoint - Too many different spawner scripts or invalid script parameter given.");
			}

			return;
		}

		void run(const VC3 &playerPosition)
		{
			// not enabled? then return
			if (!this->enabled)
				return;

			// still some manual delay set until next spawn? then return
			if (this->nextSpawnDelay > 0)
			{
				this->nextSpawnDelay--;
				return;
			}

			this->currentTime++;

			// is this a spawn interval tick? if not, then return
			if ((this->currentTime % this->spawnInterval) != 0)
				return;


			int closestId = -1;

#ifdef PROJECT_SURVIVOR
			
			// get random spawn point
			int temp_count = 0;
			int max_random = ALIENSPAWNER_MAX_SPAWNPOINTS;
			while( temp_count <= ALIENSPAWNER_MAX_SPAWNPOINTS && closestId == -1 )
			{
				temp_count++;
				int i = 0;
				if( i != 0 )
					i = game->gameRandom->nextInt() % max_random;

				if( spawnPoint[i].spawnerScriptId != 0 )
				{
					if( scriptEnabled[spawnPoint[i].spawnerScriptId] )
					{
						closestId = i;
					}
				}
				else
				{
					if( i < max_random )
						max_random = i;
				}
			}					
#else
			float closestDistSq = ALIENSPAWNER_MAX_SPAWN_RANGE * ALIENSPAWNER_MAX_SPAWN_RANGE;
			// TODO: more efficient datastructure
			for (int i = 0; i < ALIENSPAWNER_MAX_SPAWNPOINTS; i++)
			{
				if (spawnPoint[i].spawnerScriptId != 0)
				{
					if (scriptEnabled[spawnPoint[i].spawnerScriptId])
					{
						VC3 diffVec = spawnPoint[i].position - playerPosition;
						float distSq = diffVec.GetSquareLength();
						if (distSq < closestDistSq
							&& distSq > ALIENSPAWNER_MIN_SPAWN_RANGE * ALIENSPAWNER_MIN_SPAWN_RANGE)
						{
							closestId = i;
							closestDistSq = distSq;

							float d = sqrtf(closestDistSq) + ALIENSPAWNER_SPAWN_RANGE_RAND * float(game->gameRandom->nextInt() % 101) / 100.0f;
							closestDistSq = d * d;
						}
					}
				} else {
					break;
				}
			}
#endif
			if (closestId != -1)
			{
				game->gameScripting->runOtherScript(spawnerScripts[spawnPoint[closestId].spawnerScriptId].c_str(), "spawn", NULL, spawnPoint[closestId].position);
			}

		}

		void spawnRandomAt( const std::string& spawner_name )
		{
			std::vector< int > results;

			int scriptId = -1;
			// seek id for the spawner script string or assing a new id...
			{
				for (int i = 0; i < ALIENSPAWNER_MAX_SCRIPTS; i++)
				{
					if( spawnerScripts[i] == spawner_name )
					{
						scriptId = i;
						break;
					}
					if (spawnerScripts[i] == "")
					{
						break;
					}
				}
			}

			// find spawn points 
			if( scriptId != -1 )
			{
				for (int i = 0; i < ALIENSPAWNER_MAX_SPAWNPOINTS; i++)
				{
					if( spawnPoint[i].spawnerScriptId == scriptId )
					{
						results.push_back( i );
					}
				}

				if( results.empty() == false )
				{
					int random = game->gameRandom->nextInt() % results.size();
					int random_i = results[ random ];

					game->gameScripting->runOtherScript(spawnerScripts[scriptId].c_str(), "spawn", NULL, spawnPoint[random_i].position);
				}

			}
			else
			{
				// could not find a spawn point by the given name
			}

		}

		void setSpawnerScriptEnabled(const char *spawnerScript, bool enabled)
		{
			bool foundScript = false;
			for (int i = 0; i < ALIENSPAWNER_MAX_SCRIPTS; i++)
			{
				if (strcmp(spawnerScripts[i].c_str(), spawnerScript) == 0)
				{
					scriptEnabled[i] = enabled;
					foundScript = true;
					break;
				}
			}
			if (!foundScript)
			{
				Logger::getInstance()->warning("AlienSpawnerImpl::setSpawnerScriptEnabled - No given spawner scripts found.");
				Logger::getInstance()->debug(spawnerScript);
			}
		}

		void setSpawnRate(int msecInterval)
		{
			this->spawnInterval = msecInterval / GAME_TICK_MSEC;
			if (this->spawnInterval < 1)
			{
				this->spawnInterval = 1;
			}
		}

		void enable()
		{
			this->enabled = true;
		}

		void disable()
		{
			this->enabled = false;
		}

		void setNextSpawnDelay(int msecDelay)
		{
			this->nextSpawnDelay = msecDelay / GAME_TICK_MSEC;
		}

	private:
		Game *game;
		int player;

		bool enabled;

		std::string spawnerScripts[ALIENSPAWNER_MAX_SCRIPTS];
		bool scriptEnabled[ALIENSPAWNER_MAX_SCRIPTS];

		int spawnInterval;
		int currentTime;

		int nextSpawnDelay;

		AlienSpawnerPointImpl spawnPoint[ALIENSPAWNER_MAX_SPAWNPOINTS];

	};



	AlienSpawner::AlienSpawner(Game *game, int player)
	{
		this->impl = new AlienSpawnerImpl(game, player);
	}

	AlienSpawner::~AlienSpawner()
	{
		delete impl;
	}

	void AlienSpawner::addSpawnPoint(const VC3 &position, const char *spawnerScript)
	{
		impl->addSpawnPoint(position, spawnerScript);
	}

	void AlienSpawner::run(const VC3 &playerPosition)
	{
		impl->run(playerPosition);
	}

	void AlienSpawner::spawnRandomAt( const std::string& spawnName )
	{
		impl->spawnRandomAt( spawnName );
	}


	void AlienSpawner::enableSpawnerScript(const char *spawnerScript)
	{
		impl->setSpawnerScriptEnabled(spawnerScript, true);
	}

	void AlienSpawner::disableSpawnerScript(const char *spawnerScript)
	{
		impl->setSpawnerScriptEnabled(spawnerScript, false);
	}

	void AlienSpawner::setSpawnRate(int msecInterval)
	{
		impl->setSpawnRate(msecInterval);
	}

	void AlienSpawner::enable()
	{
		impl->enable();
	}

	void AlienSpawner::disable()
	{
		impl->disable();
	}

	void AlienSpawner::reset()
	{
		impl->reset();
	}

	void AlienSpawner::setNextSpawnDelay(int msecDelay)
	{
		impl->setNextSpawnDelay(msecDelay);
	}

}
