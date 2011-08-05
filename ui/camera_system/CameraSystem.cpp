#include "precompiled.h"
#include "CameraSystem.h"

#include "CameraSystemAreas.h"
#include "CameraSystemAim.h"

#include <boost/lexical_cast.hpp>
#include "../../system/Logger.h"

#include "../../game/SimpleOptions.h"
#include "../../game/options/options_camera.h"



#define DAMPER_OVERDAMP_FACTOR 1.5f

namespace ui {

CameraSystem::CameraSystem()
{

	cameraMode[0] = 0;
	cameraMode[1] = 0;

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

	cameraModes  = 2;
	cameraMode[0] = new CameraSystemAreas();
	cameraMode[1] = new CameraSystemAim();
	response = 10;


	mode = -1;
	modeChangeTimer = 0;
	setMode(0);
	modeChangeTime = 0;
	oldMode = 0;
}

CameraSystem::~CameraSystem()
{
	for (int i=0; i<cameraModes;i++) {
		if ( cameraMode[i] )
			delete cameraMode[i];
	}
}

void CameraSystem::clearCameraAreas()
{
	if ( cameraMode[0] )
		delete cameraMode[0];

	cameraMode[0] = new CameraSystemAreas();
}

void CameraSystem::setMode(int mode)
{
	if (this->mode == mode) return;
	if (mode>=cameraModes) return;
	oldMode = this->mode;
	this->mode = mode;
	for (int i=0; i<cameraModes; i++) {
		modeWeight[i] = 0;
	}
	modeWeight[mode] = 1;
	if (mode==0) modeChangeTime = 4000;
	if (mode==1) modeChangeTime = 600;
	modeChangeTimer = 0;
/*	if (mode == 0) {
		// If returning to default mode, keep camera direction from the old mode
        VC3 dir = getTargetPosition()-getCameraPosition();
		VC3 flatDir(dir.x, 0.0f, dir.z);
        VC3 zeroAngle(0.0f, 0.0f, 1.0f);
		VC3 upDir(0.0f, 1.0f, 0.0f);

		float tmp = -180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
		if (flatDir.GetCrossWith(zeroAngle).y < 0) {
			tmp = 360-tmp;
		}
		tmp += 90.0f;
		if (tmp >= 360.0f) tmp -= 360.0f;
		moveCameraAngle(tmp-angle);
	}*/
}

void CameraSystem::setAimPosition(VC3 pos)
{
/*	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->setAimPosition(pos);
	}*/
	cameraMode[mode]->setAimPosition(pos);
	this->aimPosition = pos;
}

void CameraSystem::addCameraArea(ICameraArea *cameraArea)
{
	cameraMode[0]->addCameraArea(cameraArea);
}


// Sets the position of interest and elapsed time for the camera system. Must be used before querying
// camera parameters.
void CameraSystem::update( VC3 positionOfInterest, int elapsedTime )
{
	if (modeChangeTimer<modeChangeTime) {
		modeChangeTimer += elapsedTime;
//		std::string foofoo = "TIMER: ";
//		foofoo = foofoo + boost::lexical_cast< std::string >(modeChangeTimer) + ", ";
//		Logger::getInstance()->error( foofoo.c_str() );
		if ((modeChangeTime == 0) || (modeChangeTimer>modeChangeTime)) {
			modeWeight[oldMode] = 0;
			modeWeight[mode] = 1;
		} else {
			float s = float(modeChangeTimer)/float(modeChangeTime);
			modeWeight[oldMode] = 0.5f+0.5f*cos(s*PI);
			modeWeight[mode] = 1-(0.5f+0.5f*cos(s*PI));
//			std::string foofoo2 = "";
//			foofoo2 = foofoo2 + boost::lexical_cast< std::string >(s) + ", ";
//			foofoo2 = foofoo2 + boost::lexical_cast< std::string >(modeWeight[mode]) + ", ";
//			foofoo2 = foofoo2 + boost::lexical_cast< std::string >(modeWeight[oldMode]) + ", ";
//			Logger::getInstance()->error( foofoo2.c_str() );
		}
	}
	// Update camera areas
	for (int i=0; i<cameraModes; ++i){
		if (modeWeight[i]>0) cameraMode[i]->update(positionOfInterest, elapsedTime);
	}

	// Interpolate camera parameters from camera modes
	cameraPosition = VC3(0.0f, 0.0f, 0.0f);
	targetPosition = VC3(0.0f, 0.0f, 0.0f);
	upVector = VC3(0.0f, 0.0f, 0.0f);
	FOV = 0;
	for (int i=0; i<cameraModes; ++i){
		float w = modeWeight[i];
		if (w>0) {
			cameraPosition = cameraPosition+cameraMode[i]->getCameraPosition()*w;
			targetPosition = targetPosition+cameraMode[i]->getTargetPosition()*w;
			upVector = upVector+cameraMode[i]->getUpVector()*w;
			FOV = FOV+cameraMode[i]->getFOV()*w;
		}
	}
}


// Set the absolute camera parameters
void CameraSystem::setCameraAngle(float a)
{
}

void CameraSystem::setCameraBetaAngle(float a)
{
}

void CameraSystem::setCameraBank(float a)
{
}

void CameraSystem::setCameraFOV(float fov)
{
}

void CameraSystem::setCameraDistance(float distance)
{
}

void CameraSystem::setCameraOffset(VC3 offs)
{
}


// Set the relative camera parameters
void CameraSystem::moveCameraAngle(float a)
{
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->moveCameraAngle(a);
	}
}

void CameraSystem::moveCameraBetaAngle(float a)
{
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->moveCameraBetaAngle(a);
	}
}

void CameraSystem::moveCameraBank(float a)
{
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->moveCameraBank(a);
	}
}

void CameraSystem::moveCameraFOV(float fov)
{
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->moveCameraFOV(fov);
	}
}

void CameraSystem::moveCameraDistance(float distance)
{
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->moveCameraDistance(distance);
	}
}

void CameraSystem::moveCameraOffset(VC3 offs)
{
}


// Enable/Disable camera collision (0=disabled, 1=enabled)
void CameraSystem::setCollision(int enabled)
{
}

void CameraSystem::setResponse(float response)
{
	this->response = response;
	for (int i=0; i<cameraModes; i++) {
		cameraMode[i]->setResponse(response);
	}
}

// Query the camera parameters
VC3 CameraSystem::getCameraPosition()
{
	return cameraPosition;
}

VC3 CameraSystem::getTargetPosition()
{
	return targetPosition;
}

VC3 CameraSystem::getUpVector()
{
	return upVector;
}

float CameraSystem::getFOV()
{
	return FOV;
}


} // end of namespace ui
