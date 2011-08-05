
#include "precompiled.h"

#include "DecalPositionCalculator.h"
#include "../game/GameMap.h"
#include "../game/GameScene.h"
#include "../system/SystemRandom.h"

#include "../game/MaterialManager.h"

#include <IStorm3D_Scene.h>

// TEMP: for debugging
#include <string.h>
#include <stdlib.h>
#include "../system/Logger.h"

namespace {

void getFaceNormal(const VC3 &a, const VC3 &b, const VC3 &c, VC3 &result)
{
	VC3 e1 = a;
	e1 -= b;
	VC3 e2 = c;
	e2 -= b;

	result = e1.GetCrossWith(e2);
}

void getNormal(game::GameScene *gameScene, const VC3 &position, const VC2 &area, VC3 &normal)
{
	VC3 point1(position.x - area.x, 0, position.z - area.y);
	point1.y = gameScene->getGameMap()->getScaledHeightAt(point1.x, point1.z);
	VC3 point2(position.x + area.x, 0, position.z - area.y);
	point2.y = gameScene->getGameMap()->getScaledHeightAt(point2.x, point2.z);
	VC3 point3(position.x - area.x, 0, position.z + area.y);
	point3.y = gameScene->getGameMap()->getScaledHeightAt(point3.x, point3.z);
	VC3 point4(position.x + area.x, 0, position.z + area.y);
	point4.y = gameScene->getGameMap()->getScaledHeightAt(point4.x, point4.z);

	VC3 normal1;
	getFaceNormal(point1, point2, point3, normal1);
	VC3 normal2;
	getFaceNormal(point2, point4, point3, normal2);

	normal = normal1;
	normal += normal2;
	normal.Normalize();
}

} // unnamed

namespace ui
{
	void DecalPositionCalculator::calculateDecalRotation(game::GameScene *gameScene, const VC3 &position, QUAT &resultRotation, float yAngle, VC3 &normal)
	{
		VC2 p2(position.x, position.z);
		//normal = gameScene->getTerrain()->getInterpolatedNormal(p2);
		//normal = gameScene->getTerrain()->getFaceNormal(p2);
		getNormal(gameScene, position, VC2(0.5f, 0.5f), normal);

		QUAT rotation;
		rotation.MakeFromAngles(-PI*.5f, yAngle, 0);
		QUAT::rotateToward(normal, VC3(0,1.f,0), resultRotation);

		resultRotation = rotation * resultRotation;


		/*
		VC3 x(rand() % 1000 / 999.f, 0, rand() % 1000 / 999.f);
		if(x.GetSquareLength() > 0.0001f)
			x.Normalize();
		else
			x = VC3(1.f, 0, 0);

		x -= hitNormal * x.GetDotWith(hitNormal);
		VC3 y = -x.GetCrossWith(hitNormal);

		assert(fabsf(x.GetDotWith(y)) < 0.0001f);
		assert(fabsf(x.GetDotWith(hitNormal)) < 0.0001f);

		MAT tm;
		tm.Set(0, x.x);
		tm.Set(1, x.y);
		tm.Set(2, x.z);
		tm.Set(4, y.x);
		tm.Set(5, y.y);
		tm.Set(6, y.z);
		tm.Set(8, hitNormal.x);
		tm.Set(9, hitNormal.y);
		tm.Set(10, hitNormal.z);

		resultRotation = tm.GetRotation();
		*/
	}

	// return true if position ok and decal should be added
	//        false, if position NOT ok and decal should NOT be added
	bool DecalPositionCalculator::calculateDecalPosition(
		game::GameScene *gameScene,
		const VC3 &origin, const VC3 &velocity, 
		DECAL_POSITIONING positioning, int positionRandom,
		VC3 *resultPosition, QUAT *resultRotation)
	{
		assert(positioning != DecalPositionCalculator::DECAL_POSITIONING_INVALID);

		game::GameMap *gameMap = gameScene->getGameMap();

		bool hitWall = false;

		*resultPosition = origin;
		*resultRotation = QUAT((-3.1415926f / 2.0f),0,0);

		// if velocity positioning...
		if (positioning == DecalPositionCalculator::DECAL_POSITIONING_VELOCITY)
		{
			VC3 velocityRandomized;
			if (positionRandom > 0)
			{
				velocityRandomized = velocity * GAME_TICKS_PER_SECOND;

	// TEMP
	//char buf[64];
	//sprintf(buf, "%f,%f,%f", velocity.x, velocity.y, velocity.z);
	//Logger::getInstance()->error(buf);

				// TODO: add positionRandom to velocity _angle_...
				// (or maybe just do a quick hack and add it to xz-coordinates?)
				velocityRandomized.x += float((SystemRandom::getInstance()->nextInt() % (positionRandom * 2 + 1)) - positionRandom) / 100.0f;
				velocityRandomized.z += float((SystemRandom::getInstance()->nextInt() % (positionRandom * 2 + 1)) - positionRandom) / 100.0f;
				
				// add to y too? maybe should add downward only? 
				// NOTE: biased downward
				velocityRandomized.y += float((SystemRandom::getInstance()->nextInt() % (positionRandom * 2 + 1)) - positionRandom) / 100.0f;
				velocityRandomized.y -= float(positionRandom) / 100.0f * 0.5f;

			} else {
				velocityRandomized = velocity;
			}

			velocityRandomized *= 2.0f;

			IStorm3D_Scene *scene = gameScene->getStormScene();

			VC3 dir = velocityRandomized.GetNormalized();			
			VC3 rayOrigin = origin;
			float rayLen = velocityRandomized.GetLength();

			Storm3D_CollisionInfo sceneColl;
			sceneColl.includeTerrainObjects = false;
			scene->RayTrace(rayOrigin, dir, rayLen, sceneColl, true);

			/*
			if (sceneColl.hit)
			{
				VC3 hitNormal = sceneColl.plane_normal;
				// make a "wall" hit if normal-y is not nearly 1
				//if (fabs(hitNormal.y) < 0.8f)
				{
					hitWall = true;

					VC3 x(rand() % 1000 / 999.f, 0, rand() % 1000 / 999.f);
					x.Normalize();
					x -= hitNormal * x.GetDotWith(hitNormal);
					VC3 y = -x.GetCrossWith(hitNormal);

					assert(fabsf(x.GetDotWith(y)) < 0.0001f);
					assert(fabsf(x.GetDotWith(hitNormal)) < 0.0001f);

					MAT tm;
					tm.Set(0, x.x);
					tm.Set(1, x.y);
					tm.Set(2, x.z);
					tm.Set(4, y.x);
					tm.Set(5, y.y);
					tm.Set(6, y.z);
					tm.Set(8, hitNormal.x);
					tm.Set(9, hitNormal.y);
					tm.Set(10, hitNormal.z);

					*resultRotation = tm.GetRotation();
				}
				*resultPosition = sceneColl.position;
			} else {
				*resultPosition += velocityRandomized;
			}
			*/

			// New version
			{
				VC3 hitNormal(0, 1.f, 0);
				if(sceneColl.hit)
				{
					hitNormal = sceneColl.plane_normal;
					*resultPosition = sceneColl.position;
				}
				else
				{
					*resultPosition += velocityRandomized;
					VC2 p2(resultPosition->x, resultPosition->z);
					hitNormal = gameScene->getTerrain()->getFaceNormal(p2);
				}

				{
					if(sceneColl.hit)
						hitWall = true;

		/*
		VC3 y = dir;
		VC3 z = hitNormal;
		y -= hitNormal * y.GetDotWith(hitNormal);
		VC3 x = z.GetCrossWith(y);
		*/

		VC3 x = dir;
		x.y = 0.f;
		x.Normalize();
		x -= hitNormal * x.GetDotWith(hitNormal);
		VC3 y = -x.GetCrossWith(hitNormal);
		VC3 z = hitNormal;

		MAT tm;
		tm.Set(0, x.x);
		tm.Set(1, x.y);
		tm.Set(2, x.z);
		tm.Set(4, y.x);
		tm.Set(5, y.y);
		tm.Set(6, y.z);
		tm.Set(8, z.x);
		tm.Set(9, z.y);
		tm.Set(10, z.z);

		resultRotation->MakeFromAngles(0.f, 0.f, -PI*0.5f);
		*resultRotation = (*resultRotation) * tm.GetRotation();

					/*
					VC3 x(rand() % 1000 / 999.f, 0, rand() % 1000 / 999.f);
					x.Normalize();
					x -= hitNormal * x.GetDotWith(hitNormal);
					VC3 y = -x.GetCrossWith(hitNormal);

					assert(fabsf(x.GetDotWith(y)) < 0.0001f);
					assert(fabsf(x.GetDotWith(hitNormal)) < 0.0001f);

					MAT tm;
					tm.Set(0, x.x);
					tm.Set(1, x.y);
					tm.Set(2, x.z);
					tm.Set(4, y.x);
					tm.Set(5, y.y);
					tm.Set(6, y.z);
					tm.Set(8, hitNormal.x);
					tm.Set(9, hitNormal.y);
					tm.Set(10, hitNormal.z);

					*resultRotation = tm.GetRotation();
					*/
				}
			}


			// TODO: some kind of terrain raytrace maybe...?
			// should collide to walls, etc.
		}

		// if downward positioning...
		if (positioning == DecalPositionCalculator::DECAL_POSITIONING_DOWNWARD)
		{
			if (positionRandom > 0)
			{
				// TODO: add a random xz-offset to result position
				//*resultPosition += randomizedOffset;
				resultPosition->x += float((SystemRandom::getInstance()->nextInt() % (positionRandom * 2 + 1)) - positionRandom) / 100.0f;
				resultPosition->z += float((SystemRandom::getInstance()->nextInt() % (positionRandom * 2 + 1)) - positionRandom) / 100.0f;
			}			

			/*
			// psd
			{
				VC2 p2(resultPosition->x, resultPosition->z);
				VC3 hitNormal = gameScene->getTerrain()->getFaceNormal(p2);

				VC3 x(rand() % 1000 / 999.f, 0, rand() % 1000 / 999.f);
				x.Normalize();
				x -= hitNormal * x.GetDotWith(hitNormal);
				VC3 y = -x.GetCrossWith(hitNormal);

				assert(fabsf(x.GetDotWith(y)) < 0.0001f);
				assert(fabsf(x.GetDotWith(hitNormal)) < 0.0001f);

				MAT tm;
				tm.Set(0, x.x);
				tm.Set(1, x.y);
				tm.Set(2, x.z);
				tm.Set(4, y.x);
				tm.Set(5, y.y);
				tm.Set(6, y.z);
				tm.Set(8, hitNormal.x);
				tm.Set(9, hitNormal.y);
				tm.Set(10, hitNormal.z);

				*resultRotation = tm.GetRotation();
			}
			*/
			VC3 fooNormal;
			calculateDecalRotation(gameScene, *resultPosition, *resultRotation, 0.f, fooNormal);
		}

		// now check that we're still inside map boundaries
		if (!gameMap->isWellInScaledBoundaries(resultPosition->x, resultPosition->z))
		{
			// out of map.
			return false;
		}

		// then fix decal height to ground height
		if (positioning == DecalPositionCalculator::DECAL_POSITIONING_DOWNWARD
			|| positioning == DecalPositionCalculator::DECAL_POSITIONING_VELOCITY)
		{
			if (!hitWall)
			{
				resultPosition->y = gameMap->getScaledHeightAt(resultPosition->x, resultPosition->z);
			}
		}

		// check that not on top of metal grid area...
		if (game::MaterialManager::isMaterialUnderPosition(gameMap, 
			*resultPosition, MATERIAL_METAL_GRATE))
		{
			// on top of grid, no decal here.
			return false;
		}

		// TODO: check that not inside a wall, terrainobject, etc.
		// if so, return false

		return true;
	}

}

