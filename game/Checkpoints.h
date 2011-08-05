
#ifndef CHECKPOINTS_H
#define CHECKPOINTS_H

#include <math.h>
#include <DatatypeDef.h>

namespace game
{
	class Checkpoints
	{
		public:
			Checkpoints()
			{
				checkPointX = 0;
				checkPointY = 0;
				enabled = false;
			}


			~Checkpoints() 
			{ 
				// nop
			}


			void setCheckpoint(int num, float scaledX, float scaledY)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				checkPointX = scaledX;
				checkPointY = scaledY;
			}			


			void enableCheckpoint(int num)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				enabled = true;
			}


			void disableCheckpoint(int num)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				enabled = false;
			}


			bool isCheckpointEnabled(int num)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				return enabled;
			}


			float getDistanceToCheckpoint(int num, VC3 &position)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				// NOTICE: returns the 2D distance (height ignored)
				float x = position.x - checkPointX;
				float z = position.z - checkPointY;
				return sqrtf((x * x) + (z * z));
			}


			bool isNearCheckpoint(int num, const VC3 &position, float range)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

				// NOTICE: returns the 2D distance (height ignored)
				float x = position.x - checkPointX;
				float z = position.z - checkPointY;
				if ((x * x) + (z * z) <= range * range)
					return true;
				else
					return false;
			}


			// NOTICE: returns radians
			float getAngleToCheckpoint(int num, VC3 &position)
			{
				// TODO: only 1 checkpoint supported, may want to have more.
				assert(num == 0);
				num = num; // warning removal

        VC3 pos = position;
        pos.x -= checkPointX;
        pos.z -= checkPointY;
        float distSq = pos.x * pos.x + pos.z * pos.z;
        float dist = sqrtf(distSq);
				float angle;
				if (dist == 0) 
					angle = 0;
				else
					angle = (float)acos(pos.x / dist);
        if (pos.z < 0) angle = -angle;

				return angle;
			}


		private:
			bool enabled;
			float checkPointX;
			float checkPointY;
	};
}

#endif

