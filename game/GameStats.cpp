
#include "precompiled.h"

#include <string>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "GameStats.h"
#include "Game.h"
#include "GameUI.h"
#include "PlayerWeaponry.h"
#include "UpgradeManager.h"
#include "Unit.h"
#include "Weapon.h"
#include "gamedefs.h"
#include "../convert/str2int.h"
#include "SimpleOptions.h"
#include "options/options_game.h"
#include "../system/Timer.h"
#include "../util/fb_assert.h"
#include "GameProfiles.h"
#include "../game/scripting/GameScripting.h"

// sample every 15 seconds 
#define GAMESTATS_RATE 15

// max 480 entries (480 x 15 secs = max 2 hours stats per mission)
#define GAMESTATS_MAX_ENTRIES 480

// max 15 different enemies
#define GAMESTATS_MAX_ENEMIES 15

// max 16 different weapon ammos
// (note, should preferrably equal to PLAYERWEAPONRY_MAX_WEAPONS)
#define GAMESTATS_MAX_AMMO 16

namespace game
{
	static std::string requestedScoreFile;

	class GameStatsImpl
	{
		public:
			std::string enemyNames[GAMESTATS_MAX_ENEMIES];

			int kills[GAMESTATS_MAX_ENEMIES][GAMESTATS_MAX_ENTRIES];
			int ammo[GAMESTATS_MAX_AMMO][GAMESTATS_MAX_ENTRIES];
			int deaths[GAMESTATS_MAX_ENTRIES];
			int health[GAMESTATS_MAX_ENTRIES];
			int upgrades[GAMESTATS_MAX_ENTRIES];
#ifdef PROJECT_SURVIVOR
			int charparts[GAMESTATS_MAX_ENTRIES];
			int exp[GAMESTATS_MAX_ENTRIES];
#endif
			int boughtUpgrades[GAMESTATS_MAX_ENTRIES];
			std::string marker[GAMESTATS_MAX_ENTRIES];
			std::string pickup[GAMESTATS_MAX_ENTRIES];
			std::string maplayer[GAMESTATS_MAX_ENTRIES];

			int currentEntry;
			int lastCollectTime;
			int timeOfDeath;

			int playerNum;
			Game *game;

			std::string missionid;

			bool running;


			void start(const char *missionid)
			{
				{
					for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
					{
						enemyNames[i] = "";
						for (int e = 0; e < GAMESTATS_MAX_ENTRIES; e++)
						{
							kills[i][e] = 0;
						}
					}
				}
				{
					for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
					{
						for (int e = 0; e < GAMESTATS_MAX_ENTRIES; e++)
						{
							ammo[i][e] = 0;
						}
					}
				}
				{
					for (int e = 0; e < GAMESTATS_MAX_ENTRIES; e++)
					{
						deaths[e] = 0;
						marker[e] = "";
						pickup[e] = "";
						maplayer[e] = "";
						health[e] = 0;
						upgrades[e] = 0;
#ifdef PROJECT_SURVIVOR
						charparts[e] = 0;
						exp[e] = 0;
#endif
						boughtUpgrades[e] = 0;
					}
				}
				currentEntry = 0;
				lastCollectTime = 0;
				timeOfDeath = -1;

				this->missionid = missionid;

				running = true;
			}


			void addKill(const char *enemy)
			{
				if (enemy == NULL)
				{
					fb_assert(!"addKill - null enemy parameter given.");
					return;
				}

				for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
				{
					if (strcmp(enemyNames[i].c_str(), enemy) == 0)
					{
						kills[i][currentEntry]++;
						break;
					}
					if (enemyNames[i] == "")
					{
						enemyNames[i] = std::string(enemy);
						kills[i][currentEntry] = 1;
						break;
					}
				}
			}


			void addDeath()
			{
				deaths[currentEntry]++;
			}

			void markTimeOfDeath()
			{
				if(timeOfDeath == -1)
					timeOfDeath = game->gameTimer;
			}

			void addMapChange(const char *maplayer)
			{
				this->maplayer[currentEntry] = std::string(maplayer);
			}


			void addMarker(const char *marker)
			{
				// last marker may not be set... (it is reserved for "stats max time")
				if (currentEntry < GAMESTATS_MAX_ENTRIES - 1)
				{
					this->marker[currentEntry] = std::string(marker);
				}
			}

			void addPickup(const char *pickup)
			{
				// last pickups ignored... (or they would totally "flood" the last entry)
				if (currentEntry < GAMESTATS_MAX_ENTRIES - 1)
				{
					if (this->pickup[currentEntry] == "")
					{
						this->pickup[currentEntry] = "";
						this->pickup[currentEntry] += std::string(pickup);
					} else {
						this->pickup[currentEntry] += "+";
						this->pickup[currentEntry] += std::string(pickup);
					}
				}
			}


			void runCollect()
			{
				if (!running)
					return;

				if (!game->inCombat)
					return;

				if (!SimpleOptions::getBool(DH_OPT_B_COLLECT_GAME_STATS))
					return;

				if (Timer::getTime() / 1000 < this->lastCollectTime + GAMESTATS_RATE)
				{
					return;
				}

				Unit *u = game->gameUI->getFirstPerson(playerNum);

				if (u != NULL)
				{
					this->health[currentEntry] = u->getHP();
				} else {
					this->health[currentEntry] = 0;
				}

				if (u != NULL)
				{
					this->upgrades[currentEntry] = game->upgradeManager->getUpgradePartsAmount(u);
#ifdef PROJECT_SURVIVOR
					this->charparts[currentEntry] = game->upgradeManager->getCharacterPartsAmount(u);
					this->exp[currentEntry] = game->gameScripting->getGlobalIntVariableValue(("survivor_current_expo[" + boost::lexical_cast<std::string>(playerNum) + "]").c_str());
#endif
				} else {
					this->upgrades[currentEntry] = 0;
#ifdef PROJECT_SURVIVOR
					this->charparts[currentEntry] = 0;
					this->exp[currentEntry] = 0;
#endif
				}

				if (u != NULL)
				{
					this->boughtUpgrades[currentEntry] = game->upgradeManager->getUpgradedAmount(u);
				} else {
					this->boughtUpgrades[currentEntry] = 0;
				}

				if (u != NULL)
				{
					for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
					{
						int weapId = PlayerWeaponry::getWeaponIdByUINumber(u, i);
						if (weapId != -1
							&& u->getWeaponByWeaponType(weapId) != -1
							&& u->getWeaponAmmoType(u->getWeaponByWeaponType(weapId)) != NULL)
						{
							this->ammo[i][currentEntry] = u->getWeaponAmmoAmount(u->getWeaponByWeaponType(weapId));
						} else {
							this->ammo[i][currentEntry] = 0;
						}
					}
				}

				lastCollectTime = Timer::getTime() / 1000;

				currentEntry++;
				if (currentEntry >= GAMESTATS_MAX_ENTRIES)
				{
					currentEntry = GAMESTATS_MAX_ENTRIES - 1;
					this->marker[currentEntry] = "(stats at max time)";
				}
			}

			
			void end()
			{
				running = false;

				if (!SimpleOptions::getBool(DH_OPT_B_COLLECT_GAME_STATS))
					return;

				Unit *u = game->gameUI->getFirstPerson(playerNum);
				if (u == NULL)
					return;

#ifdef LEGACY_FILES
				std::string filename = std::string("Stats/stats_");
#else
				std::string filename = std::string("stats/stats_");
#endif
				filename += missionid;
				filename += "_player";
				filename += int2str(playerNum + 1);
				filename += ".txt";

				FILE *f = fopen(filename.c_str(), "wb");

				if (f != NULL)
				{
					fprintf(f, "<mission_stats>\r\n");

					fprintf(f, "\r\n<summary>\r\n");

					int totalKills = 0;
					int totalDeaths = 0;
					int idleSamples = 0;
					{
						for (int e = 0; e <= currentEntry; e++)
						{
							bool didKills = false;
							for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
							{
								if (enemyNames[i] != "")
								{
									totalKills += this->kills[i][e];
									didKills = true;
								}
							}
							totalDeaths += this->deaths[e];

							// idle sample?
							if (this->deaths[e] == 0 
								&& !didKills
								&& this->pickup[e] == ""
								&& (e > 0 && this->health[e-1] == this->health[e])
								&& (e > 0 && this->upgrades[e-1] == this->upgrades[e]))
							{
								idleSamples++;
							}
						}
					}

					int mins = (this->currentEntry * GAMESTATS_RATE) / 60;
					int secs = (this->currentEntry * GAMESTATS_RATE) % 60;
					fprintf(f, "Total mission time roughly: %d min %d sec\r\n", mins, secs);
					fprintf(f, "Total kills (not counting indirect kills): %d\r\n", totalKills);
					fprintf(f, "Uninteresting time slices in minutes: %d min\r\n", (idleSamples * GAMESTATS_RATE) / 60);

					fprintf(f, "\r\n<spreadsheed_start>\r\n");

					fprintf(f, "marker;");
					fprintf(f, "maplayer;");
					fprintf(f, "pickups;");
					fprintf(f, "health;");
					fprintf(f, "deaths;");
					fprintf(f, "upgradeparts;");
					fprintf(f, "boughtparts;");
					fprintf(f, "totalupgparts;");
					fprintf(f, "boughtupg;");
#ifdef PROJECT_SURVIVOR
					fprintf(f, "characterparts;");
					fprintf(f, "exp;");
#endif
					{
						for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
						{
							if (u != NULL
								&& PlayerWeaponry::getWeaponIdByUINumber(u, i) != -1
								&& u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)) != -1
								&& u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i))) != NULL)
							{
								fprintf(f, "%s;", u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)))->getPartTypeIdString());
							} else {
								//fprintf(f, "ammo %d;", i);
							}
						}
					}
					{
						for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
						{
							if (u != NULL
								&& PlayerWeaponry::getWeaponIdByUINumber(u, i) != -1
								&& u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)) != -1
								&& u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i))) != NULL)
							{
								fprintf(f, "%s perc;", u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)))->getPartTypeIdString());
							} else {
								//fprintf(f, "ammo %d;", i);
							}
						}
					}
					{
						for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
						{
							if (enemyNames[i] != "")
							{
								fprintf(f, "%s;", enemyNames[i].c_str());
							}
						}
					}
					fprintf(f, "\r\n");

					{
						int totalEntries = currentEntry;
						if (currentEntry == GAMESTATS_MAX_ENTRIES-1)
						{
							totalEntries++;
						}
						for (int e = 0; e < totalEntries; e++)
						{
							fprintf(f, "%s;", this->marker[e].c_str());
							fprintf(f, "%s;", this->maplayer[e].c_str());
							fprintf(f, "%s;", this->pickup[e].c_str());
							fprintf(f, "%d;", this->health[e]);
							fprintf(f, "%d;", this->deaths[e]);
							fprintf(f, "%d;", this->upgrades[e]);
							fprintf(f, "%d;", this->boughtUpgrades[e] * 5);
							fprintf(f, "%d;", this->upgrades[e] + this->boughtUpgrades[e] * 5);
							fprintf(f, "%d;", this->boughtUpgrades[e]);
#ifdef PROJECT_SURVIVOR
							fprintf(f, "%d;", this->charparts[e]);
							fprintf(f, "%d;", this->exp[e]);
#endif
							{
								for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
								{
									if (u != NULL
										&& PlayerWeaponry::getWeaponIdByUINumber(u, i) != -1
										&& u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)) != -1
										&& u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i))) != NULL)
									{
										if (this->ammo[i][e] >= 99999)
											fprintf(f, "999;");
										else
											fprintf(f, "%d;", this->ammo[i][e]);
									} else {
										//fprintf(f, "%d;", 0);
									}
								}
							}
							{
								for (int i = 0; i < GAMESTATS_MAX_AMMO; i++)
								{
									if (u != NULL
										&& PlayerWeaponry::getWeaponIdByUINumber(u, i) != -1
										&& u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)) != -1
										&& u->getWeaponType(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i))) != NULL)
									{
										int maxammo = u->getWeaponMaxAmmoAmount(u->getWeaponByWeaponType(PlayerWeaponry::getWeaponIdByUINumber(u, i)));
										if (maxammo <= 0) 
											maxammo = 1;
										int ammoPercent = (100 * this->ammo[i][e]) / maxammo;
										if (ammoPercent > 100)
											ammoPercent = 100;
										if (this->ammo[i][e] >= 99999)
											fprintf(f, "100;");
										else
											fprintf(f, "%d;", ammoPercent);
									} else {
										//fprintf(f, "%d;", 0);
									}
								}
							}
							{
								for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
								{
									if (enemyNames[i] != "")
									{
										fprintf(f, "%d;", this->kills[i][e]);
									}
								}
							}
							fprintf(f, "\r\n");
						}
					}

					fclose(f);
				}
			}

			int getTotalKills() const
			{
				int totalKills = 0;
				for (int e = 0; e <= currentEntry; e++)
				{
					for (int i = 0; i < GAMESTATS_MAX_ENEMIES; i++)
					{
						if (enemyNames[i] != "")
						{
							totalKills += this->kills[i][e];
							// didKills = true;
						}
					}
					// totalDeaths += this->deaths[e];
				}

				return totalKills;
			}

			int getTotalTime() const
			{
				return (this->currentEntry * GAMESTATS_RATE);
			}

			int getSurvivalTime() const
			{
				int timeEnd;
				if(timeOfDeath != -1)
				{
					timeEnd = timeOfDeath;
				}
				else
				{
					timeEnd = game->gameTimer;
				}
				return (timeEnd - game->missionStartTime - game->missionPausedTime) / GAME_TICKS_PER_SECOND;
			}

			void getScoreValues(int &time, int &kills, int &scoreTime, int &scoreKills, int &totalScore)
			{
				time = getSurvivalTime();
				kills = getTotalKills();
				scoreTime = time * 50;
				std::string str = "survivor_current_expo[" + std::string(int2str(playerNum)) + "]";
				scoreKills = game->gameScripting->getGlobalIntVariableValue( str.c_str() );
				totalScore = scoreTime + scoreKills;
			}

			void updateScoreList(std::vector<GameStats::ScoreData> &scores, unsigned int &myPosition, const std::string &myName)
			{
				int time, kills, scoreTime, scoreKills, totalScore;
				getScoreValues(time, kills, scoreTime, scoreKills, totalScore);

				// load scores
				GameStats::loadScores(requestedScoreFile.c_str(), scores);
				std::vector<GameStats::ScoreData>::iterator myPlacement = scores.end();
				myPosition = 0;
				if(scores.size() > 0)
				{
					std::vector<GameStats::ScoreData>::iterator it;
					for(it = scores.begin(); it != scores.end(); it++)
					{
						// bigger score
						if(totalScore >= str2int((*it).score.c_str()))
						{
							myPlacement = it;
							break;
						}
						myPosition++;
					}
				}
				else
				{
					// no other scores = first place
					myPlacement = scores.begin();
					myPosition = 0;
				}

				GameStats::ScoreData newscore;
				newscore.name = myName; //game->getGameProfiles()->getCurrentProfile(playerNum);
				newscore.time = time2str(time);
				newscore.score = int2str(totalScore);
				scores.insert(myPlacement, newscore);
			}
	};


	GameStats::GameStats(Game *game, int playerClientNumber) :
		impl( NULL ),
		listeners()
	{
		this->impl = new GameStatsImpl();

		impl->game = game;
		impl->playerNum = playerClientNumber;

		impl->currentEntry = 0;
		impl->lastCollectTime = 0;
		impl->timeOfDeath = 0;
	}

	GameStats::~GameStats()
	{
		delete this->impl;
	}

	void GameStats::addListener( IGameStatsListener* listener )
	{
		listeners.push_back( listener );
	}
	
	void GameStats::removeListener( IGameStatsListener* listener )
	{
		std::list< IGameStatsListener* >::iterator i;
		i = std::find( listeners.begin(), listeners.end(), listener );
		if( i != listeners.end() ) 
			listeners.erase( i );

	}

	void GameStats::addKill(const char *enemy)
	{
		impl->addKill(enemy);

		std::list< IGameStatsListener* >::iterator i = listeners.begin();
		for( i = listeners.begin(); i != listeners.end(); ++i )
		{
			(*i)->onKill( enemy );
		}
	}

	void GameStats::addMapChange(const char *maplayer)
	{
		impl->addMapChange(maplayer);

		std::list< IGameStatsListener* >::iterator i = listeners.begin();
		for( i = listeners.begin(); i != listeners.end(); ++i )
		{
			(*i)->onMapChange( maplayer );
		}
	}

	void GameStats::addMarker(const char *marker)
	{
		impl->addMarker(marker);

		std::list< IGameStatsListener* >::iterator i = listeners.begin();
		for( i = listeners.begin(); i != listeners.end(); ++i )
		{
			(*i)->onMarker( marker );
		}
	}

	void GameStats::addPickup(const char *pickup)
	{
		impl->addPickup(pickup);

		std::list< IGameStatsListener* >::iterator i = listeners.begin();
		for( i = listeners.begin(); i != listeners.end(); ++i )
		{
			(*i)->onPickup( pickup );
		}
	}

	void GameStats::addDeath()
	{
		impl->addDeath();

		std::list< IGameStatsListener* >::iterator i = listeners.begin();
		for( i = listeners.begin(); i != listeners.end(); ++i )
		{
			(*i)->onDeath();
		}
	}

	void GameStats::runCollect()
	{
		impl->runCollect();
	}

	void GameStats::start(const char *missionid)
	{
		impl->start(missionid);
	}

	void GameStats::end()
	{
		impl->end();
	}

	int GameStats::getTotalKills() const
	{
		return impl->getTotalKills();
	}

	int GameStats::getTotalTime() const
	{
		return impl->getTotalTime();
	}

	int GameStats::getSurvivalTime() const
	{
		return impl->getSurvivalTime();
	}

	GameStats *GameStats::instances[MAX_PLAYERS_PER_CLIENT] = { NULL, NULL, NULL, NULL };


	void GameStats::loadScores(const char *file, std::vector<ScoreData> &scores)
	{
		frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(file, "rb");
		if(f == NULL)
		{
			Logger::getInstance()->debug("GameStats::loadScores - File could not be opened for reading.");
			Logger::getInstance()->debug(file);
			return;
		}

		int flen = frozenbyte::filesystem::fb_fsize(f);
		char *buf = (char *)alloca(flen + 1);
		int got = frozenbyte::filesystem::fb_fread(buf, sizeof(char), flen, f);
		frozenbyte::filesystem::fb_fclose(f);

		scores.reserve(3*3);
		ScoreData newscore;
		int start = 0;
		int word = 0;
		for(int j = 0; j < got; j++)
		{
			// decrypt
			buf[j] = ~(buf[j]);

			// parse
			if(buf[j] == '$')
			{
				// null terminate
				buf[j] = 0;
				word++;

				if(word == 1)
					newscore.name = buf + start;
				else if(word == 2)
					newscore.time = buf + start;
				else if(word == 3)
				{
					newscore.score = buf + start;
					scores.push_back(newscore);
					word = 0;
				}

				start = j + 1;
			}
		}
	}

	void GameStats::saveScores(const char *file, std::vector<ScoreData> &scores)
	{
		FILE *f = fopen(file, "wb");
		if(f == NULL)
		{
			Logger::getInstance()->error("GameStats::saveScores - File could not be opened for writing.");
			Logger::getInstance()->debug(file);
			return;
		}

		// build string
		std::string text;
		for(unsigned int i = 0; i < scores.size(); i++)
		{
			text += scores[i].name + "$" + scores[i].time + "$" + scores[i].score + "$";
		}

		// encrypt
		for(unsigned int i = 0; i < text.length(); i++)
		{
			text[i] = ~text[i];
		}

		// write
		fwrite(text.c_str(), text.length() + 1, 1, f);
		fclose(f);
	}

	void GameStats::setCurrentScoreFile(const char *file)
	{
		requestedScoreFile = file;
	}

	const char *GameStats::getCurrentScoreFile()
	{
		return requestedScoreFile.c_str();
	}

	void GameStats::markTimeOfDeath()
	{
		this->impl->markTimeOfDeath();
	}

	void GameStats::getScoreValues(int &time, int &kills, int &scoreTime, int &scoreKills, int &totalScore)
	{
		this->impl->getScoreValues(time, kills, scoreTime, scoreKills, totalScore);
	}

	void GameStats::updateScoreList(std::vector<ScoreData> &scores, unsigned int &myPosition, const std::string &myName)
	{
		this->impl->updateScoreList(scores, myPosition, myName);
	}

	bool GameStats::getScoreLimits(const char *mission, int &developer, int &ultimate)
	{
		struct ScoreLimit
		{
			const char *mission;
			int developer;
			int ultimate;
		};

		static const ScoreLimit limits[] =
		{
			{"survival\\surv_allover", 30000, 50000},
			{"survival\\surv_clawsoff", 30000, 50000},
			{"survival\\surv_comecloser", 30000, 50000},
			{"survival\\surv_forest", 30000, 50000},
			{"survival\\surv_overtime", 30000, 50000},
			{"survival\\surv_shriek", 30000, 50000},
			{"survival\\surv_snowstora", 30000, 50000},
			{"survival\\surv_spider", 30000, 50000},
			{"survival\\surv_techfacil", 300000, 50000},
			{NULL, 0, 0}
		};

		unsigned int i = 0;
		while(limits[i].mission != NULL)
		{
			if(strcmp(limits[i].mission, mission) == 0)
			{
				developer = limits[i].developer;
				ultimate = limits[i].ultimate;
				return true;
			}
			i++;
		}
		return false;
	}
}

