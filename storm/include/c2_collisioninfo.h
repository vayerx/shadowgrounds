// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef C2_COLLISIONINFO_H
#define C2_COLLISIONINFO_H

#include "c2_vectors.h"

class IStorm3D_Model;
class IStorm3D_Model_Object;

struct Storm3D_CollisionInfo
{
	bool hit;

	// Information about collision
	VC3 position;					// Nearest collision position
	VC3 plane_normal;				// Collided face's normal vector (in global coordinates)
	float range;						// Range of collision (from position)
	IStorm3D_Model *model;				// Model that was collided
	IStorm3D_Model_Object *object;		// Object that was collided

	// Spherecollision specific:
	// The amount (among plane_normal) that must be added to coordinate in order to get the sphere out of the wall...
	// Use like this: position+=colinfo.plane_normal*colinfo.inside_amount
	float inside_amount;				
	bool onlySolidSurfaces;
	bool includeTerrainObjects;
	int terrainInstanceId;
	int terrainModelId;

	Storm3D_CollisionInfo() 
	:	hit(false), 
		position(0.0f),
		plane_normal(0.0f),
		range(static_cast<float>(HUGE)),
		model(0),
		object(0),
		inside_amount(0),
		onlySolidSurfaces(false),
		includeTerrainObjects(true),
		terrainInstanceId(-1),
		terrainModelId(-1)
	{
	}
};

#endif
