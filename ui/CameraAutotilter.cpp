
#include "precompiled.h"

#include "CameraAutotilter.h"
#include "../game/GameMap.h"
#include "../game/scaledefs.h"
#include "../system/Timer.h"
#include "../system/Logger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_camera.h"
#include <assert.h>


// obstacle height after which the camera will tilt...
// obstacles lower than this should have no effect.
#define CAMERATILT_ABOVE_OBSTACLE_HEIGHT 1.0f

// NOTE: this is in obstacle blocks, so if map scale changes, this
// needs to be changed too to compensate the change in meters...
#define CAMERATILT_CHECK_DISTANCE 10


namespace ui
{

	CameraAutotilter::CameraAutotilter(game::GameMap *gameMap)
	{
		this->gameMap = gameMap;
		this->lastRotation = VC3(0,0,0);
		this->lastPosition = VC3(0,0,0);
		this->lastTime = Timer::getTime();
	}

	CameraAutotilter::~CameraAutotilter()
	{
		// nop
	}

	VC3 CameraAutotilter::getTilt(const VC3 &position, float alphaAngle)
	{
		VC3 ret = VC3(0,0,0);

		assert(gameMap != NULL);

		// FIXME: need to rotate based on camera!!!
		// if camera points to some other direction, tilt
		// won't work corretly!!!

		int px = gameMap->scaledToObstacleX(position.x);
		int py = gameMap->scaledToObstacleY(position.z);

		//assert(px >= 0 && py >= 0 && px < gameMap->getObstacleSizeX()
		//	&& py < gameMap->getObstacleSizeY());

		if(px <= 0 || py < 0 || px >= gameMap->getObstacleSizeX()
			|| py >= gameMap->getObstacleSizeY())
		{
			return ret;
		}

		float rotAngle = 90 + alphaAngle;
		if (rotAngle < 0) rotAngle += 360;
		if (rotAngle >= 360) rotAngle -= 360;

		// FIXME: rotated_x_vec is not correct!
		// therefore, returned y-axis rotation result is incorrect!!!

		VC2 rotated_y_vec = VC2(sinf(UNIT_ANGLE_TO_RAD(rotAngle)), cosf(UNIT_ANGLE_TO_RAD(rotAngle)));
		VC2 rotated_x_vec = -VC2(cosf(UNIT_ANGLE_TO_RAD(rotAngle)), sinf(UNIT_ANGLE_TO_RAD(rotAngle)));

		int obstSizeX = gameMap->getObstacleSizeX();
		int obstSizeY = gameMap->getObstacleSizeY();

		for (int y = -1; y > -CAMERATILT_CHECK_DISTANCE; y--)
		{
			int tx = int(px + rotated_y_vec.x * float(y));
			int ty = int(py + rotated_y_vec.y * float(y));
			if (tx >= 0 && ty >= 0 && tx < obstSizeX && ty < obstSizeY)
			{
				if (gameMap->getObstacleHeight(tx, ty) > CAMERATILT_ABOVE_OBSTACLE_HEIGHT
					&& !gameMap->isMovingObstacle(tx, ty))
				{
  				ret.x = float(-CAMERATILT_CHECK_DISTANCE - y) * 2;
					break;
				}
			}
		}

		int x;
		int minx = -CAMERATILT_CHECK_DISTANCE;
		for (x = 1; x < CAMERATILT_CHECK_DISTANCE; x++)
		{
			int tx = int(px + rotated_x_vec.x * float(x));
			int ty = int(py + rotated_x_vec.y * float(x));
			if (tx >= 0 && ty >= 0 && tx < obstSizeX && ty < obstSizeY)
			{
				if (gameMap->getObstacleHeight(tx, ty) > CAMERATILT_ABOVE_OBSTACLE_HEIGHT
					&& !gameMap->isMovingObstacle(tx, ty))
				{
					minx = -x;
					ret.y = float(CAMERATILT_CHECK_DISTANCE - x);
					break;
				}
			}
		}
		for (x = -1; x >= minx; x--)
		{
			int tx = int(px + rotated_x_vec.x * float(x));
			int ty = int(py + rotated_x_vec.y * float(x));
			if (tx >= 0 && ty >= 0 && tx < obstSizeX && ty < obstSizeY)
			{
				if (gameMap->getObstacleHeight(tx, ty) > CAMERATILT_ABOVE_OBSTACLE_HEIGHT
					&& !gameMap->isMovingObstacle(tx, ty))
				{
					if (x == minx)
					{
						ret.y = 0.0f;
					} else {
						ret.y = float(-CAMERATILT_CHECK_DISTANCE - x);
					}
					break;
				}
			}
		}
		ret.y = -ret.y;

		int curTime = Timer::getTime();
		int timeDiff = curTime - lastTime;
		if (timeDiff > 100) 
		{
			timeDiff = 100;
			lastTime = curTime;
		}
		if (timeDiff < 0)
		{
			timeDiff = 0;
			lastTime = curTime;
		}
		if (lastTime < curTime)
			lastTime += timeDiff;

		float confFactor = 0.02f * game::SimpleOptions::getInt(DH_OPT_I_CAMERA_AUTOTILT_AMOUNT);
		if (fabs(ret.y - lastRotation.y) <= 2 * confFactor)
		{
			ret.y = lastRotation.y;
		}
		if (fabs(ret.x - lastRotation.x) <= 2 * confFactor)
		{
			ret.x = lastRotation.x;
		}
		float timeFactor = confFactor * 0.01f * float(timeDiff);

		VC3 posDiffVector = position - lastPosition;
		VC3 posDiffVectorRotated = VC3(
			posDiffVector.x * cosf(UNIT_ANGLE_TO_RAD(rotAngle)) 
			+ posDiffVector.z * sinf(UNIT_ANGLE_TO_RAD(rotAngle)),
			0,
			posDiffVector.x * sinf(UNIT_ANGLE_TO_RAD(rotAngle)) 
			+ posDiffVector.z * cosf(UNIT_ANGLE_TO_RAD(rotAngle)));

		//if (posDiffVectorRotated.z < -0.01f)
		//	Logger::getInstance()->error("z m");
		//if (posDiffVectorRotated.z > 0.01f)
		//	Logger::getInstance()->error("z p");

		if (ret.x < lastRotation.x)
		{
			//if (position.z < lastPosition.z)
			//if (posDiffVectorRotated.z < -0.01f)
				ret.x = lastRotation.x - timeFactor;
			//else
			//	ret.x = lastRotation.x;
		}
		if (ret.x > lastRotation.x)
		{
			//if (position.z > lastPosition.z)
			//if (posDiffVectorRotated.z > 0.01f)
				ret.x = lastRotation.x + timeFactor;
			//else
			//	ret.x = lastRotation.x;
		}

		if (ret.y < lastRotation.y)
		{
			//if (position.x > lastPosition.x)
			//if (posDiffVectorRotated.x < -0.01f)
				ret.y = lastRotation.y - timeFactor;
			//else
			//	ret.y = lastRotation.y;
		}
		if (ret.y > lastRotation.y)
		{
			//if (position.x < lastPosition.x)
			//if (posDiffVectorRotated.x > 0.01f)
				ret.y = lastRotation.y + timeFactor;
			//else
			//	ret.y = lastRotation.y;
		}

		lastRotation = ret;
		lastPosition = position;

		return ret;
	}

}

