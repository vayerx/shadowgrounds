
#ifndef PLAYERUNITCAMERABOUNDARY_H
#define PLAYERUNITCAMERABOUNDARY_H

#include "IGameCameraBoundary.h"

namespace game
{
	class UnitList;
}

namespace ui
{
	/**
	 * The normal camera boundary checker while in mission.
	 * Checks that the camera is near the given player's units.
	 * See the NoCameraBoundary if you don't want this sort of 
	 * limitation for the camera.
	 *
   * @version 0.5, 28.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see NoCameraBoundary
   * @see IGameCameraBoundary
	 */
	class PlayerUnitCameraBoundary : public IGameCameraBoundary
	{
		public:
			PlayerUnitCameraBoundary(game::UnitList *unitList, int player);

			~PlayerUnitCameraBoundary();

			virtual bool isPositionInsideBoundaries(const VC3 &position);

			virtual VC3 getVectorToInsideBoundaries(const VC3 &position);

		private:
			game::UnitList *unitList;
			int player;
	};
}

#endif

