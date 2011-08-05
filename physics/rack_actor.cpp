
#include "precompiled.h"

#include "rack_actor.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {
namespace {

	const float xsize = 2.1763f * 0.5f;
	const float zsize = 0.70193f * 0.5f;
	const float SHELF_THICKNESS = 0.05f * 0.5f;
	const float height1 = 0.17605f;
	const float height2 = 1.2361f;
	const float height3 = 2.3161f;

} // unnamed

RackActor::RackActor(NxScene &scene, const VC3 &position)
{
	// Shelves
	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions = NxVec3(xsize, SHELF_THICKNESS, zsize);
	boxDesc1.localPose.t.set(NxVec3(0, height1 - SHELF_THICKNESS, 0));

	NxBoxShapeDesc boxDesc2;
	boxDesc2.dimensions = NxVec3(xsize, SHELF_THICKNESS, zsize);
	boxDesc2.localPose.t.set(NxVec3(0, height2 - SHELF_THICKNESS, 0));

	NxBoxShapeDesc boxDesc3;
	boxDesc3.dimensions = NxVec3(xsize, SHELF_THICKNESS * 0.5f, zsize);
	boxDesc3.localPose.t.set(NxVec3(0, height3 - SHELF_THICKNESS, 0));

	// 
	NxBoxShapeDesc boxDesc4;
	boxDesc4.dimensions = NxVec3(SHELF_THICKNESS, height3 * 0.5f, zsize);
	boxDesc4.localPose.t.set(NxVec3(-xsize - SHELF_THICKNESS, height3 * 0.5f, 0));
	NxBoxShapeDesc boxDesc5;
	boxDesc5.dimensions = NxVec3(SHELF_THICKNESS, height3 * 0.5f, zsize);
	boxDesc5.localPose.t.set(NxVec3(xsize + SHELF_THICKNESS, height3 * 0.5f, 0));

	NxBodyDesc bodyDesc;
	NxActorDesc actorDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	actorDesc.shapes.pushBack(&boxDesc1);
	actorDesc.shapes.pushBack(&boxDesc2);
	actorDesc.shapes.pushBack(&boxDesc3);
	actorDesc.shapes.pushBack(&boxDesc4);
	actorDesc.shapes.pushBack(&boxDesc5);
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

	actor = scene.createActor(actorDesc);

	this->scene = &scene;
	init();
}

RackActor::~RackActor()
{
}

bool RackActor::isValid() const
{
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
