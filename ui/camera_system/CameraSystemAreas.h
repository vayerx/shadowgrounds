#ifndef CAMERASYSTEMAREAS_H
#define CAMERASYSTEMAREAS_H

#include "ICameraSystem.h"
#include "../../util/Dampers.h"

namespace ui {

#define MAX_CAMERA_AREAS 1024

class CameraSystemAreas : public ICameraSystem
{
protected:
    VC3 positionOfInterest;
	VC3 aimPosition;

	int mode;
	int modeChangeTimer;

	float angle;
	float betaAngle;
	float bank;
	float FOV;
	VC3 offset;
	VC3 cameraPosition;
	VC3 targetPosition;
	VC3 upVector;
	float distance;
	float response;

	ICameraArea * cameraArea[MAX_CAMERA_AREAS];
	int cameraAreas;
	int lastCameraArea;

	util::VectorDamper posDamper;
	util::VectorDamper targetDamper;
	util::VectorDamper upDamper;
	util::VectorDamper attributeDamper;

public:
	CameraSystemAreas();
	virtual ~CameraSystemAreas();

	virtual void clearCameraAreas() {};
	virtual void addCameraArea(ICameraArea *cameraArea);

	 // Sets the position of interest and elapsed time for the camera system. Must be used before querying
	 // camera parameters.
	virtual void update( VC3 positionOfInterest, int elapsedTime );


	virtual void setMode(int mode);
	virtual void setAimPosition(VC3 pos);

     // Set the absolute camera parameters
	virtual void setCameraAngle(float a);
	virtual void setCameraBetaAngle(float a);
	virtual void setCameraBank(float a);
	virtual void setCameraFOV(float fov);
	virtual void setCameraDistance(float distance);
	virtual void setCameraOffset(VC3 offs);

     // Set the relative camera parameters
	virtual void moveCameraAngle(float a);
	virtual void moveCameraBetaAngle(float a);
	virtual void moveCameraBank(float a);
	virtual void moveCameraFOV(float fov);
	virtual void moveCameraDistance(float distance);
	virtual void moveCameraOffset(VC3 offs);

	 // Enable/Disable camera collision (0=disabled, 1=enabled)
	virtual void setCollision(int enabled);

	virtual void setResponse(float response);

	 // Query the camera parameters
	virtual VC3 getCameraPosition();
	virtual VC3 getTargetPosition();
	virtual VC3 getUpVector();
	virtual float getFOV();

};

} // end of namespace ui

#endif // CAMERASYSTEMAREAS_H
