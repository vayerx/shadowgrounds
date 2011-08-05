
#ifndef OBJECTSTRECHINGCALCULATOR_H
#define OBJECTSTRECHINGCALCULATOR_H

#include <DatatypeDef.h>

// WARNING! bad dependency to the game! (angle conversion)
#include "../game/scaledefs.h"


namespace util
{
  class ObjectStretchingCalculator
	{
		public:
			/**
			 * Calculate the necessary values for creating objects that are
			 * stretched between two points.
			 * @param from, const VC3&, the starting coordinates.
			 * @param to, const VC3&, the ending coordinates.
			 * @param midpos, VC3*, the resulting midposition.
			 * @param rotation, VC3*, the resulting rotation. Notice,
			 *        these values are not radians, they are degrees
			 *        (in the game's angle system)
			 * @param scale, float*, the resulting scale.
			 */
			static void calculateStretchValues(const VC3 &from, const VC3 &to,
				VC3 *midpos, VC3 *rotation, float *scale)
			{
				// midposition...
				*midpos = (from + to) / 2;

				// rotation for the object to be placed at the midfrom.
				VC2 destFloat = VC2(
					(float)(to.x-from.x), (float)(to.z-from.z));
				float destAngleFloat = destFloat.CalculateAngle();
				float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
				while (destAngle >= 360) destAngle -= 360;

				VC2 destFloat2 = VC2(
					(float)(destFloat.GetLength()), (float)(to.y-from.y));
				float destAngleFloat2 = destFloat2.CalculateAngle();
				float destAngle2 = RAD_TO_UNIT_ANGLE(destAngleFloat2) + 360;
				while (destAngle2 >= 360) destAngle2 -= 360;

				rotation->x = destAngle2;
				rotation->y = destAngle;
				rotation->z = 0;

				// and scale for the object.
				// notice: this of course depends on the object's original
				// size. this value applies for 1 meter. (probably want to
				// scale the z-coordinates with it.)
				VC3 distVector = to - from;
        float distLen = distVector.GetLength();
				*scale = distLen;
			}
	};
}

#endif



