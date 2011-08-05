// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "camera.h"
#include "mouse.h"
#include "storm.h"
#include <istorm3d.h>
#include <datatypedef.h>
#include <windows.h>

//#define TEKES

#pragma warning(disable: 4244) // double to float

namespace frozenbyte {
namespace editor {
namespace {
	bool _hasFocus = false;
	const float SPEED = 100.f;
	float TARGET_DISTANCE = 20.f;

	float timeDelta = 0.f;

	VC2 minValue(-100000.f, -1000000.f);
	VC2 maxValue( 100000.f,  1000000.f);

	VC2 getClampedPosition(const VC2 &pos)
	{
		VC2 result = pos;

		if(result.x < minValue.x)
			result.x = minValue.x;
		if(result.x > maxValue.x)
			result.x = maxValue.x;
		if(result.y < minValue.y)
			result.y = minValue.y;
		if(result.y > maxValue.y)
			result.y = maxValue.y;

		return result;
	}
}

struct CameraData
{
	Storm &storm;

	Vector targetPosition;
	float xAngle;
	float yAngle;
	float height;

	bool beginDrag;
	int timeValue;
	bool gameCamera;

	VC3 cameraPos;

	CameraData(Storm &storm_)
	:	storm(storm_),
		gameCamera(false)
	{
		timeValue = 0;
		timeDelta = 0;

		targetPosition = Vector(0, 100, 0);
		xAngle = yAngle = 0;
		height = 20;

		beginDrag = true;
		timeBeginPeriod(1);
	}

	~CameraData()
	{
		timeEndPeriod(1);
	}

	bool isKeyDown(int key)
	{
		if(!_hasFocus) return false;

		if(GetKeyState(key) & 0x80)
			return true;

		return false;
	}

	void update()
	{
		// Time
		if(timeValue == 0)
			timeValue = timeGetTime(); //GetTickCount();

		int newTime = timeGetTime(); //GetTickCount();
		timeDelta = (float) (newTime - timeValue) / 1000.0f;
		if(timeDelta > 0.1f)
			timeDelta = 0.1f;

		timeValue = newTime;
	}

	float getTimeDelta()
	{
		return timeDelta;
	}
};

Camera::Camera(Storm &storm)
{
	boost::scoped_ptr<CameraData> tempData(new CameraData(storm));
	data.swap(tempData);
}

Camera::~Camera()
{
}

void Camera::forceGameCamera(bool force)
{
	data->gameCamera = force;
}

const VC3 &Camera::getPosition() const
{
	static VC3 pos;

	VC3 dir = data->targetPosition - data->cameraPos;
	dir.Normalize();

	pos = data->cameraPos + (dir * 3.f);

	return pos;
}

void Camera::nudgeCamera(const VC3 &direction)
{
	data->cameraPos += direction;
	data->targetPosition += direction;
}

void Camera::update(const Mouse &mouse, bool hasFocus)
{
#ifdef TEKES
	return;
#endif

	_hasFocus = hasFocus;
	
	data->update();
	float timeDelta = data->getTimeDelta();

	// got fed up with way too fast camera. -jpk ;)
	float multiplier = 0.5f;
	if(!data->isKeyDown(VK_SHIFT))
		multiplier = .075f;

	if(data->isKeyDown(VK_CONTROL))
		multiplier = .02f;

	if(!data->storm.terrain)
	{
		multiplier = .075f;
		
		if(!data->isKeyDown(VK_SHIFT))
			multiplier = .01f;
	}

#ifndef PROJECT_VIEWER
#ifdef GAME_SIDEWAYS
	{
		if(mouse.isRightButtonDown())
		{
			static int oldX = mouse.getX();
			static int oldY = mouse.getY();

			int newX = mouse.getX();
			int newY = mouse.getY();

			if(data->beginDrag)
			{
				oldX = newX;
				oldY = newY;

				data->beginDrag = false;
			}

			data->xAngle += (newY - oldY) * .004f;
			data->yAngle += (newX - oldX) * -.004f;

			oldX = newX;
			oldY = newY;
		}
		else
			data->beginDrag = true;

		if(data->xAngle > PI/2.3f)
			data->xAngle = PI/2.3f;
		if(data->xAngle < -PI/2.3f)
			data->xAngle = -PI/2.3f;

		if(data->yAngle > PI/2.3f)
			data->yAngle = PI/2.3f;
		if(data->yAngle < -PI/2.3f)
			data->yAngle = -PI/2.3f;

		if(data->isKeyDown('W'))
			data->targetPosition.z += timeDelta * SPEED * multiplier;
		if(data->isKeyDown('S'))
			data->targetPosition.z -= timeDelta * SPEED * multiplier;
		if(data->isKeyDown('D'))
			data->targetPosition.x += timeDelta * SPEED * multiplier;
		if(data->isKeyDown('A'))
			data->targetPosition.x -= timeDelta * SPEED * multiplier;

		if(data->isKeyDown(VK_SUBTRACT) || data->isKeyDown(VK_MULTIPLY))
			data->height +=  1.f * timeDelta * SPEED * .5f * multiplier;
		if(data->isKeyDown(VK_ADD))
			data->height -=  1.f * timeDelta * SPEED * .5f * multiplier;

		if (data->height < 1.0f)
			data->height = 1.0f;
		if (data->height > 1000.0f)
			data->height = 1000.0f;

		if(!data->storm.viewerCamera && data->storm.terrain)
		{
			float height1 = data->storm.terrain->getHeight(getClampedPosition(VC2(data->targetPosition.x, data->targetPosition.z)));
			data->targetPosition.y = height1;
		}

		QUAT rotation;
		rotation.MakeFromAngles(data->xAngle, 0, data->yAngle);
		VC3 targVec = VC3(0, -1 * data->height, 0);
		rotation.RotateVector(targVec);
		VC3 pos = data->targetPosition - targVec;
		VC3 targ = data->targetPosition;
		data->storm.scene->GetCamera()->SetUpVec(VC3(0,0,1));
		data->storm.scene->GetCamera()->SetPosition(pos);
		data->storm.scene->GetCamera()->SetTarget(targ);

		data->cameraPos = pos;
		data->targetPosition = targ;

		return;
	}
#endif
#endif


//	if(data->storm.terrain)
	if(hasFocus && mouse.isRightButtonDown())
	{
		static int oldX = mouse.getX();
		static int oldY = mouse.getY();

		int newX = mouse.getX();
		int newY = mouse.getY();

		if(data->beginDrag)
		{
			oldX = newX;
			oldY = newY;

			data->beginDrag = false;
		}

		data->xAngle += (newY - oldY) * .003f;
		data->yAngle += (newX - oldX) * .003f;

		oldX = newX;
		oldY = newY;
	}
	else
		data->beginDrag = true;

	if(data->xAngle > PI/2.01f)
		data->xAngle = PI/2.01f;
	if(data->xAngle < -.5f)
		data->xAngle = -.5f;

	if(data->yAngle > 2*PI)
		data->yAngle -= 2*PI;
	if(data->yAngle < 0)
		data->yAngle += 2*PI;

	if(data->gameCamera)
		data->xAngle = 60.f * 3.1415f / 180.f;

{
	bool rotateH = false;
	float xOtherAngle = 0.f;
	if(data->isKeyDown('T'))
	{
		rotateH = true;
		xOtherAngle -= timeDelta;
	}
	if(data->isKeyDown('G'))
	{
		rotateH = true;
		xOtherAngle += timeDelta;
	}

	float limitMin = -0.49f;
	float limitMax = PI/2.02f;
	if(rotateH && (data->xAngle + xOtherAngle) >= limitMin && (data->xAngle + xOtherAngle) <= limitMax)
	{
		if(data->xAngle <= limitMin)
		{
			data->xAngle += xOtherAngle;
		
			if(data->xAngle <= limitMin)
				data->xAngle = limitMin;
		}
		if(data->xAngle >= limitMax)
		{
			data->xAngle += xOtherAngle;
		
			if(data->xAngle >= limitMax)
				data->xAngle = limitMax;
		}
		else
		{
			QUAT rrotation;
			rrotation.MakeFromAngles(0, data->yAngle, -data->xAngle);
			Vector direction(1.f, 0, 0);
			direction = rrotation.GetRotated(direction);
			
			VC3 pos = data->targetPosition + direction * TARGET_DISTANCE;

			QUAT rotation;
			rotation.MakeFromAngles(0, data->yAngle, -data->xAngle - xOtherAngle);
			VC3 otherDirection(1.f, 0, 0);
			otherDirection = rotation.GetRotated(otherDirection);

			data->xAngle += xOtherAngle;
			VC3 newPos = pos - otherDirection * TARGET_DISTANCE;
			
			if(xOtherAngle < 0)
				data->height += fabsf(newPos.y - data->targetPosition.y);
			else
				data->height -= fabsf(newPos.y - data->targetPosition.y);

			data->targetPosition = newPos;
		}
	}
}

	QUAT rotation;
	rotation.MakeFromAngles(0, data->yAngle, -data->xAngle);
	Vector direction(1.f, 0, 0);
	direction = rotation.GetRotated(direction);

	Vector moveDirection = -direction;
	moveDirection.y = 0;
	moveDirection.Normalize();

	Vector strafeDirection = moveDirection.GetCrossWith(Vector(0,1,0));

	if(data->isKeyDown('W'))
		data->targetPosition += moveDirection * timeDelta * SPEED * multiplier;
	if(data->isKeyDown('S'))
		data->targetPosition -= moveDirection * timeDelta * SPEED * multiplier;
	if(data->isKeyDown('D'))
		data->targetPosition -=  strafeDirection * timeDelta * SPEED * multiplier;
	if(data->isKeyDown('A'))
		data->targetPosition +=  strafeDirection * timeDelta * SPEED * multiplier;

	// and had enough of reversed zoom too :) -jpk 
	if(data->isKeyDown(VK_SUBTRACT) || data->isKeyDown(VK_MULTIPLY))
		data->height +=  1.f * timeDelta * SPEED * .5f * multiplier;
	if(data->isKeyDown(VK_ADD))
		data->height -=  1.f * timeDelta * SPEED * .5f * multiplier;

	Vector position = data->targetPosition + direction * TARGET_DISTANCE;
	
	float yOtherAngle = data->yAngle + PI;
	bool rotate = false;
	if(data->isKeyDown('Q'))
	{
		yOtherAngle -= timeDelta;
		rotate = true;
	}
	if(data->isKeyDown('E'))
	{
		yOtherAngle += timeDelta;
		rotate = true;
	}

	data->yAngle = yOtherAngle + PI;

	rotation.MakeFromAngles(0, yOtherAngle, data->xAngle);
	Vector otherDirection(1.f, 0, 0);
	otherDirection = rotation.GetRotated(otherDirection);

	if(rotate)
	{
		data->targetPosition.x = position.x + otherDirection.x * TARGET_DISTANCE;
		data->targetPosition.z = position.z + otherDirection.z * TARGET_DISTANCE;
	}

	if(!data->storm.viewerCamera && data->storm.terrain)
	{
		/*
		float height1 = 0.f; //data->storm.terrain->getHeight(getClampedPosition(VC2(data->targetPosition.x, data->targetPosition.z)));
		float height2 = 0.f; //data->storm.terrain->getHeight(getClampedPosition(VC2(position.x + otherDirection.x, position.z + otherDirection.z)));

		float targetHeight = height1;
		targetHeight += data->height;
		*/

		float delta = data->height - data->targetPosition.y;
		data->targetPosition.y += delta;
		position.y += delta;

		if(data->gameCamera)
		{
			float height1 = data->storm.terrain->getHeight(getClampedPosition(VC2(data->targetPosition.x, data->targetPosition.z)));
			VC3 pos = position;
			pos.y = height1 + 11.5f;
			VC3 target = pos + otherDirection;

			data->storm.scene->GetCamera()->SetPosition(pos);
			data->storm.scene->GetCamera()->SetTarget(target);
			data->storm.scene->GetCamera()->SetFieldOfView(120.f / 2.f / 180.f * 3.1451f);

			data->cameraPos = pos;
			//data->targetPosition = target;
		}
		else
		{
			data->storm.scene->GetCamera()->SetPosition(position);
			data->storm.scene->GetCamera()->SetTarget(data->targetPosition);
			data->cameraPos = position;
		}
	}
	else
	{
		position.y = data->height;
		data->storm.scene->GetCamera()->SetPosition(position - (otherDirection * 3.f));
		data->storm.scene->GetCamera()->SetTarget(position);

		data->cameraPos = position;
	
		/*
		float height1 = 0.f;
		float height2 = 0.f;
		float targetHeight = height1;
		targetHeight += data->height;

		float newHeight = data->targetPosition.y + ((targetHeight - data->targetPosition.y) / 16);
		float delta = newHeight - data->targetPosition.y;
		
		data->targetPosition.y += delta;
		position.y += delta;

		if(position.y < height2 + 2)
			position.y = height2 + 2;

		data->storm.scene->GetCamera()->SetPosition(position);
		data->storm.scene->GetCamera()->SetTarget(data->targetPosition);
		*/
	}
}

void Camera::setToOrigo()
{
	data->yAngle = 3.14f - PI/2.f;
	data->xAngle = 0;
	data->targetPosition = Vector(0, 0, TARGET_DISTANCE - 10);
	data->height = 2;

#ifdef TEKES
	data->height = 1.5f;
	data->targetPosition = Vector(4.f, 0, 10);
	data->yAngle += PI/8.f;

	data->storm.scene->GetCamera()->SetPosition(VC3(-2.f, 1.4f, -8.f));
	data->storm.scene->GetCamera()->SetTarget(VC3(0, 1.4f, 0));

#endif

}

float Camera::getHorizontal()
{
	float timeDelta = data->getTimeDelta();
	if(data->isKeyDown(VK_LEFT))
		return timeDelta;
	if(data->isKeyDown(VK_RIGHT))
		return -timeDelta;

	return 0;
}

float Camera::getVertical()
{
	float timeDelta = data->getTimeDelta();
	if(data->isKeyDown(VK_UP))
		return timeDelta;
	if(data->isKeyDown(VK_DOWN))
		return -timeDelta;

	return 0;
}

void Camera::setToSky() 
{
	//data->yAngle = 0.0f;
	//data->height = 80.0f;
	//data->xAngle = 0.f;

	data->yAngle = 0.0f;
	data->height = 20.0f;
	data->xAngle = PI/3.f;

	data->targetPosition = Vector(0.0f, 0.0f, 80.0f);
}

float getTimeDelta()
{
	return timeDelta;
}

void setBoundary(const VC2 &min_, const VC2 &max_)
{
	minValue = min_;
	maxValue = max_;
}

} // end of namespace editor
} // end of namespace frozenbyte

