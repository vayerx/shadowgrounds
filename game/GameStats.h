
#ifndef GAMESTATS_H
#define GAMESTATS_H

#include <list>
#include "gamedefs.h"

namespace game
{
	class Game;
	class GameStatsImpl;

	class IGameStatsListener
	{
	public:
		virtual ~IGameStatsListener() { }

		virtual void onKill( const std::string& enemy ) = 0;
		virtual void onMapChange( const std::string& maplayer ) = 0;
		virtual void onMarker( const std::string& marker ) = 0;
		virtual void onPickup( const std::string& pickup ) = 0;
		virtual void onDeath() = 0;
	};

	class GameStats
	{
		public:
			GameStats(Game *game, int playerClientNum);
			~GameStats();

			// added by Pete for the use ingame ui special score systems (combo/momentum)
			void addListener( IGameStatsListener* listener );
			void removeListener( IGameStatsListener* listener );

			// these should be called by game as such events happen
			void addKill(const char *enemy);
			void addMapChange(const char *maplayer);
			void addMarker(const char *marker);
			void addPickup(const char *pickup);
			void addDeath();
			void markTimeOfDeath();

			// this call should be continuosly called, it will automagically collect some
			// data at the defined stats collect rate
			void runCollect();

		  // call at mission start
			void start(const char *missionid);

		  // call at mission end
			void end();

			// highscore stuff
			//
			struct ScoreData
			{
				std::string name;
				std::string time;
				std::string score;
			};
			void getScoreValues(int &time, int &kills, int &scoreTime, int &scoreKills, int &totalScore);
			void updateScoreList(std::vector<ScoreData> &scores, unsigned int &myPosition, const std::string &myName);
			static void loadScores(const char *file, std::vector<ScoreData> &scores);
			static void saveScores(const char *file, std::vector<ScoreData> &scores);
			static void setCurrentScoreFile(const char *file);
			static bool getScoreLimits(const char *mission, int &developer, int &ultimate);
			static const char *getCurrentScoreFile();

			//=================================================================
			// added by Pete, getters for the data, used for score window
			
			int getTotalKills() const;
			
			int getTotalTime() const;	// result in secs

			int getSurvivalTime() const;

			//=================================================================
			static GameStats *instances[MAX_PLAYERS_PER_CLIENT];

		private:
			GameStatsImpl *impl;
			std::list< IGameStatsListener* > listeners;
	};

}

#endif


