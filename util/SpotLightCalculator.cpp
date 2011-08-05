
#include "precompiled.h"

#include "SpotLightCalculator.h"
#include "PositionsDirectionCalculator.h"
#include "AngleRotationCalculator.h"
#include "../game/scaledefs.h"
#include <IStorm3D_Terrain.h>
#include <Storm3D_Datatypes.h>

namespace util {
namespace {

	enum { RAY_AMOUNT = 8 };

} // unnamed

struct SpotLightCalculator::Data
{
	const float fov;
	const float range;
	const float squareRange;
	ui::IVisualObjectData *visualData;

	VC3 position;
	VC3 direction;

	// Rays counterclockwise
	mutable float length[RAY_AMOUNT];
	mutable bool lengthOk[RAY_AMOUNT];

	Data(float fov_, float range_, ui::IVisualObjectData *visualData_)
	:	fov(fov_),
		range(range_),
		squareRange(range_ * range_),
		visualData(visualData_)
	{
		for(int i = 0; i < RAY_AMOUNT; ++i)
		{
			length[i] = 0;
			lengthOk[i] = false;
		}
	}

	float getAmount(const VC3 &pos, float angle, const IStorm3D_Terrain &terrain, float rayHeight) const
	{
		float fovRadians = fov * (3.1415927f / 180.f);
		int index = int(RAY_AMOUNT * (angle + fovRadians) / (2.f * fovRadians));
		assert(index >= 0 && index < RAY_AMOUNT);

		float distance = pos.GetRangeTo(position);
		if(!lengthOk[index])
		{
			float rayAngle = -fovRadians + float(index) * ((2 * fovRadians) / (RAY_AMOUNT - 1));

			VC3 rayDir = direction;
			float x = rayDir.x;
			float z = rayDir.z;

			rayDir.x = x * cosf(rayAngle) + z * sinf(rayAngle);
			rayDir.y = 0;
			rayDir.z = -x * sinf(rayAngle) + z * cosf(rayAngle);
			rayDir.Normalize();

			Storm3D_CollisionInfo cInfo;
			ObstacleCollisionInfo oInfo;
			terrain.rayTrace(position + VC3(0,rayHeight,0) + (rayDir * .5f), rayDir, range, cInfo, oInfo, true, true);

			if(oInfo.hit && oInfo.hitAmount > 0)
				length[index] = oInfo.ranges[0];
			else if(cInfo.hit)
				length[index] = cInfo.range;
			else
				length[index] = range;

			lengthOk[index] = true;
		}

		if(distance > length[index])
			return 0;

		return 1.f - distance / range;
	}
};

SpotLightCalculator::SpotLightCalculator(float fov, float range, ui::IVisualObjectData *visualData)
:	data(new Data(fov * .5f, range, visualData))
{
}

SpotLightCalculator::~SpotLightCalculator()
{
}

void SpotLightCalculator::update(const VC3 &position, const VC3 &direction)
{
	data->position = position;
	data->direction = direction;

	for(int i = 0; i < RAY_AMOUNT; ++i)
	{
		data->length[i] = data->range;
		data->lengthOk[i] = false;
	}
}

float SpotLightCalculator::getLightAmount(const VC3 &position, const IStorm3D_Terrain &terrain, float rayHeight) const
{
	float squareRange = position.GetSquareRangeTo(data->position);
	if(squareRange > data->squareRange)
		return 0;

	VC2 pdir(position.x - data->position.x, position.z - data->position.z);
	VC2 sdir(data->direction.x, data->direction.z);

	float pAngle = pdir.CalculateAngle();
	float sAngle = sdir.CalculateAngle();
	float diff = pAngle - sAngle;
	//assert(diff

	if(diff < -3.1415f)
		diff += 3.1415f * 2;
	else if(diff > 3.1415f)
		diff -= 3.1415f * 2;

	if(fabsf(diff) / 3.1415f * 180.f > data->fov)
		return 0;

	return data->getAmount(position, -diff, terrain, rayHeight);
}

ui::IVisualObjectData *SpotLightCalculator::getData() const
{
	return data->visualData;
}

} // util
