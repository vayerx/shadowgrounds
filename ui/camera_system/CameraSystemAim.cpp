#include "precompiled.h"
#include "CameraSystemAim.h"

#include <boost/lexical_cast.hpp>
#include "../../system/Logger.h"

#include "../../game/SimpleOptions.h"
#include "../../game/options/options_camera.h"



#define DAMPER_OVERDAMP_FACTOR 1.5f

namespace ui {

CameraSystemAim::CameraSystemAim()
{
	positionOfInterest = VC3(0, 0, 0);
	angle = 0;
	betaAngle = 0;
	bank = 0;
	cameraPosition = VC3(0, 0, 1);
	targetPosition = VC3(0, 0, 0);
	upVector = VC3(0, 1, 0);
	FOV = 90;
	offset = VC3(0,2,0);
	distance = 10;

	response = 10;

	mode = 0;
}

CameraSystemAim::~CameraSystemAim()
{
}

void CameraSystemAim::setMode(int mode)
{
}

void CameraSystemAim::setAimPosition(VC3 pos)
{
	this->aimPosition = pos;
}

void CameraSystemAim::addCameraArea(ICameraArea *cameraArea)
{
}

// Sets the position of interest and elapsed time for the camera system. Must be used before querying
// camera parameters.
void CameraSystemAim::update( VC3 positionOfInterest, int elapsedTime )
{

	cameraPosition = positionOfInterest + VC3(0, 1.395f, 0);
	targetPosition = aimPosition;
	targetPosition.y = cameraPosition.y;
	cameraPosition = cameraPosition + (cameraPosition-targetPosition).GetNormalized()*1.687f;
	upVector = VC3(0, 1, 0);
	QUAT tmp_quat3( (targetPosition-cameraPosition).GetNormalized(), DEG2RAD( -7.7f ));
	tmp_quat3.RotateVector( upVector );
	FOV = 70.0f;
}


// Set the absolute camera parameters
void CameraSystemAim::setCameraAngle(float a)
{
}

void CameraSystemAim::setCameraBetaAngle(float a)
{
}

void CameraSystemAim::setCameraBank(float a)
{
}

void CameraSystemAim::setCameraFOV(float fov)
{
}

void CameraSystemAim::setCameraDistance(float distance)
{
}

void CameraSystemAim::setCameraOffset(VC3 offs)
{
}


// Set the relative camera parameters
void CameraSystemAim::moveCameraAngle(float a)
{
}

void CameraSystemAim::moveCameraBetaAngle(float a)
{
}

void CameraSystemAim::moveCameraBank(float a)
{
}

void CameraSystemAim::moveCameraFOV(float fov)
{
}

void CameraSystemAim::moveCameraDistance(float distance)
{
}

void CameraSystemAim::moveCameraOffset(VC3 offs)
{
}


// Enable/Disable camera collision (0=disabled, 1=enabled)
void CameraSystemAim::setCollision(int enabled)
{
}

void CameraSystemAim::setResponse(float response)
{
}

// Query the camera parameters
VC3 CameraSystemAim::getCameraPosition()
{
	return( cameraPosition );
}

VC3 CameraSystemAim::getTargetPosition()
{
	return( targetPosition );
}

VC3 CameraSystemAim::getUpVector()
{
	return( upVector );
}

float CameraSystemAim::getFOV()
{
	return( FOV );
}



} // end of namespace ui
