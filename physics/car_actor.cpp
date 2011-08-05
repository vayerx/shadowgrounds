
#include "precompiled.h"

#include "car_actor.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

CarActor::CarActor(NxScene &scene, const VC3 &position)
{
	NxBoxShapeDesc boxDesc1;
	//boxDesc1.dimensions = NxVec3(2.65f, 0.55f, 1.05f);
	boxDesc1.dimensions = NxVec3(1.05f, 0.55f, 2.65f);
	boxDesc1.localPose.t.set(NxVec3(0, boxDesc1.dimensions.y, 0));

	NxBoxShapeDesc boxDesc2;
	//boxDesc2.dimensions = NxVec3(1.30f, 0.77f - boxDesc1.dimensions.y, 0.84f);
	boxDesc2.dimensions = NxVec3(0.84f, 0.77f - boxDesc1.dimensions.y, 1.30f);
	boxDesc2.localPose.t.set(NxVec3(0, (boxDesc1.dimensions.y * 2.f) + boxDesc2.dimensions.y, 0));

	NxBodyDesc bodyDesc;
	NxActorDesc actorDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	actorDesc.shapes.pushBack(&boxDesc1);
	actorDesc.shapes.pushBack(&boxDesc2);
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

	actor = scene.createActor(actorDesc);

	this->scene = &scene;
	init();
}

CarActor::~CarActor()
{
}

bool CarActor::isValid() const
{
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
