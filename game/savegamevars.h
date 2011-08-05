
#ifndef SAVEGAMEVARS_H
#define SAVEGAMEVARS_H

#include <string>

namespace game
{
	/*
	enum SAVEGAME_TYPE
	{
		SAVEGAME_TYPE_INVALID = 0,
		SAVEGAME_TYPE_SAVEGAME = 1,
		SAVEGAME_TYPE_NEWGAME = 2,
	};
	*/

	extern std::string savegame_type;
	extern std::string savegame_version;
	extern std::string savegame_description;
	extern std::string savegame_time;
	extern std::string savegame_stats;
	extern std::string savegame_mission_id;
}

#endif

