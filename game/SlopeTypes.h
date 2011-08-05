
#ifndef SLOPETYPES_H
#define SLOPETYPES_H

#include <DatatypeDef.h>

// obstacle map based slope types for sideways gameplay...

namespace game
{
	class GameMap;
	class GameScene;

	enum SLOPE_TYPE
	{
		SLOPE_TYPE_NONE = 0,
		SLOPE_TYPE_LEFT_45,
		SLOPE_TYPE_RIGHT_45,
		SLOPE_TYPE_LEFT_22_5_LOWER,
		SLOPE_TYPE_LEFT_22_5_UPPER,
		SLOPE_TYPE_RIGHT_22_5_LOWER,
		SLOPE_TYPE_RIGHT_22_5_UPPER,
		SLOPE_TYPE_LEFT_45_TEMP, // (never returned as result)
		SLOPE_TYPE_RIGHT_45_TEMP, // (never returned as result)
		SLOPE_TYPE_FLAT,
		SLOPE_TYPE_INNER,
		/*
		SLOPE_TYPE_LEFT_67_5_LOWER,
		SLOPE_TYPE_LEFT_67_5_UPPER,
		SLOPE_TYPE_RIGHT_67_5_LOWER,
		SLOPE_TYPE_RIGHT_67_5_UPPER,
		*/

		NUM_SLOPE_TYPES
	};

	void solveSlopeType(game::GameMap *gameMap, game::GameScene *gameScene, const VC3 &pos, bool &ongroundOut, bool &onslopeOut, SLOPE_TYPE &slopeTypeOut, VC3 &surfacePositionOut);
}

#endif

