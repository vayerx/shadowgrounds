// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "align_units.h"
#include "camera.h"
#include <istorm3d_scene.h>
#include <windows.h>

namespace frozenbyte {
namespace editor {

struct AlignUnitsData
{
	float gridX;
	float gridY;
	float gridHeight;

	AlignUnitsData()
	:	gridX(.25f),
		gridY(.25f),
		gridHeight(.1f)
	{
	}
};

AlignUnits::AlignUnits()
{
	boost::scoped_ptr<AlignUnitsData> tempData(new AlignUnitsData());
	data.swap(tempData);
}

AlignUnits::~AlignUnits()
{
}

void AlignUnits::getGridSize(float &x, float &y) const
{
	x = data->gridX;
	y = data->gridY;
}

void AlignUnits::setGridSize(float x, float y)
{
	data->gridX = x;
	data->gridY = y;
}

VC3 AlignUnits::getAlignedPosition(const VC3 &position) const
{
	VC3 result(position.x, position.y, position.z);

	if((GetKeyState(VK_MENU) & 0x80))
	{
		float gx = data->gridX / sqrtf(2.f);
		float gy = data->gridY / sqrtf(2.f);

		result.x = int(result.x / gx) * gx;
		result.z = int(result.z / gy) * gy;
	}
	else if((GetKeyState(VK_CONTROL) & 0x80) == 0)
	{
		result.x = int(result.x / data->gridX) * data->gridX;
		result.z = int(result.z / data->gridY) * data->gridY;
	}

	return result;
}

VC2 AlignUnits::getAlignedPosition(const VC2 &position) const
{
	VC3 result = getAlignedPosition(VC3(position.x, 0, position.y));
	return VC2(result.x, result.z);	
}

float AlignUnits::getRotation(float oldRotation, int delta, bool cap) const
{
	float result = oldRotation;

	float factor = 750.f;
	if((GetKeyState(VK_SHIFT) & 0x80) == 0)
		factor = 10000.f;

	if(delta >= 0 || !cap)
		result += delta / factor;
	else
		result = (int(result / (PI / 4) + 0.01f) % 8) * (PI / 4);

	while(result > 2*PI)
		result -= 2*PI;
	while(result < -2*PI)
		result += 2*PI;

	return result;
}

float AlignUnits::getHeight(const float &height, int delta) const
{
	// old...
	/*
	float factor = .25f;
	if((GetKeyState(VK_SHIFT) & 0x80))
		factor = 1.f;

	height += float(delta) * factor * getTimeDelta();
	//if((GetKeyState(VK_HOME) & 0x80))
	//	height = 0;

	float result = height;
	result = int(result / data->gridHeight) * data->gridHeight;  // <-- see the bug here. only result affected, not originally non-const height ;)
	*/

	// new...

	float result = height;

	float factor = 1.0f;
	bool useGrid = false;

	if((GetKeyState(VK_SHIFT) & 0x80))
		factor = 10.0f;

	// if ctrl was not down, use grid to move objects
	if((GetKeyState(VK_CONTROL) & 0x80) == 0)
		useGrid = true;

	static int lastGridMoveTime = 0;
	long curTime = timeGetTime();

	if (useGrid)
	{
		if (curTime < lastGridMoveTime + 100)
		{
			factor = 0.0f;
		}
	} else {
		factor = 0.1f;
		if (curTime < lastGridMoveTime + 50)
		{
			factor = 0.0f;
		}
	}

	if (factor != 0.0f)
	{
		result += factor * data->gridHeight * delta;
		lastGridMoveTime = curTime;
	}

	// snap to grid.
	if (useGrid)
	{
		// (a little epsilon, to make sure rounding errors won't get in the way when snapping to grid)
		if (result < 0.0001f)
		{
			result = int((result - 0.0001f) / data->gridHeight) * data->gridHeight;
		}
		else if (result > 0.0001f)
		{
			result = int((result + 0.0001f) / data->gridHeight) * data->gridHeight;
		}
	}

	return result;
}

VC3 AlignUnits::getMovedPosition(const VC3 &position, IStorm3D_Camera &camera, bool *updated) const
{
	VC3 result = position;
	float factor = .005f;
	bool useGrid = false;

	if((GetKeyState(VK_SHIFT) & 0x80))
		factor = .2f;

	if((GetKeyState(VK_CONTROL) & 0x80) == 0)
		useGrid = true;

	factor *= getTimeDelta();

	VC3 y = camera.GetTarget() - camera.GetPosition();
	y.y = 0;
	y.Normalize();
	VC3 x = VC3(0.f, 1.f, 0.f).GetCrossWith(y);

#ifdef PROJECT_AOV
	x = VC3(1, 0, 0);
	y = VC3(0, 0, 1);
#endif

	y *= 40.f;
	x *= 40.f;

	// new: if ctrl was not down, use grid to move objects
	static int lastGridMoveTime = 0;
	long curTime = timeGetTime();

	if (useGrid)
	{
		if (fabsf(x.x) > fabsf(x.z))
		{
			x = VC3(x.x,0,0);
			y = VC3(0,0,y.z);
		} else {
			x = VC3(0,0,x.z);
			y = VC3(y.x,0,0);
		}
		if (x.GetSquareLength() > 0.0001f)
		{
			x.Normalize();
			x *= data->gridX;
		} else {
			x = VC3(0,0,0);
		}
		if (y.GetSquareLength() > 0.0001f)
		{
			y.Normalize();
			y *= data->gridY;
		} else {
			y = VC3(0,0,0);
		}

		if((GetKeyState(VK_SHIFT) & 0x80))
		{
//#ifdef PROJECT_AOV
//			x *= 10.0f; 
//			y *= 10.0f; 
//#else
			x *= 10.0f; 
			y *= 10.0f; 
//#endif
		}

		if (curTime < lastGridMoveTime + 100)
		{
			factor = 0.0f;
		} else {
			factor = 1.0f;
		}
	}
	// end of new grid thingy

	if(updated)
		*updated = false;

	if (factor != 0.0f)
	{
		if((GetKeyState(VK_UP) & 0x80))
		{
			result += y * factor;
			if(updated)
				*updated = true;
			lastGridMoveTime = curTime;
		}
		if((GetKeyState(VK_DOWN) & 0x80))
		{
			result -= y * factor;
			if(updated)
				*updated = true;
			lastGridMoveTime = curTime;
		}
		if((GetKeyState(VK_RIGHT) & 0x80))
		{
			result += x * factor;
			if(updated)
				*updated = true;
			lastGridMoveTime = curTime;
		}
		if((GetKeyState(VK_LEFT) & 0x80))
		{
			result -= x * factor;
			if(updated)
				*updated = true;
			lastGridMoveTime = curTime;
		}
	}

	return result;
}

} // end of namespace editor
} // end of namespace frozenbyte
