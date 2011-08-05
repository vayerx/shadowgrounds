
#ifndef ANGLEROTATIONCALCULATOR_H
#define ANGLEROTATIONCALCULATOR_H

namespace util
{
	class AngleRotationCalculator
	{
		public:
			inline static float getFactoredRotationForAngles(float fromAngle, 
				float toAngle, float accuracy)
			{
				float rotSpeed = 0;

				assert(toAngle >= 0 && toAngle <= 360);
				assert(fromAngle >= 0 && fromAngle <= 360);

				fromAngle -= toAngle;
				if (fromAngle < -180) fromAngle += 360;
				if (fromAngle >= 180) fromAngle -= 360;

				if (fromAngle > accuracy)
				{
					rotSpeed = -fromAngle;
				} else {
					if (fromAngle < -accuracy)
						rotSpeed = -fromAngle;
				}

				return rotSpeed;
			}


			inline static float getRotationForAngles(float fromAngle, 
				float toAngle, float accuracy)
			{
				float rotSpeed = getFactoredRotationForAngles(fromAngle, toAngle, accuracy);
				if (rotSpeed != 0)
				{
					if (rotSpeed < 0)
						rotSpeed = -1;
					else
						rotSpeed = 1;
				}
				return rotSpeed;

				/*
				float rotSpeed = 0;

				assert(toAngle >= 0 && toAngle <= 360);
				assert(fromAngle >= 0 && fromAngle <= 360);

				fromAngle -= toAngle;
				if (fromAngle < -180) fromAngle += 360;
				if (fromAngle >= 180) fromAngle -= 360;

				if (fromAngle > accuracy)
				{
					rotSpeed = -1;
				} else {
					if (fromAngle < -accuracy)
						rotSpeed = 1;
				}

				return rotSpeed;
				*/
			}

	};
}

#endif



