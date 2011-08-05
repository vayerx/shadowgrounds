
#ifndef DECALPOSITIONCALCULATOR_H
#define DECALPOSITIONCALCULATOR_H

#include <DatatypeDef.h>

namespace game
{
	class GameScene;
}

namespace ui
{
	class DecalPositionCalculator
	{
		public:

			enum DECAL_POSITIONING
			{
				DECAL_POSITIONING_INVALID = 1,
				DECAL_POSITIONING_VELOCITY = 2,
				DECAL_POSITIONING_DOWNWARD = 3,
				DECAL_POSITIONING_ORIGIN = 4
			};

			static void calculateDecalRotation(game::GameScene *gameScene, const VC3 &position, QUAT &resultRotation, float yAngle, VC3 &normal);

			// return true if position ok and decal should be added
			//        false, if position NOT ok and decal should NOT be added
			static bool calculateDecalPosition(
				game::GameScene *gameScene,
				const VC3 &origin, const VC3 &velocity, 
				DECAL_POSITIONING positioning, int positionRandom,
				VC3 *resultPosition, QUAT *resultRotation);
	};

}

#endif


