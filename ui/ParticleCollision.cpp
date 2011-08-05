
#include "precompiled.h"

#include "ParticleCollision.h"
#include "../game/GameMap.h"
#include "../game/GameScene.h"
#include "../game/areamasks.h"
#include "../util/AreaMap.h"
#include <IStorm3D_Scene.h>

using namespace util;
using namespace game;

namespace ui {
namespace {

	int getFloorHeightInt(game::GameMap *map, float wx, float wy)
	{
		int x = map->scaledToHeightmapX(wx);
		int y = map->scaledToHeightmapY(wy);

		return map->getHeightmapHeightAt(x, y);
	}

	int getObstacleHeightInt(game::GameMap *map, float wx, float wy)
	{
		int x = map->scaledToObstacleX(wx);
		int y = map->scaledToObstacleY(wy);

		return map->getObstacleHeight(x, y) + getFloorHeightInt(map, wx, wy);
	}

	void rotateVector(VC3 &vector, float angle)
	{
		float x = vector.x;
		float z = vector.z;

		vector.x = x * cosf(angle) + z * sinf(angle);
		vector.z = -x * sinf(angle) + z * cosf(angle);
	}
	
	bool canSpawn(game::GameMap *map, const VC3 &pos, int maxDifference, int yInt)
	{
		float wx = pos.x;
		float wy = pos.z;
		int x = map->scaledToObstacleX(wx);
		int y = map->scaledToObstacleY(wy);
		int height = map->getObstacleHeight(x, y);
		int floor = getFloorHeightInt(map, pos.x, pos.z);

		// ???? ok?
		if (map->getAreaMap()->isAreaAnyValue(x, y, AREAMASK_OBSTACLE_MOVABLE))
			return true;

		if(yInt < height + floor)
			return false;

		int sheight[8] = { 0 };
		sheight[0] = map->getObstacleHeight(x + 1, y);
		sheight[1] = map->getObstacleHeight(x - 1, y);
		sheight[2] = map->getObstacleHeight(x, y + 1);
		sheight[3] = map->getObstacleHeight(x, y - 1);
		sheight[4] = map->getObstacleHeight(x + 1, y + 1);
		sheight[5] = map->getObstacleHeight(x + 1, y - 1);
		sheight[6] = map->getObstacleHeight(x - 1, y + 1);
		sheight[7] = map->getObstacleHeight(x - 1, y - 1);

		for(int i = 0; i < 8; ++i)
		{
			if(abs(sheight[i] - height) > maxDifference)
				return false;
		}

		return true;
	}

} // unnamed

ParticleCollision::ParticleCollision(game::GameMap *gameMap_, game::GameScene *scene_)
:	gameMap(gameMap_),
	scene(scene_),
	scale(0),
	meters_1(0),
	meters_1_5(0),
	meters_3_5(0),
	meters_0_2(0),
	meters_0_3(0)
{
	assert(gameMap);

	scale = 1.f / gameMap->getScaleHeight();
	meters_1 = int(scale);
	meters_1_5 = int(1.5f * scale);
	meters_3_5 = int(3.5f * scale);
	meters_0_2 = int(0.2f * scale);
	meters_0_3 = int(0.3f * scale);
}

ParticleCollision::~ParticleCollision()
{
}

bool ParticleCollision::spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const
{
	if(!gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;
	if(canSpawn(gameMap, position, meters_1_5, int(position.y * scale)))
		return true;

	VC3 halfPosition = emitter - position;
	halfPosition *= .5f;
	halfPosition += position;
	if(canSpawn(gameMap, halfPosition, meters_1_5, int(halfPosition.y * scale)))
	{
		position = halfPosition;
		return true;
	}

	if(canSpawn(gameMap, emitter, meters_1_5, int(emitter.y * scale)))
	{
		position = emitter;
		return true;
	}

	VC3 final = emitter - dir;
	if(canSpawn(gameMap, final, meters_1_5, int(final.y * scale)))
	{
		position = final;
		return true;
	}

	return false;
}

/*
bool ParticleCollision::getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float groundSlowFactor, float wallSlowFactor) const
{
	if(!gameMap || !gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;

	VC3 dir = position;
	dir -= oldPosition;
	float dirLengthSquared = dir.GetSquareLength();
	if(dirLengthSquared > 0.000000001f)
		dir.Normalize();

	int positionY = int(position.y * scale);
	int floor = getFloorHeightInt(gameMap, position.x, position.z);
	int height = getObstacleHeightInt(gameMap, position.x, position.z);
	float scaleHeight = gameMap->getScaleHeight();

	// Try to handle steep walls
	if(positionY > floor + meters_0_2)
	{
		VC3 forward = dir;
		// WARNING: this used to be 1.5 (meters), now it's 0.2 - seems to work better
		// may cause some problems on some cases... --jpk
		//forward *= 1.5f;
		forward *= 0.2f;
		forward += position;

		int forwardHeight = getObstacleHeightInt(gameMap, forward.x, forward.z);
		if(forwardHeight - height > meters_1)
			height = forwardHeight;
	}

	// Avoid flying too high
	if(positionY > floor + meters_3_5 && velocity.y > 0)
	{
		AreaMap *areaMap = gameMap->getAreaMap();

		float wx = position.x;
		float wy = position.z;
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);
		
		if(areaMap->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
		{
			velocity.y = -velocity.y;
			velocity *= groundSlowFactor;

			position.y = (floor * scaleHeight) + 3.5f;
			return true;
		}
	}

	if(positionY > height)
		return false;

	if(positionY < height && height - positionY < meters_0_3)
	{
		velocity.y = -velocity.y;
		velocity *= groundSlowFactor;
		position.y = height * scaleHeight;

		if(fabsf(velocity.y) < 0.02f)
			velocity.y = 0;
	}

	VC3 side(dir.z, dir.y, -dir.x);
	side.x *= 1.5f * gameMap->getScaleX() / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY;
	side.z *= 1.5f * gameMap->getScaleY() / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY;

	VC3 left(position.x - side.x, position.y, position.z - side.z);
	VC3 right(position.x + side.x, position.y, position.z + side.z);
	int heightL = getObstacleHeightInt(gameMap, left.x, left.z);
	int heightR = getObstacleHeightInt(gameMap, right.x, right.z);
	bool leftCollision = (positionY < heightL) ? true: false;
	bool rightCollision = (positionY < heightR) ? true: false;

	VC3 normal = -dir;
	if(leftCollision && !rightCollision)
		rotateVector(normal, -PI / 4.f);
	else if(!leftCollision && rightCollision)
		rotateVector(normal, PI / 4.f);

	position.x = oldPosition.x;
	position.z = oldPosition.z;
	velocity -= normal * (2 * normal.GetDotWith(velocity));

	if(positionY < floor)
	{
		velocity *= groundSlowFactor;

		if(floor - positionY < meters_0_3)
		{
			velocity.y = -velocity.y;
			position.y = floor * scaleHeight;
		}

		if(fabsf(velocity.y) < 0.02f)
			velocity.y = 0;
	}
	else
	{
		velocity *= wallSlowFactor;
	}

	return true;
}
*/
/*
bool ParticleCollision::getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const
{
	if(!gameMap || !gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;

	VC3 dir = position;
	dir -= oldPosition;
	dir.y = 0;
	float dirLengthSquared = dir.GetSquareLength();
	if(dirLengthSquared > 0.000000001f)
		dir.Normalize();

	int positionY = int(position.y * scale);
	int floor = getFloorHeightInt(gameMap, position.x, position.z);
	int height = getObstacleHeightInt(gameMap, position.x, position.z);
	float scaleHeight = gameMap->getScaleHeight();

	// Avoid flying too high
	if(positionY > floor + meters_3_5 && velocity.y > 0)
	{
		AreaMap *areaMap = gameMap->getAreaMap();

		float wx = position.x;
		float wy = position.z;
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		if(areaMap->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
		{
			velocity = -velocity;
			return true;
		}
	}

	if(positionY > height)
		return false;

	VC3 side(dir.z, dir.y, -dir.x);
	side.x *= 1.5f * gameMap->getScaleX() / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY;
	side.z *= 1.5f * gameMap->getScaleY() / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY;

	VC3 left(position.x - side.x, position.y, position.z - side.z);
	VC3 right(position.x + side.x, position.y, position.z + side.z);
	int heightL = getObstacleHeightInt(gameMap, left.x, left.z);
	int heightR = getObstacleHeightInt(gameMap, right.x, right.z);
	bool leftCollision = (positionY < heightL) ? true: false;
	bool rightCollision = (positionY < heightR) ? true: false;

	VC3 normal = -dir;
	if(leftCollision && !rightCollision)
		rotateVector(normal, -PI / 4.f);
	else if(!leftCollision && rightCollision)
		rotateVector(normal, PI / 4.f);

	position.x = oldPosition.x;
	position.z = oldPosition.z;
	velocity -= normal * (2 * normal.GetDotWith(velocity));

	if(positionY <= floor)
	{
		position.y = floor * scaleHeight;

		velocity.x *= slowFactor;
		velocity.y = -velocity.y * groundSlowFactor;
		velocity.z *= slowFactor;
	}
	else
	{
		VC3 originalPosition = position;

		// We need to push particle back to avoid getting stuck in walls
		// Just an iterative binary search
		{
			VC3 start = position;
			VC3 end = oldPosition;

			position.x = start.x + ((end.x - start.x) * .5f);
			position.y = start.y + ((end.y - start.y) * .5f);
			position.z = start.z + ((end.z - start.z) * .5f);

			const int iterations = 5;
			for(int i = 0; i < iterations; ++i)
			{
				int positionHeight = int(position.y * scale);
				int currentHeight = getObstacleHeightInt(gameMap, position.x, position.z);
				if(positionHeight < currentHeight)
				{
					start.x = position.x;
					start.y = position.y;
					start.z = position.z;
					
					if(i != iterations - 1)
					{
						position.x = start.x + ((end.x - start.x) * .5f);
						position.y = start.y + ((end.y - start.y) * .5f);
						position.z = start.z + ((end.z - start.z) * .5f);
					}
				}
				else
				{
					end.x = position.x;
					end.y = position.y;
					end.z = position.z;
					
					position.x = start.x + ((end.x - start.x) * .5f);
					position.y = start.y + ((end.y - start.y) * .5f);
					position.z = start.z + ((end.z - start.z) * .5f);
				}
			}
		}

		velocity -= normal * (2 * normal.GetDotWith(velocity));
		velocity.x *= wallSlowFactor;
		velocity.y *= slowFactor;
		velocity.z *= wallSlowFactor;
	}

	return true;
}
*/

bool ParticleCollision::getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const
{
	if(!gameMap || !gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;

	VC3 dir = position;
	dir -= oldPosition;
	dir.y = 0;
	float dirLengthSquared = dir.GetSquareLength();
	if(dirLengthSquared > 0.000000001f)
		dir.Normalize();

	int positionY = int(position.y * scale);
	int floor = getFloorHeightInt(gameMap, position.x, position.z);
	int height = getObstacleHeightInt(gameMap, position.x, position.z);
	float scaleHeight = gameMap->getScaleHeight();

	// Avoid flying too high
	if(positionY > floor + meters_3_5 && velocity.y > 0)
	{
		AreaMap *areaMap = gameMap->getAreaMap();

		float wx = position.x;
		float wy = position.z;
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		if(areaMap->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
		{
			velocity = -velocity;
			return true;
		}
	}

	if(positionY > height)
		return false;

	if(positionY <= floor)
	{
		position.y = floor * scaleHeight;

		velocity.x *= slowFactor;
		velocity.y = -velocity.y * groundSlowFactor;
		velocity.z *= slowFactor;
	}
	else
	{
		VC3 rayDir = position - oldPosition;
		float range = rayDir.GetLength();
		if (range == 0.0f)
		{
			return false;
		}
		rayDir.Normalize();

		//Storm3D_CollisionInfo info;
		//scene->rayTrace(oldPosition, rayDir, range, info);
		//const VC3 &normal = info.plane_normal;
		//if(!info.hit)
		//	return false;

		VC3 rayOrigin = oldPosition;
		rayOrigin -= rayDir * 0.3f;
		GameCollisionInfo info;
		scene->rayTrace(rayOrigin, rayDir, 1.f, info, true, false, true, true);
		const VC3 &normal = info.hitPlaneNormal;
		if(!info.hit || info.range - 0.3f > 0.2f)
			return false;

		velocity -= normal * (2 * normal.GetDotWith(velocity));
		float upFactor = normal.y;
		if(upFactor < 0)
			upFactor = 0;

		float yFactor = (upFactor * groundSlowFactor) + ((1.f - upFactor) * slowFactor);
		float xzFactor = (upFactor * slowFactor) + ((1.f - upFactor) * wallSlowFactor);
		velocity.x *= xzFactor;
		velocity.y *= yFactor;
		velocity.z *= xzFactor;

		// Iterative search for position which isnt blocked
		{
			VC3 start = position;
			VC3 end = oldPosition;
			VC3 center = start + ((end - start) * .5f);
			VC3 lastValid = oldPosition;

			const int iterations = 5;
			for(int i = 0; i < iterations; ++i)
			{
				int positionHeight = int(center.y * scale);
				int currentHeight = getObstacleHeightInt(gameMap, center.x, center.z);
				if(positionHeight < currentHeight)
				{
					start = center;
					center = start + ((end - start) * .5f);
				}
				else
				{
					lastValid = center;
					end = center;
					center = start + ((end - start) * .5f);
				}
			}

			position = lastValid;
		}
	}

	return true;
}




FluidParticleCollision::FluidParticleCollision(game::GameMap *gameMap_, game::GameScene *scene_)
:	gameMap(gameMap_),
	scene(scene_),
	scale(0),
	meters_1(0),
	meters_1_5(0),
	meters_3_5(0),
	meters_0_2(0),
	meters_0_3(0)
{
	assert(gameMap);

	scale = 1.f / gameMap->getScaleHeight();
	meters_1 = int(scale);
	meters_1_5 = int(1.5f * scale);
	meters_3_5 = int(3.5f * scale);
	meters_0_2 = int(0.2f * scale);
	meters_0_3 = int(0.3f * scale);
}

FluidParticleCollision::~FluidParticleCollision()
{
}

bool FluidParticleCollision::spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const
{
	return true;
}

bool FluidParticleCollision::getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const
{
	if(!gameMap || !gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;

	VC3 dir = position;
	dir -= oldPosition;
	dir.y = 0;
	float dirLengthSquared = dir.GetSquareLength();
	if(dirLengthSquared > 0.000000001f)
		dir.Normalize();

	int positionY = int(position.y * scale);
	int floor = getFloorHeightInt(gameMap, position.x, position.z);
	int height = getObstacleHeightInt(gameMap, position.x, position.z);
	float scaleHeight = gameMap->getScaleHeight();

	// Avoid flying too high
	if(positionY > floor + meters_3_5 && velocity.y > 0)
	{
		AreaMap *areaMap = gameMap->getAreaMap();

		float wx = position.x;
		float wy = position.z;
		int x = gameMap->scaledToObstacleX(wx);
		int y = gameMap->scaledToObstacleY(wy);

		if(areaMap->isAreaAnyValue(x, y, AREAMASK_INBUILDING))
		{
			velocity = -velocity;
			return true;
		}
	}

	if(positionY > height)
		return false;

	if(positionY <= floor)
	{
		position.y = floor * scaleHeight;

		velocity.x *= slowFactor;
		velocity.y = -velocity.y * groundSlowFactor;
		velocity.z *= slowFactor;
	}
	else
	{
		VC3 rayDir = position - oldPosition;
		float range = rayDir.GetLength();
		if (range == 0.0f)
		{
			return false;
		}
		rayDir.Normalize();

		//Storm3D_CollisionInfo info;
		//scene->rayTrace(oldPosition, rayDir, range, info);
		//const VC3 &normal = info.plane_normal;
		//if(!info.hit)
		//	return false;

		VC3 rayOrigin = oldPosition;
		rayOrigin -= rayDir * 0.3f;
		GameCollisionInfo info;
		scene->rayTrace(rayOrigin, rayDir, 1.f, info, true, false, false, false);
		const VC3 &normal = info.hitPlaneNormal;
		if(!info.hit || info.range - 0.3f > 0.2f)
			return false;

		velocity -= normal * (2 * normal.GetDotWith(velocity));
		float upFactor = normal.y;
		if(upFactor < 0)
			upFactor = 0;

		float yFactor = (upFactor * groundSlowFactor) + ((1.f - upFactor) * slowFactor);
		float xzFactor = (upFactor * slowFactor) + ((1.f - upFactor) * wallSlowFactor);
		velocity.x *= xzFactor;
		velocity.y *= yFactor;
		velocity.z *= xzFactor;

		// Iterative search for position which isnt blocked
		{
			VC3 start = position;
			VC3 end = oldPosition;
			VC3 center = start + ((end - start) * .5f);
			VC3 lastValid = oldPosition;

			const int iterations = 5;
			for(int i = 0; i < iterations; ++i)
			{
				int positionHeight = int(center.y * scale);
				int currentHeight = getObstacleHeightInt(gameMap, center.x, center.z);
				if(positionHeight < currentHeight)
				{
					start = center;
					center = start + ((end - start) * .5f);
				}
				else
				{
					lastValid = center;
					end = center;
					center = start + ((end - start) * .5f);
				}
			}

			position = lastValid;
		}
	}

	return true;

	/*
	if(!gameMap || !gameMap->isWellInScaledBoundaries(position.x, position.z))
		return false;

	VC3 dir = position;
	dir -= oldPosition;
	dir.y = 0;
	float dirLengthSquared = dir.GetSquareLength();
	if(dirLengthSquared > 0.000000001f)
		dir.Normalize();

	int positionY = int(position.y * scale);
	int floor = getFloorHeightInt(gameMap, position.x, position.z);
	int height = getObstacleHeightInt(gameMap, position.x, position.z);
	float scaleHeight = gameMap->getScaleHeight();

	if(positionY > height)
		return false;

	if(positionY <= floor)
	{
		position.y = floor * scaleHeight;

		velocity.x *= slowFactor;
		velocity.y = -velocity.y * groundSlowFactor;
		velocity.z *= slowFactor;
	}
	else
	{
		VC3 rayDir = position - oldPosition;
		float range = rayDir.GetLength();
		if (range == 0.0f)
		{
			return false;
		}
		rayDir.Normalize();

		//Storm3D_CollisionInfo info;
		//scene->rayTrace(oldPosition, rayDir, range, info);
		//const VC3 &normal = info.plane_normal;
		//if(!info.hit)
		//	return false;

		VC3 rayOrigin = oldPosition;
		rayOrigin -= rayDir * 0.3f;
		GameCollisionInfo info;
		scene->rayTrace(rayOrigin, rayDir, 1.f, info, true, false, true);
		const VC3 &normal = info.hitPlaneNormal;
		if(!info.hit || info.range - 0.3f > 0.2f)
			return false;

		velocity -= normal * (2 * normal.GetDotWith(velocity));
		float upFactor = normal.y;
		if(upFactor < 0)
			upFactor = 0;

		float yFactor = (upFactor * groundSlowFactor) + ((1.f - upFactor) * slowFactor);
		float xzFactor = (upFactor * slowFactor) + ((1.f - upFactor) * wallSlowFactor);
		velocity.x *= xzFactor;
		velocity.y *= yFactor;
		velocity.z *= xzFactor;

		// Iterative search for position which isnt blocked
		{
			VC3 start = position;
			VC3 end = oldPosition;
			VC3 center = start + ((end - start) * .5f);
			VC3 lastValid = oldPosition;

			const int iterations = 5;
			for(int i = 0; i < iterations; ++i)
			{
				int positionHeight = int(center.y * scale);
				int currentHeight = getObstacleHeightInt(gameMap, center.x, center.z);
				if(positionHeight < currentHeight)
				{
					start = center;
					center = start + ((end - start) * .5f);
				}
				else
				{
					lastValid = center;
					end = center;
					center = start + ((end - start) * .5f);
				}
			}

			position = lastValid;
		}
	}

	return true;
	*/
}

} // ui
