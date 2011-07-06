
#include "precompiled.h"

#include "box_actor.h"
#include "NxPhysics.h"
#include <vector>
#include <stdio.h>

namespace frozenbyte {
namespace physics {

// HACK: ...
extern NxPhysicsSDK *physxSDK;


NxCCDSkeleton* CreateCCDSkeleton(const VC3 &sizes)
{
  NxU32 triangles[3 * 12] = {
    0,1,3,
    0,3,2,
    3,7,6,
    3,6,2,
    1,5,7,
    1,7,3,
    4,6,7,
    4,7,5,
    1,0,4,
    5,1,4,
    4,0,2,
    4,2,6
  };

  NxVec3 points[8];

  // Static mesh
  points[0].set(sizes.x, -sizes.y, -sizes.z);
  points[1].set(sizes.x, -sizes.y,  sizes.z);
  points[2].set(sizes.x,  sizes.y, -sizes.z);
  points[3].set(sizes.x,  sizes.y,  sizes.z);

  points[4].set(-sizes.x, -sizes.y, -sizes.z);
  points[5].set(-sizes.x, -sizes.y,  sizes.z);
  points[6].set(-sizes.x,  sizes.y, -sizes.z);
  points[7].set(-sizes.x,  sizes.y,  sizes.z);

  NxSimpleTriangleMesh stm;
  stm.numVertices = 8;
  stm.numTriangles = 6*2;
  stm.pointStrideBytes = sizeof(NxVec3);
  stm.triangleStrideBytes = sizeof(NxU32)*3;

  stm.points = points;
  stm.triangles = triangles;
  stm.flags |= NX_MF_FLIPNORMALS;
  return physxSDK->createCCDSkeleton(stm);
}

BoxActor::BoxActor(NxScene &scene, const VC3 &sizes, const VC3 &position, const VC3 &localPosition, bool ccd, float ccdMaxThickness)
{
	NxBodyDesc bodyDesc;
	//bodyDesc.solverIterationCount = 2;

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = NxVec3(sizes.x, sizes.y, sizes.z);
	boxDesc.localPose.t.set(NxVec3(localPosition.x, localPosition.y + sizes.y, localPosition.z));

	// CCD, but for thin objects only... --jpk
	if (ccd && (sizes.x*2 < ccdMaxThickness || sizes.y*2 < ccdMaxThickness || sizes.z*2 < ccdMaxThickness))
	{
		VC3 ccdSizes = sizes * 0.6f;
		boxDesc.ccdSkeleton = CreateCCDSkeleton(ccdSizes);
		boxDesc.shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;

		// also, in this case, a minimal skin width too.
		boxDesc.skinWidth = 0.002f;
	}

	NxActorDesc actorDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

	// !!!!!!!!!!!!!!
	//actorDesc.managedHwSceneIndex = 1;

	actor = scene.createActor(actorDesc);

	this->scene = &scene;
	init();
}


BoxActor::BoxActor(NxScene &scene, const std::string &shapes, const VC3 &position, bool ccd, float ccdMaxThickness)
{
	NxBodyDesc bodyDesc;
	//bodyDesc.solverIterationCount = 2;

	NxBoxShapeDesc boxes[64];
	int numBoxes = 0;
	
	// parse shapes string, format:
	// (sizeX,sizeY,sizeZ;posX,posY,posZ;rotX,rotY,rotZ),(sizeX,sizeY,sizeZ;posX,posY,posZ;rotX,rotY,rotZ)
	std::string::size_type pos_start = 0;
	std::string::size_type pos_end = 0;
	while(pos_end + 1 < shapes.length())
	{
		// find ( )
		pos_start = shapes.find('(', pos_end);
		pos_end = shapes.find(')', pos_start);

		// end of string
		if(pos_start == std::string::npos && pos_end == std::string::npos)
		{
			break;
		}
		else if(pos_end == std::string::npos || pos_start == std::string::npos)
		{
#ifndef PROJECT_PARTICLE_EDITOR
			Logger::getInstance()->error("BoxActor::BoxActor - parsing shape string failed: braces not properly closed");
			Logger::getInstance()->error(shapes.c_str());
#endif
			break;
		}

		std::string data = shapes.substr(pos_start + 1, pos_end - pos_start - 1);

		float sizeX,sizeY,sizeZ, posX,posY,posZ, rotX,rotY,rotZ;
		if(sscanf(data.c_str(), "%f,%f,%f;%f,%f,%f;%f,%f,%f",
			&sizeX,&sizeY,&sizeZ,&posX,&posY,&posZ,&rotX,&rotY,&rotZ) != 9)
		{
#ifndef PROJECT_PARTICLE_EDITOR
			char str[256];
			sprintf(str, "BoxActor::BoxActor - parsing shape string failed: data \"%s\" format not valid", data.c_str());
			Logger::getInstance()->error(str);
			Logger::getInstance()->error(shapes.c_str());
#endif
			break;
		}

		NxBoxShapeDesc &boxDesc = boxes[numBoxes++];
		boxDesc.dimensions = NxVec3(sizeX,sizeY,sizeZ);
		boxDesc.localPose.t.set(NxVec3(posX,posY,posZ));
		NxMat33 mX,mZ;
		mX.rotX(rotX * PI / 180.0f);
		mZ.rotZ(rotZ * PI / 180.0f);
		boxDesc.localPose.M.rotY(rotY * PI / 180.0f);
		boxDesc.localPose.M *= mX;
		boxDesc.localPose.M *= mZ;
		
		// CCD, but for thin objects only... --jpk
		if (ccd && (sizeX*2 < ccdMaxThickness || sizeY*2 < ccdMaxThickness || sizeZ*2 < ccdMaxThickness))
		{
			VC3 ccdSizes(sizeX * 0.6f, sizeY * 0.6f, sizeZ * 0.6f);
			boxDesc.ccdSkeleton = CreateCCDSkeleton(ccdSizes);
			boxDesc.shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;

			// also, in this case, a minimal skin width too.
			boxDesc.skinWidth = 0.002f;
		}
	}

	NxActorDesc actorDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	for(int i = 0; i < numBoxes; i++)
		actorDesc.shapes.pushBack(&boxes[i]);
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

	// !!!!!!!!!!!!!!
	//actorDesc.managedHwSceneIndex = 1;

	actor = scene.createActor(actorDesc);

	this->scene = &scene;
	init();
}


BoxActor::~BoxActor()
{
}

bool BoxActor::isValid() const
{
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
