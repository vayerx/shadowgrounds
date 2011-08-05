
#include "precompiled.h"

#include "convex_actor.h"
#include "file_stream.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

// ----------
// ConvexMesh
// ----------

ConvexMesh::ConvexMesh(NxPhysicsSDK &sdk_, const char *filename)
:	sdk(sdk_),
	mesh(0)
{
	mesh = sdk.createConvexMesh(InputPhysicsStream(filename));

	if (mesh == NULL)
	{
		//Logger::getInstance()->error("ConvexMesh - Shit happens.");
		//Logger::getInstance()->debug(filename);
		//abort();
	}
}

ConvexMesh::~ConvexMesh()
{
	if(mesh)
		sdk.releaseConvexMesh(*mesh);
}

bool ConvexMesh::isValidForHardware() const
{
	if(mesh)
	{
		int triangles = mesh->getCount(0, NX_ARRAY_TRIANGLES);
		int vertices = mesh->getCount(0, NX_ARRAY_VERTICES);
		if(triangles <= 32 && vertices <= 32)
			return true;
	}

	return false;

}

bool ConvexMesh::isValid() const
{
	if(mesh)
		return true;
	
	return false;
}

// -----------
// ConvexActor
// -----------

ConvexActor::ConvexActor(NxScene &scene, const boost::shared_ptr<ConvexMesh> &mesh_, const VC3 &position)
:	mesh(mesh_)
{
	if(!mesh)
		return;

	NxBodyDesc bodyDesc;
	NxConvexShapeDesc convexDesc;

	if (!mesh->isValidForHardware()) {
		// FIXME: need to display mesh name
		// not easy...
		Logger::getInstance()->warning("mesh too large for hardware - software fallback\n");
	}
	convexDesc.meshData = mesh->mesh;

	NxActorDesc actorDesc;
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.f;
	actorDesc.shapes.pushBack(&convexDesc);
	actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));
	actor = scene.createActor(actorDesc);

	this->scene = &scene;
	init();
}

ConvexActor::~ConvexActor()
{
	if(scene && actor)
	{
		scene->releaseActor(*actor);
		actor = 0;
	}
}

bool ConvexActor::isValid() const
{
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
