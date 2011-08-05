#include "precompiled.h"
#include "CameraSystemAreas.h"

#include <boost/lexical_cast.hpp>
#include "../../system/Logger.h"

#include "../../game/SimpleOptions.h"
#include "../../game/options/options_camera.h"



#define DAMPER_OVERDAMP_FACTOR 1.5f

namespace ui {

CameraSystemAreas::CameraSystemAreas()
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

	cameraAreas  = 0;
	lastCameraArea = -1;
	response = 10;

	posDamper = util::VectorDamper();
	posDamper.setK(response);
	posDamper.setOverdampFactor(DAMPER_OVERDAMP_FACTOR);

	targetDamper = util::VectorDamper();
	targetDamper.setK(response);
	targetDamper.setOverdampFactor(DAMPER_OVERDAMP_FACTOR);

	upDamper = util::VectorDamper();
	upDamper.setK(response);
	upDamper.setOverdampFactor(DAMPER_OVERDAMP_FACTOR);

	attributeDamper = util::VectorDamper();
	attributeDamper.setK(response);
	attributeDamper.setOverdampFactor(DAMPER_OVERDAMP_FACTOR);

	posDamper.setTarget(cameraPosition);
	posDamper.cutToTarget();
	targetDamper.setTarget(targetPosition);
	targetDamper.cutToTarget();
	upDamper.setTarget(upVector);
	upDamper.cutToTarget();
	attributeDamper.setTarget(VC3(FOV, 0, 0));
	attributeDamper.cutToTarget();

	mode = 0;
	modeChangeTimer = 0;
}

CameraSystemAreas::~CameraSystemAreas()
{
	for (int i=0; i<cameraAreas;i++) {
		delete cameraArea[i];
	}
}

void CameraSystemAreas::setMode(int mode)
{
	if (this->mode == mode) return;
	this->mode = mode;
	if (mode == 0) {
		// If returning to default mode, keep camera direction from the old mode
        VC3 dir = getTargetPosition()-getCameraPosition();
		VC3 flatDir(dir.x, 0.0f, dir.z);
        VC3 zeroAngle(0.0f, 0.0f, 1.0f);
		VC3 upDir(0.0f, 1.0f, 0.0f);

		/*
		alphaAngle = 180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
		if (flatDir.GetCrossWith(zeroAngle).y < 0) {
			alphaAngle = 360-alphaAngle;
		}
		*/
		float tmp = -180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
		if (flatDir.GetCrossWith(zeroAngle).y < 0) {
			tmp = 360-tmp;
		}
		tmp += 90.0f;
		if (tmp >= 360.0f) tmp -= 360.0f;
		moveCameraAngle(tmp-angle);
	}
}

void CameraSystemAreas::setAimPosition(VC3 pos)
{
	this->aimPosition = pos;
}

void CameraSystemAreas::addCameraArea(ICameraArea *cameraArea)
{
	this->cameraArea[cameraAreas] = cameraArea;
	cameraAreas++;
}

float matchAngle(float a1, float a2)
{
	float a = a1;
	while (a-a2>180.0f) a-=360.0f;
	while (a-a2<-180.0f) a+=360.0f;
	return a;
}

// Sets the position of interest and elapsed time for the camera system. Must be used before querying
// camera parameters.
void CameraSystemAreas::update( VC3 positionOfInterest, int elapsedTime )
{
	VC3 speed = positionOfInterest - this->positionOfInterest;
	float totalWeight = 0;
	float totalTWeight = 0;
	float oldAngle;
	float scrollAmount = game::SimpleOptions::getInt(DH_OPT_I_CAMERA_SCROLLY_AMOUNT) * 0.01f;

	this->positionOfInterest = positionOfInterest;

	std::string foofoo = "";


	// Update camera areas
	for (int i=0; i<cameraAreas; ++i){
//		cameraArea[i]->update((positionOfInterest*(1.0f-scrollAmount)+aimPosition*scrollAmount), angle, elapsedTime);
		cameraArea[i]->update(positionOfInterest, angle, elapsedTime);
	}
	// Calculate total weights
	for (int i=0; i<cameraAreas; ++i){
		if ((cameraArea[i]->getWeight()+cameraArea[i]->getTransitionWeight())==0.0f) {
			cameraArea[i]->reset();
		} else {
			foofoo = foofoo + boost::lexical_cast< std::string >(i) + ": " +
				boost::lexical_cast< std::string >(cameraArea[i]->getWeight()) + ", " +
				boost::lexical_cast< std::string >(cameraArea[i]->getTransitionWeight()) + ", "+
				boost::lexical_cast< std::string >(cameraArea[i]->getCameraAngle()) + "    ";

			totalWeight+=cameraArea[i]->getWeight();
			totalTWeight+=cameraArea[i]->getTransitionWeight();
		}
	}
	if (lastCameraArea!=-1) oldAngle = cameraArea[lastCameraArea]->getCameraAngle(); else oldAngle = angle;
	// Interpolate camera parameters from camera areas
	if (totalWeight>0) {
		angle = 0;
		betaAngle = 0;
		bank = 0;
		distance = 0;
		FOV = 0;
		offset = VC3(0, 0, 0);
		for (int i=0; i<cameraAreas; ++i){
			float w = cameraArea[i]->getWeight();
			if (w>0) {
				angle+=w*matchAngle(cameraArea[i]->getCameraAngle(), oldAngle)/totalWeight;
				betaAngle+=w*cameraArea[i]->getCameraBetaAngle()/totalWeight;
				bank+=w*cameraArea[i]->getCameraBank()/totalWeight;
				distance+=w*cameraArea[i]->getCameraDistance()/totalWeight;
				FOV+=w*cameraArea[i]->getCameraFOV()/totalWeight;
				offset+=cameraArea[i]->getCameraOffset()*w/totalWeight;
				lastCameraArea = i;
			}
		}
	} else {
		if (lastCameraArea!=-1) {
			angle = cameraArea[lastCameraArea]->getCameraAngle();
			betaAngle = cameraArea[lastCameraArea]->getCameraBetaAngle();
			bank = cameraArea[lastCameraArea]->getCameraBank();
			distance = cameraArea[lastCameraArea]->getCameraDistance();
			FOV = cameraArea[lastCameraArea]->getCameraFOV();
			offset = cameraArea[lastCameraArea]->getCameraOffset();
		}
	}
	if (totalTWeight>0) {
		if (totalTWeight>1) {
			angle = 0;
			betaAngle = 0;
			bank = 0;
			distance = 0;
			FOV = 0;
			offset = VC3(0, 0, 0);
		} else {
			angle *= 1-totalTWeight;
			betaAngle *= 1-totalTWeight;
			bank *= 1-totalTWeight;
			distance *= 1-totalTWeight;
			FOV *= 1-totalTWeight;
			offset *= 1-totalTWeight;
			totalTWeight = 1;
		}
		for (int i=0; i<cameraAreas; ++i){
			float w = cameraArea[i]->getTransitionWeight();
			if (w>0) {
				angle+=w*matchAngle(cameraArea[i]->getCameraAngle(), oldAngle)/totalTWeight;
				betaAngle+=w*cameraArea[i]->getCameraBetaAngle()/totalTWeight;
				bank+=w*cameraArea[i]->getCameraBank()/totalTWeight;
				distance+=w*cameraArea[i]->getCameraDistance()/totalTWeight;
				FOV+=w*cameraArea[i]->getCameraFOV()/totalTWeight;
				offset+=cameraArea[i]->getCameraOffset()*w/totalTWeight;
			}
		}
	}	

	// Account for player movement
	offset+=speed*300/response;

	// Calculate camera, target and up

	targetPosition = (positionOfInterest*(1.0f-scrollAmount)+aimPosition*scrollAmount)+offset;

	VC3 v(distance, 0.0f, 0.0f);
	QUAT tmp_quat( 0, DEG2RAD( angle ), -DEG2RAD( betaAngle ) );
	tmp_quat.RotateVector( v );
	cameraPosition = targetPosition+v;

	upVector = VC3(0, 1, 0);
	QUAT tmp_quat2( (targetPosition-cameraPosition).GetNormalized(), DEG2RAD( bank ));
	tmp_quat2.RotateVector( upVector );

//	modeChangeTimer = 0;

	foofoo = foofoo + boost::lexical_cast< std::string >(angle);
	if (game::SimpleOptions::getBool(DH_OPT_B_CAMERA_SYSTEM_DEBUG)) Logger::getInstance()->error( foofoo.c_str() );


	if (mode==1) {
		cameraPosition = positionOfInterest + VC3(0, 1.395f, 0);
		targetPosition = aimPosition;
		targetPosition.y = cameraPosition.y;
		cameraPosition = cameraPosition + (cameraPosition-targetPosition).GetNormalized()*1.687f;
//		VC3 left = (targetPosition-cameraPosition).GetCrossWith(VC3(0.0f, 1.0f, 0.0f));
//		upVector = (targetPosition-cameraPosition).GetCrossWith(left)*(-1.0f);
//		QUAT tmp_quat3((targetPosition-cameraPosition), -7.7f);
		upVector = VC3(0, 1, 0);
		QUAT tmp_quat3( (targetPosition-cameraPosition).GetNormalized(), DEG2RAD( -7.7f ));
		tmp_quat3.RotateVector( upVector );
		FOV = 70.0f;


//		posDamper.setK(500);
//		targetDamper.setK(500);
//		upDamper.setK(500);
//		attributeDamper.setK(500);

		posDamper.cutToTarget();
		targetDamper.cutToTarget();
		upDamper.cutToTarget();

		modeChangeTimer = 1000;
	} else {
		posDamper.setK(response);
		targetDamper.setK(response);
		upDamper.setK(response);
		attributeDamper.setK(response);

		if (modeChangeTimer>0) {
			modeChangeTimer -=elapsedTime;
			cameraPosition = positionOfInterest + VC3(0, 1.395f, 0);
			targetPosition = aimPosition;
			targetPosition.y = cameraPosition.y;
			cameraPosition = cameraPosition + (cameraPosition-targetPosition).GetNormalized()*1.687f;
	//		VC3 left = (targetPosition-cameraPosition).GetCrossWith(VC3(0.0f, 1.0f, 0.0f));
	//		upVector = (targetPosition-cameraPosition).GetCrossWith(left)*(-1.0f);
	//		QUAT tmp_quat3((targetPosition-cameraPosition), -7.7f);
			upVector = VC3(0, 1, 0);
			QUAT tmp_quat3( (targetPosition-cameraPosition).GetNormalized(), DEG2RAD( -7.7f ));
			tmp_quat3.RotateVector( upVector );
			FOV = 70.0f;

		}
	}
	
	posDamper.setTarget(cameraPosition);
	posDamper.update(elapsedTime);
	targetDamper.setTarget(targetPosition);
	targetDamper.update(elapsedTime);
	upDamper.setTarget(upVector);
	upDamper.update(elapsedTime);
	attributeDamper.setTarget(VC3(FOV, 0, 0));
	attributeDamper.update(elapsedTime);
}


// Set the absolute camera parameters
void CameraSystemAreas::setCameraAngle(float a)
{
}

void CameraSystemAreas::setCameraBetaAngle(float a)
{
}

void CameraSystemAreas::setCameraBank(float a)
{
}

void CameraSystemAreas::setCameraFOV(float fov)
{
}

void CameraSystemAreas::setCameraDistance(float distance)
{
}

void CameraSystemAreas::setCameraOffset(VC3 offs)
{
}


// Set the relative camera parameters
void CameraSystemAreas::moveCameraAngle(float a)
{
	for (int i=0; i<cameraAreas; i++) {
		cameraArea[i]->moveCameraAngle(a);
	}
}

void CameraSystemAreas::moveCameraBetaAngle(float a)
{
	for (int i=0; i<cameraAreas; i++) {
		cameraArea[i]->moveCameraBetaAngle(a);
	}
}

void CameraSystemAreas::moveCameraBank(float a)
{
	for (int i=0; i<cameraAreas; i++) {
		cameraArea[i]->moveCameraTilt(a);
	}
}

void CameraSystemAreas::moveCameraFOV(float fov)
{
	for (int i=0; i<cameraAreas; i++) {
		cameraArea[i]->moveCameraFOV(fov);
	}
}

void CameraSystemAreas::moveCameraDistance(float distance)
{
	for (int i=0; i<cameraAreas; i++) {
		cameraArea[i]->moveCameraDistance(distance);
	}
}

void CameraSystemAreas::moveCameraOffset(VC3 offs)
{
}


// Enable/Disable camera collision (0=disabled, 1=enabled)
void CameraSystemAreas::setCollision(int enabled)
{
}

void CameraSystemAreas::setResponse(float response)
{
	this->response = response;
	posDamper.setK(response);
	targetDamper.setK(response);
	upDamper.setK(response);
}

// Query the camera parameters
VC3 CameraSystemAreas::getCameraPosition()
{
	return( posDamper.getPosition() );
}

VC3 CameraSystemAreas::getTargetPosition()
{
	return( targetDamper.getPosition() );
}

VC3 CameraSystemAreas::getUpVector()
{
	return( upDamper.getPosition() );
}

float CameraSystemAreas::getFOV()
{
	return( attributeDamper.getPosition().x );
}



} // end of namespace ui
