
#include "precompiled.h"

#include "physics_lib.h"
#include "capsule_actor.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

CapsuleActor::CapsuleActor(NxScene &scene, float height, float radius, const VC3 &position, float offset, int axisNumber)
{
	NxBodyDesc bodyDesc;

	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.localPose.t.set(NxVec3(0, radius + (0.5f * height) + offset, 0));
	if (axisNumber == 0)
		capsuleDesc.localPose.M.rotZ(PI/2);
	else if (axisNumber == 2)
		capsuleDesc.localPose.M.rotX(PI/2);

#ifdef PROJECT_CLAW_PROTO
	if( PhysicsLib::unitMaterial )
	{
		capsuleDesc.materialIndex = PhysicsLib::unitMaterial->getMaterialIndex();
	}
#endif

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

// !!!!!!!!!!!!!!
//actorDesc.managedHwSceneIndex = 1;


	actor = scene.createActor(actorDesc); 
	


	this->scene = &scene;
	init();
}

CapsuleActor::~CapsuleActor()
{
}

bool CapsuleActor::isValid() const
{
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
