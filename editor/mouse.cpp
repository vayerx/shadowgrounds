// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "mouse.h"
#include "storm.h"
#include <cassert>
#include <datatypedef.h>
#include <istorm3d_scene.h>
#include <istorm3d_terrain.h>

#include <string>
#include <set>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace editor {

Mouse::Mouse()
{
	window = 0;
	x = 0;
	y = 0;

	leftButtonDown = false;
	leftButtonClicked = false;
	rightButtonDown = false;
	rightButtonClicked = false;
	
	insideWindow = false;
	wheelDelta = 0;

	storm = 0;
}

Mouse::~Mouse()
{
}

void Mouse::setTrackWindow(HWND windowHandle)
{
	window = windowHandle;
}

void Mouse::setStorm(Storm &storm_)
{
	storm = &storm_;
}

void Mouse::update()
{
	assert(window);
	
	POINT p = { 0 };
	RECT r = { 0 };

	leftButtonClicked = false;
	rightButtonClicked = false;
	wheelDelta = 0;

	if(GetCursorPos(&p))
	if(MapWindowPoints(HWND_DESKTOP, window, &p, 1))
	{
		x = p.x;
		y = p.y;

		GetClientRect(window, &r);
		if((x < r.left) || (y < r.top) || (x > r.right) || (y > r.bottom))
			insideWindow = false;
		else 
			insideWindow = true;

		return;
	}

	insideWindow = false;
}

void Mouse::setLeftButtonDown()
{
	leftButtonDown = true;
}

void Mouse::setLeftButtonUp()
{
	leftButtonDown = false;
	leftButtonClicked = true;
}

void Mouse::setRightButtonDown()
{
	rightButtonDown = true;
}

void Mouse::setRightButtonUp()
{
	rightButtonDown = false;
	rightButtonClicked = true;
}

void Mouse::setWheelDelta(int delta)
{
	wheelDelta = delta;
}

int Mouse::getX() const
{
	return x;
}

int Mouse::getY() const
{
	return y;
}

int Mouse::getWheelDelta() const
{
	return wheelDelta;
}

bool Mouse::isLeftButtonDown() const
{
	return leftButtonDown;
}

bool Mouse::hasLeftClicked() const
{
	return leftButtonClicked;
}

bool Mouse::isRightButtonDown() const
{
	return rightButtonDown;
}

bool Mouse::hasRightClicked() const
{
	return rightButtonClicked;
}

bool Mouse::isInsideWindow() const
{
	return insideWindow;
}

bool Mouse::cursorRayTrace(Storm3D_CollisionInfo &ci, VC3 *position, VC3 *direction)
{
	if(!storm)
		return false;

	VC3 fooP, fooD;
	if(!position)
		position = &fooP;
	if(!direction)
		direction = &fooD;

	VC2I screen(getX(), getY());
	storm->scene->GetEyeVectors(screen, *position, *direction);

	set<weak_ptr<IStorm3D_Model> >::iterator it = storm->floorModels.begin();
	for(; it != storm->floorModels.end(); ++it)
	{
		shared_ptr<IStorm3D_Model> m = it->lock();
		if(!m)
			continue;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			string name = object->GetName();
			if(name.find("BuildingFloor") == name.npos)
				continue;

			bool oldNoCollision = object->GetNoCollision();
			if(oldNoCollision != false)
				object->SetNoCollision(false);

			object->RayTrace(*position, *direction, 1000.f, ci, true);
		}
	}

	if(ci.hit)
		return true;

	ObstacleCollisionInfo oi;
	storm->terrain->rayTrace(*position, *direction, 1000.f, ci, oi, true);

	return ci.hit;
}

} // end of namespace editor
} // end of namespace frozenbyte
