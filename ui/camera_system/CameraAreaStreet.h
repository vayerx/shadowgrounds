#ifndef CAMERAAREASTREET_H
#define CAMERAAREASTREET_H

#include "ICameraArea.h"
#include <string>

namespace ui {

//class Item;

	class CameraAreaStreet : public ICameraArea
{
protected:
	VC3 corner[4];
	float FOV;
	float angle;
	float betaAngle;
	float bank;
	float distance;
	VC3 offset;
	VC3 follow;
	VC3 target;
	std::string animation;
	std::string name;
	int group;
	int collision;

	float weight;
	float transitionWeight;

	float curFOV;
	float curAngle;
	float curBetaAngle;
	float curBank;
	float curDistance;
	float angleOffset;
	float betaAngleOffset;
	float distanceOffset;
	VC3 curOffset;

	VC3 p1;
	VC3 p2;
	VC3 streetX;
	VC3 streetZ;

	VC3 entryPoint;
	float entryAngle;
	bool hasEntryPoint;
	bool hasCameraAngle;
	float selectedAngle;

	bool bidirectional;

	bool flipped;

	VC3 getTransformedCoord(VC3 v);
	VC3 getGlobalCoord(VC3 v);

public:
	CameraAreaStreet(std::string name, int group, int collision,
		             VC3 corner1, VC3 corner2, VC3 corner3, VC3 corner4,
		             float angle, float betaAngle, float bank, float distance, float fov,
					 VC3 target, VC3 offset, VC3 follow,
					 std::string animation);
	virtual ~CameraAreaStreet() { }

	virtual void reset();

	 // Sets the position of interest and elapsed time for the camera area. Must be used before querying
	 // camera parameters.
	virtual void update( VC3 positionOfInterest, float currentAngle, int elapsedTime );

     // Set the relative camera parameters
	virtual void moveCameraAngle(float a);
	virtual void moveCameraBetaAngle(float a);
	virtual void moveCameraTilt(float a);
	virtual void moveCameraFOV(float fov);
	virtual void moveCameraDistance(float distance);
	virtual void moveCameraOffset(VC3 offs);

	 // Query the camera parameters
	virtual float getWeight();
	virtual float getTransitionWeight();

	virtual float getCameraAngle();
	virtual float getCameraBetaAngle();
	virtual float getCameraBank();
	virtual float getCameraFOV();
	virtual float getCameraDistance();
	virtual VC3 getCameraOffset();


};

} // end of namespace ui

#endif // CAMERAAREASTREET_H
