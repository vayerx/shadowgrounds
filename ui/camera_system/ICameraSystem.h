#ifndef ICAMERASYSTEM_H
#define ICAMERASYSTEM_H

#include <DatatypeDef.h>
#include "ICameraArea.h"

namespace ui {

//class Item;

class ICameraSystem
{
protected:
public:
	virtual ~ICameraSystem() { }
	 // Sets the position of interest and elapsed time for the camera system. Must be used before querying
	 // camera parameters.

	virtual void clearCameraAreas() = 0;
	virtual void addCameraArea(ICameraArea *cameraArea) = 0;

	virtual void setMode(int mode) = 0;
	virtual void setAimPosition(VC3 pos) = 0;

	virtual void update( VC3 positionOfInterest, int elapsedTime ) = 0;

     // Set the absolute camera parameters
	virtual void setCameraAngle(float a) = 0;
	virtual void setCameraBetaAngle(float a) = 0;
	virtual void setCameraBank(float a) = 0;
	virtual void setCameraFOV(float fov) = 0;
	virtual void setCameraDistance(float distance) = 0;
	virtual void setCameraOffset(VC3 offs) = 0;


     // Set the relative camera parameters
	virtual void moveCameraAngle(float a) = 0;
	virtual void moveCameraBetaAngle(float a) = 0;
	virtual void moveCameraBank(float a) = 0;
	virtual void moveCameraFOV(float fov) = 0;
	virtual void moveCameraDistance(float distance) = 0;
	virtual void moveCameraOffset(VC3 offs) = 0;

	 // Enable/Disable camera collision (0=disabled, 1=enabled)
	virtual void setCollision(int enabled) = 0;

	virtual void setResponse(float response) = 0;

	 // Query the camera parameters
	virtual VC3 getCameraPosition() = 0;
	virtual VC3 getTargetPosition() = 0;
	virtual VC3 getUpVector() = 0;
	virtual float getFOV() = 0;

};

} // end of namespace ui

#endif
