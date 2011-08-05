
#ifndef POSITIONSDIRECTIONCALCULATOR_H
#define POSITIONSDIRECTIONCALCULATOR_H

#include <DatatypeDef.h>
#include "ObjectStretchingCalculator.h"

namespace util
{
  class PositionDirectionCalculator
	{
		public:
			/**
			 * Calculate the angle from point a to point b.
			 * The angle will only be given for y-rotation.
			 * (To get other angles too, see ObjectStretchingCalculator.)
			 *
			 * @param from, const VC3&, the starting coordinate.
			 * @param to, const VC3&, the ending coordinate.
			 * @return float, the resulting rotation. Notice,
			 *        these values are not radians, they are degrees
			 *        (in the game's angle system)
			 * @see ObjectStretchingCalculator
			 */
			static float calculateDirection(const VC3 &from, const VC3 &to)
			{
				// not too efficient, but very simple. :)
				// TODO: optimize 
				// (copy necessary parts of the implementation here)
				float scale;
				VC3 rotation;
				VC3 midpos;
				util::ObjectStretchingCalculator::calculateStretchValues(
					from, to, &midpos, &rotation, &scale);
				return rotation.y;
			}
	};
}

#endif



