#include "precompiled.h"
#include "CameraAreaStreet.h"

#include <boost/lexical_cast.hpp>
#include "../../system/Logger.h"

#define ROTATE_DISTANCE 7.0f
#define ROTATE_DISTANCE_SIDE 3.0f
#define ROTATE_SHARPNESS 5.0f
#define ROTATE_OFFSET 1.5f

#define WALL_ROTATE_DIST 3.0f
#define WALL_ROTATE_STRENGTH 5.0f
#define WALL_ROTATE_WIDTH 0.2f

#define ANGLE_TOLERANCE 25.0f

#define ANGLE_SELECTION_DISTANCE 2.0f;

namespace ui {

CameraAreaStreet::CameraAreaStreet(std::string name, int group, int collision,
		             VC3 corner1, VC3 corner2, VC3 corner3, VC3 corner4,
		             float angle, float betaAngle, float bank, float distance, float fov,
					 VC3 target, VC3 offset, VC3 follow,
					 std::string animation)
{
	this->name = name;
	this->group = group;
	this->collision = collision;
	this->corner[0] = corner1;
	this->corner[1] = corner2;
	this->corner[2] = corner3;
	this->corner[3] = corner4;
	this->angle = angle;
	this->betaAngle = betaAngle;
	this->bank = bank;
	this->distance = distance;
	this->FOV = fov;
	this->target = target;
	this->offset = offset;
	this->follow = follow;
	this->animation = animation;

	this->bidirectional = false;
	this->angleOffset = 0;
	this->betaAngleOffset = 0;
	this->distanceOffset = 0;

	curFOV = this->FOV;
	curAngle = this->angle;
	curBetaAngle = this->betaAngle;
	curBank = this->bank;
	curDistance = this->distance;
	curOffset = this->offset;

	entryPoint = VC3(0, 0, 0);
	entryAngle = angle;
	selectedAngle = 0;
	hasEntryPoint = true;
	hasCameraAngle = true;

	p1 = (corner[0]+corner[1])*0.5;
	p2 = (corner[2]+corner[3])*0.5;
	streetZ = (p2-p1).GetNormalized();
	streetX = (streetZ.GetCrossWith(VC3(0, 1, 0))).GetNormalized();

	VC3 v(1.0f, 0.0f, 0.0f);
	QUAT tmp_quat( 0, DEG2RAD( this->angle ), 0 );
	tmp_quat.RotateVector( v );

	flipped = v.GetAngleTo(streetZ)<PI*0.5f;

}

void CameraAreaStreet::reset()
{
	hasCameraAngle = false;
	hasEntryPoint = false;
}

VC3 CameraAreaStreet::getTransformedCoord(VC3 v)
{
	VC3 v2 = v;
    v2.y=p1.y;
	v2 = v2-p1;
	float x = v2.GetDotWith(streetX);
	float z = v2.GetDotWith(streetZ);
	return VC3(x, v.y, z);
}

VC3 CameraAreaStreet::getGlobalCoord(VC3 v)
{   
	VC3 pos = p1+streetX*v.x+streetZ*v.z;
	pos.y = v.y;
	return pos;
}

float matchAngle2(float a1, float a2)
{
	float a = a1;
	while (a-a2>180.0f) a-=360.0f;
	while (a-a2<-180.0f) a+=360.0f;
	return a;
}

	 // Sets the position of interest and elapsed time for the camera area. Must be used before querying
	 // camera parameters.
void CameraAreaStreet::update( VC3 positionOfInterest, float currentAngle, int elapsedTime )
{
    VC3 v2=positionOfInterest;
    v2.y=corner[0].y;
//	angle += 0.001f*elapsedTime;
	weight=1;
	for (int i=0; (i<4)&&(weight!=0); ++i) {
		VC3 edge = corner[(i+1)%4]-corner[i];
		VC3 point = v2-corner[i];

		if ((edge.GetCrossWith(point)).y<0) {weight = 0;}
	}

	transitionWeight = 0;

	for (int i=0; i<2; i++) {
		VC3 line = corner[i*2+1]-corner[i*2];
//v3:=vectorTo(vectorSum(corner[i*2], vMult(line, dot(line, vectorTo(corner[i*2], v2)) / sqrvLen(line))), v2);
		VC3 v3 = v2 - corner[i*2] + line*(line.GetDotWith(v2-corner[i*2])/line.GetSquareLength());
		VC3 temp = VC3(1, 1, 1);
		float w = 1 + (v3.GetDotWith(((temp.GetCrossWith(line)).GetNormalized()))-ROTATE_OFFSET)/ROTATE_DISTANCE;
		bool outside = false;
		for (int k=0; k<2; k++) {
			line = corner[(k*2+2)%4]-corner[k*2+1];
			VC3 point = v2-corner[k*2+1];
			if ((line.GetCrossWith(point)).y<0) {
				if (w>1) w = 2-w;
				if (w<0) w = 0;

				v3 = v2 - corner[k*2+1] + line*(line.GetDotWith(v2-corner[k*2+1])/line.GetSquareLength());
				float w2 = 1 + (v3.GetDotWith(((temp.GetCrossWith(line)).GetNormalized())))/ROTATE_DISTANCE_SIDE;
				if (w2<0) w2 = 0;
				transitionWeight+=w*w2;
				outside = true;
			}
		}
		if (!outside) {
			if (w>1) w=0;
			if (w<0) w=0;
			transitionWeight+=w;
		}
	}
//	transitionWeight = 0;
	if (transitionWeight>1) transitionWeight = 1;
	if (transitionWeight>0) weight = 0;

	if (bidirectional) {
		 // If the player is entering the street, prepare to select an appropriate camera angle
		if (((transitionWeight+weight)>0) && (!hasCameraAngle)) {
			angleOffset = 0;
			betaAngleOffset = 0;
			distanceOffset = 0;
			if (!hasEntryPoint) {
				float a = fabs(currentAngle-matchAngle2(angle, currentAngle));
				if ((a<ANGLE_TOLERANCE) || (a>(180-ANGLE_TOLERANCE))) {
					entryPoint = positionOfInterest;
					hasEntryPoint = true;
					selectedAngle = currentAngle;
					hasCameraAngle = true;
				} else {
					entryPoint = positionOfInterest;
					hasEntryPoint = true;
					selectedAngle = currentAngle;
					entryAngle = currentAngle;
				}
			} else {
				VC3 v = getTransformedCoord(positionOfInterest)-getTransformedCoord(entryPoint);
				if (flipped) v.z = -v.z;
				std::string foofoo = boost::lexical_cast< std::string >(v.z);
				Logger::getInstance()->error( foofoo.c_str() );
				if (v.z>0) {
					float s = v.z/ANGLE_SELECTION_DISTANCE;
					if (s>1) {
						s=1;
						hasCameraAngle = true;
					}
					selectedAngle = entryAngle*(1-s)+matchAngle2(angle, entryAngle)*s;
				} else {
					float s = -v.z/ANGLE_SELECTION_DISTANCE;
					if (s>1) {
						s=1;
						hasCameraAngle = true;
					}
					selectedAngle = entryAngle*(1-s)+matchAngle2((angle+180.0f), entryAngle)*s;
				}
			}
		}
	} else {
/*		selectedAngle = angle;
		if (((transitionWeight+weight)>0) && (!hasCameraAngle)) {
			angleOffset = 0;
			betaAngleOffset = 0;
			distanceOffset = 0;
			hasCameraAngle = true;
		}*/
	}
//	selectedAngle = RAD2DEG(angle);
	selectedAngle = angle;
		std::string foofoo = "ANGLE: ";
		foofoo = foofoo + boost::lexical_cast< std::string >(selectedAngle) + ", ";
	//	Logger::getInstance()->error( foofoo.c_str() );

//	angleOffset = 0;
	// Calculate current parameters based on follow

//	follow = VC3(1.0f,1.0f,1.0f);

	VC3 targetPosition = positionOfInterest+offset;

	VC3 v(distance, 0.0f, 0.0f);
	QUAT tmp_quat( 0, DEG2RAD( (selectedAngle+angleOffset) ), -DEG2RAD( betaAngle ) );
//	QUAT tmp_quat( 0, DEG2RAD( selectedAngle ), -DEG2RAD( betaAngle ) );
	tmp_quat.RotateVector( v );
	VC3 t = getTransformedCoord(target);
	VC3 cameraPosition = getGlobalCoord(t + (getTransformedCoord(positionOfInterest)-t)*follow) + offset + v;

    VC3 dir = targetPosition-cameraPosition;
	VC3 flatDir(dir.x, 0.0f, dir.z);
    VC3 zeroAngle(-1.0f, 0.0f, 0.0f);
	VC3 upDir(0.0f, 1.0f, 0.0f);

	curAngle = -180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
	if (flatDir.GetCrossWith(zeroAngle).y < 0) {
		curAngle = 360-curAngle;
	}
	curBetaAngle = -90+180*dir.GetAngleTo(upDir)/3.1415f + betaAngleOffset;
    curDistance = dir.GetLength()+distanceOffset;
	curAngle = curAngle+angleOffset;
}

     // Set the relative camera parameters
void CameraAreaStreet::moveCameraAngle(float a)
{
//	curAngle+=a;
	angleOffset+=a;
}

void CameraAreaStreet::moveCameraBetaAngle(float a)
{
//	curBetaAngle+=a;
	betaAngleOffset+=a;
}

void CameraAreaStreet::moveCameraTilt(float a)
{
	curBank+=a;
}

void CameraAreaStreet::moveCameraFOV(float fov)
{
	curFOV+=fov;
}

void CameraAreaStreet::moveCameraDistance(float distance)
{
//	curDistance+=distance;
	distanceOffset+=distance;
}

void CameraAreaStreet::moveCameraOffset(VC3 offs)
{
	curOffset+=offs;
}


	 // Query the camera parameters
float CameraAreaStreet::getWeight()
{
	return(weight);
}

float CameraAreaStreet::getTransitionWeight()
{
	return(transitionWeight);
}


float CameraAreaStreet::getCameraAngle()
{
	return(curAngle);
}

float CameraAreaStreet::getCameraBetaAngle()
{
	return(curBetaAngle);
}

float CameraAreaStreet::getCameraBank()
{
	return(curBank);
}

float CameraAreaStreet::getCameraFOV()
{
	return(curFOV);
}

float CameraAreaStreet::getCameraDistance()
{
	return(curDistance);
}

VC3 CameraAreaStreet::getCameraOffset()
{
	return(curOffset);
}




} // end of namespace ui
