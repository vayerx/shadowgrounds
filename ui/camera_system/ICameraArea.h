#ifndef ICAMERAAREA_H
#define ICAMERAAREA_H

#include <DatatypeDef.h>

namespace ui {

#define CAMERA_AREA_COLLISION_NONE 0
#define CAMERA_AREA_COLLISION_PITCH 1
#define CAMERA_AREA_COLLISION_DISTANCE 2
//class Item;

class ICameraArea
{
protected:
public:
	virtual ~ICameraArea() { }
	virtual void reset() = 0;

	 // Sets the position of interest and elapsed time for the camera system. Must be used before querying
	 // camera parameters.
	virtual void update( VC3 positionOfInterest, float currentAngle, int elapsedTime ) = 0;

     // Set the relative camera parameters
	virtual void moveCameraAngle(float a) = 0;
	virtual void moveCameraBetaAngle(float a) = 0;
	virtual void moveCameraTilt(float a) = 0;
	virtual void moveCameraFOV(float fov) = 0;
	virtual void moveCameraDistance(float distance) = 0;
	virtual void moveCameraOffset(VC3 offs) = 0;

	 // Enable/Disable camera collision (0=disabled, 1=enabled)
//	virtual void setCollision(int enabled) = 0;

	 // Query the camera parameters
	virtual float getWeight() = 0;
	virtual float getTransitionWeight() = 0;

	virtual float getCameraAngle() = 0;
	virtual float getCameraBetaAngle() = 0;
	virtual float getCameraBank() = 0;
	virtual float getCameraFOV() = 0;
	virtual float getCameraDistance() = 0;
	virtual VC3 getCameraOffset() = 0;

};

} // end of namespace ui

#endif
