
#include "precompiled.h"

#include "static_mesh_actor.h"
#include "file_stream.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

// ----------
// StaticMesh
// ----------

StaticMesh::StaticMesh(NxPhysicsSDK &sdk_, const char *filename)
:	sdk(sdk_),
	mesh(0)
{
	InputPhysicsStream strm(filename);
	if(strm.getSize() > 0)
	{
		mesh = sdk.createTriangleMesh(strm);
	}

	if (mesh == NULL)
	{
		//Logger::getInstance()->error("StaticMesh - Shit happens.");
		//Logger::getInstance()->debug(filename);
		//abort();
	}
}

StaticMesh::~StaticMesh()
{
	// FIXED: crash due to physics being deleted already at Game::endCombat()
	// 
	if(mesh)
		sdk.releaseTriangleMesh(*mesh);
}

// ---------------
// StaticMeshActor
// ---------------

StaticMeshActor::StaticMeshActor(NxScene &scene, const boost::shared_ptr<StaticMesh> &mesh_, const VC3 &position, const QUAT &rotation)
:	mesh(mesh_)
{
	if(!mesh)
		return;

	NxTriangleMeshShapeDesc triangleDesc;
	triangleDesc.meshData = mesh->mesh;
	triangleDesc.meshPagingMode = NX_MESH_PAGING_AUTO;
	NxActorDesc actorDesc;

	{
		QUAT r = rotation.GetInverse();
		NxQuat quat;
		quat.setXYZW(r.x, r.y, r.z, r.w);

		actorDesc.globalPose.M = quat;
		actorDesc.globalPose.t = NxVec3(position.x, position.y, position.z);
	}

	actorDesc.shapes.pushBack(&triangleDesc);
	actor = scene.createActor(actorDesc);
/*
	if(actor)
	{
		shape = actor->getShapes()[0]->isTriangleMesh();
	}

	if(triangleDesc.meshData && actor && shape)
	{
		for(unsigned int i = 0; i < triangleDesc.meshData->getPageCount(); ++i)
		{
			shape->mapPageInstance(i);
		}
	}
*/
	this->scene = &scene;

	// TODO: is this ok?
	// this was missing, so contacts did not work properly, added it... --jpk
	init();
}

StaticMeshActor::~StaticMeshActor()
{
	if(scene && actor)
	{
		scene->releaseActor(*actor);
		actor = 0;
	}
}

bool StaticMeshActor::isValid() const
{
	//if(actor && shape)
	if(actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
